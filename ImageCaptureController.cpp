/*
*   ImageCaptureController.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*	kyle@kylem.org
*/

#define REQUIRE_MANUAL_STEP true

#include "ImageCaptureController.h"

// Initialize the static member variable
bool ImageCaptureController::pylonInitialized = false;

void ImageCaptureController::initializePylon()
{
    if (!pylonInitialized)
    {
        PylonInitialize();
        pylonInitialized = true;;
    }
}

ImageCaptureController::ImageCaptureController(std::string id) : camera(CTlFactory::GetInstance().CreateFirstDevice()), captureId(id), stopWorker(false), lastImageId(0)
{   
    // Print the model name of the camera.
    cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

    // The parameter MaxNumBuffer can be used to control the count of buffers
    // allocated for grabbing. The default value of this parameter is 10.
    camera.MaxNumBuffer = 5;
    
    // Start the worker thread
    workerThread = std::thread(&ImageCaptureController::processQueue, this);
    initializeCamera();
}

void ImageCaptureController::initializeCamera()
{
    try
    {
        // Ensure the camera is open
        if (!camera.IsOpen())
        {
            camera.Open();
        }

        // Set the pixel format to Mono16
        GenApi::INodeMap& nodemap = camera.GetNodeMap();
        GenApi::CEnumerationPtr pixelFormat(nodemap.GetNode("PixelFormat"));
        if (IsAvailable(pixelFormat->GetEntryByName("Mono12")))
        {
            pixelFormat->FromString("Mono12");
            cout << "Pixel format set to Mono12" << endl;
        }
        else
        {
            cout << "Mono12 pixel format not available" << endl;
        }

    }
    catch (const GenericException& e)
    {
        cerr << "An exception occurred while initializing the camera." << endl
            << e.GetDescription() << endl;
    }
}

// Manually step through the image capture process
// Used to debug and manually swap colors
void ImageCaptureController::manuallyStepThroughImage()
{   
	if (!REQUIRE_MANUAL_STEP)
	{
		return;
	}
	cerr << endl << "Press enter to take next photo" << endl;
	while (cin.get() != '\n');
}

/*
* Captures a frame by capturing the red, green, and blue images.
* This will also check with the arduino and / or set the colors on the arduino before each step of the process.
*/
int ImageCaptureController::captureFrame()
{
    manuallyStepThroughImage();
	// Capture the red image
	OIIO::ImageBuf* redImageBuff = captureImageAsBuffer();

	// Set LED to green
	//arduinoConnection.sendCommand(SerialConn::SET_COLOR_GREEN);

    manuallyStepThroughImage();
	// Capture the green image
	OIIO::ImageBuf* greenImageBuff = captureImageAsBuffer();

	// Set LED to blue
	//arduinoConnection.sendCommand(SerialConn::SET_COLOR_BLUE);

    manuallyStepThroughImage();
	// Capture the blue image
	OIIO::ImageBuf* blueImageBuff = captureImageAsBuffer();

	// Create an RGBImage object and set the images
	RGBImage* rgbImage = new RGBImage();
	rgbImage->setRedImage(redImageBuff);
	rgbImage->setGreenImage(greenImageBuff);
	rgbImage->setBlueImage(blueImageBuff);

    // File details
	rgbImage->setCaptureId(captureId);
	rgbImage->setImageId(lastImageId);

	cout << "Pushing image to queue" << endl;

    // Push the RGBImage object to the queue
    imageQueue.push(rgbImage);

    // Notify the worker thread that a new image is available
    stopCondition.notify_all();

    lastImageId++;
    return 0;
}

OIIO::ImageBuf* ImageCaptureController::captureImageAsBuffer()
{
    OIIO::ImageBuf* image; // Image to return
    try
    {
        // Ensure the camera is open and ready to grab images
        if (!camera.IsOpen())
        {
			cout << "Opening camera..." << endl;
            camera.Open();
        }

        // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
        camera.GrabOne(5000, ptrGrabResult, TimeoutHandling_ThrowException);

        // Image grabbed successfully?
        if (ptrGrabResult->GrabSucceeded())
        {
            cout << "Grabbed image: " << lastImageId << endl;
            cout << "Image buffer size: " << ptrGrabResult->GetBufferSize() << endl;

            const uint16_t* pImageBuffer = (uint16_t*)ptrGrabResult->GetBuffer();

            // Get image specifications
            int width = ptrGrabResult->GetWidth();
            int height = ptrGrabResult->GetHeight();

			// Number of channels in the image
            int nchannels = 1; // Assumed monochrome

            EPixelType pixelType = ptrGrabResult->GetPixelType();

            // Debugging information
            cout << "Pixel type value: " << static_cast<int>(pixelType) << endl;
            cout << "PixelType_Mono16 value: " << static_cast<int>(PixelType_Mono12) << endl;
            cout << "Comparison result: " << (pixelType == PixelType_Mono12) << endl;


            // Determine the data type based on the pixel type
            OIIO::TypeDesc dataType = OIIO::TypeDesc::UINT16; // Cast to 16 because OIIO does not have 12bit
            cout << "Data type is uint8" << endl;


            // Create an ImageSpec
            OIIO::ImageSpec spec(width, height, nchannels, dataType);

            // Create an ImageBuf from the raw image data
            image = new OIIO::ImageBuf(spec);

            // Scale the 12-bit data to the full 16-bit range
            std::vector<uint16_t> scaledBuffer(width * height);
            for (int i = 0; i < width * height; ++i)
            {
                scaledBuffer[i] = pImageBuffer[i] << 4; // Left shift by 4 bits to scale to 16-bit range
            }

            image->set_pixels(OIIO::ROI::All(), dataType, scaledBuffer.data());

            #ifdef PYLON_WIN_BUILD
            // Display the grabbed image.
            Pylon::DisplayImage(1, ptrGrabResult);
            #endif
        }
        else
        {
            cout << "Error: " << std::hex << ptrGrabResult->GetErrorCode() << std::dec << " " << ptrGrabResult->GetErrorDescription() << endl;
        }

        return image;
    }
    catch (const GenericException& e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl
            << e.GetDescription() << endl;
        return nullptr;
    }
}

ImageCaptureController::~ImageCaptureController()
{
    {
        std::lock_guard<std::mutex> lock(stopMutex);
        stopWorker = true;
    }
    stopCondition.notify_all();
    workerThread.join();
    //PylonTerminate();
}

void ImageCaptureController::processQueue()
{
    while (true)
    {
        RGBImage* rgbImage;
        {
            std::unique_lock<std::mutex> lock(stopMutex);
            stopCondition.wait(lock, [this] { return stopWorker || !imageQueue.empty(); });
            if (stopWorker && imageQueue.empty())
            {
                break;
            }
        }

        if (imageQueue.pop(rgbImage))
        {
            cout << "Processing image " << rgbImage->getImageId() << endl;
            if (rgbImage->isReadyToMerge())
            {
                OIIO::ImageBuf* mergedImage = ImagesProcessor::mergeRGBImages(rgbImage->getRedImage(), rgbImage->getGreenImage(), rgbImage->getBlueImage());
                if (mergedImage != nullptr)
                {
                    ImagesProcessor::saveImage(mergedImage, "img/image" + rgbImage->getCaptureId() + "_" + to_string(rgbImage->getImageId()) + ".tiff");
                    delete mergedImage;
                }
                else
                {
                    cout << "Error: Merged image is null." << endl;
                }
            }
            else
            {
                cout << "Error: Not all images are ready to be merged." << endl;
            }
            delete rgbImage; // Don't forget to delete the RGBImage object
        }
    }
}

