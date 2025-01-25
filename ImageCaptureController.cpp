/*
*   ImageCaptureController.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*	kyle@kylem.org
*/

#define REQUIRE_MANUAL_STEP false

#include "ImageCaptureController.h"

// Initialize the static member variable
bool ImageCaptureController::pylonInitialized = false;

/*
* 
*/
void ImageCaptureController::initializePylon()
{
    if (!pylonInitialized)
    {
        PylonInitialize();
        pylonInitialized = true;;
    }
}

/*
* 
*/
ImageCaptureController::ImageCaptureController(std::string id) : captureId(id), stopWorker(false), lastImageId(0)
{   
    try {
        camera.Attach(CTlFactory::GetInstance().CreateFirstDevice());

        // Print the model name of the camera.
        cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        camera.MaxNumBuffer = 5;

        // Start the worker thread
        workerThread = std::thread(&ImageCaptureController::processQueue, this);
        initializeCamera();

        // Pre-allocate buffers and start grabbing
        camera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByUser);
    }
    catch (const GenericException& e)
    {
        cerr << "Pylon Camera was not found, please check the connection." << endl
            << e.GetDescription() << endl;
        std::exit(EXIT_FAILURE);
    }
}

/*
* Setup the camera and various parameters to configure
* that is different from the default values (such as set it to 12bit mode
*/
void ImageCaptureController::initializeCamera()
{
    try
    {
        // Ensure the camera is open
        if (!camera.IsOpen())
        {
            cout << "Opening camera..." << endl;
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
            cout << "Mono12 pixel format not available. Cannot proceed." << endl;
            std::exit(EXIT_FAILURE);
        }

    }
    catch (const GenericException& e)
    {
        cerr << "An exception occurred while initializing the camera." << endl
            << e.GetDescription() << endl;
    }
    // Create a window and set its size
    window.Create(1);

}

/*
* Manually step through the image capture process. Used to debug and manually swap colors.
*/ 
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

    // Push the RGBImage object to the queue
    imageQueue.push(rgbImage);

    // Notify the worker thread that a new image is available
    stopCondition.notify_all();

    lastImageId++;
    return 0;
}

/*
* Capture a single image from the Basler camera. From here convert the basler image type
* to an OIIO image type that can be manipulated better. Also, if this is a windows 
* computer we can view the image through the Basler DisplayImage function.
*/
OIIO::ImageBuf* ImageCaptureController::captureImageAsBuffer()
{
    OIIO::ImageBuf* image = nullptr; // Image to return
    try
    {
        // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
        camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

        // Image grabbed successfully?
        if (ptrGrabResult->GrabSucceeded())
        {
            cout << "Grabbed image: " << lastImageId << endl;
            cout << "Image buffer size: " << ptrGrabResult->GetBufferSize() << endl;

            // TODO: Offload all this image processing to the "display" function to the processing queue
            // so that we do not bog down the main capture thread
            const uint16_t* pImageBuffer = (uint16_t*)ptrGrabResult->GetBuffer();

            // Get image specifications
            int width = ptrGrabResult->GetWidth();
            int height = ptrGrabResult->GetHeight();

			// Number of channels in the image
            int nchannels = 1; // Assumed monochrome

            EPixelType pixelType = ptrGrabResult->GetPixelType();

            // Debugging information
           /* cout << "Pixel type value: " << static_cast<int>(pixelType) << endl;
            cout << "PixelType_Mono16 value: " << static_cast<int>(PixelType_Mono12) << endl;
            cout << "Comparison result: " << (pixelType == PixelType_Mono12) << endl;*/


            // Determine the data type based on the pixel type
            OIIO::TypeDesc dataType = OIIO::TypeDesc::UINT16; // Cast to 16 because OIIO does not have 12bit

            // Create an ImageSpec
            OIIO::ImageSpec spec(width, height, nchannels, dataType);

            // Create an ImageBuf from the raw image data
            image = new OIIO::ImageBuf(spec);

            // Scale the 12-bit data to the full 16-bit range
            std::vector<uint16_t> scaledBuffer = scale12BitTo16Bit(pImageBuffer, width, height);

            image->set_pixels(OIIO::ROI::All(), dataType, scaledBuffer.data());

            #ifdef PYLON_WIN_BUILD
            window.SetImage(ptrGrabResult);
            window.Show();
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

/*
* The camera is set to 12bit mode but OIIO does not support 12bit, only 8 or 16, thus
* we will use a 16bit container with the 12bit data, so we have to scale it to the 
* correct range or else the image wil appear much darker
*/
std::vector<uint16_t> ImageCaptureController::scale12BitTo16Bit(const uint16_t* pImageBuffer, int width, int height)
{
    std::vector<uint16_t> scaledBuffer(width * height);
    for (int i = 0; i < width * height; ++i)
    {
        scaledBuffer[i] = pImageBuffer[i] << 4; // Left shift by 4 bits to scale to 16-bit range
    }
    return scaledBuffer;
}

/*
* In a seperate thread than the main application, process the mono images into the final 
* full color full bit image, and write to disk
*/
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
                OIIO::ImageBuf* mergedImage = ImagesProcessor::createProcessedRGBImage(rgbImage->getRedImage(), rgbImage->getGreenImage(), rgbImage->getBlueImage());
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

ImageCaptureController::~ImageCaptureController()
{
    {
        std::lock_guard<std::mutex> lock(stopMutex);
        stopWorker = true;
    }
    stopCondition.notify_all();
    workerThread.join();
    if (camera.IsGrabbing())
    {
        camera.StopGrabbing();
    }
    //PylonTerminate();
}

