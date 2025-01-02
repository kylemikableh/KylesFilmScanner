/*
*   ImageCaptureController.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*	kyle@kylem.org
*/

#include "ImageCaptureController.h"

// Initialize the static member variable
bool ImageCaptureController::pylonInitialized = false;

void ImageCaptureController::initializePylon()
{
    if (!pylonInitialized)
    {
        PylonInitialize();
        pylonInitialized = true;
    }
}

ImageCaptureController::ImageCaptureController(std::string id) : camera(CTlFactory::GetInstance().CreateFirstDevice()), captureId(id)
{   
    // Print the model name of the camera.
    cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

    // The parameter MaxNumBuffer can be used to control the count of buffers
    // allocated for grabbing. The default value of this parameter is 10.
    camera.MaxNumBuffer = 5;

    // Start the grabbing of c_countOfImagesToGrab images.
    // The camera device is parameterized with a default configuration which
    // sets up free-running continuous acquisition.
	//camera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
}

ImageCaptureController::~ImageCaptureController()
{
	// Releases all pylon resources.
	PylonTerminate();
}

/*
* Captures a frame by capturing the red, green, and blue images.
* This will also check with the arduino and / or set the colors on the arduino before each step of the process.
*/
int ImageCaptureController::captureFrame()
{
	// Capture the red image
	captureImage(RED);
	// Capture the green image
	captureImage(GREEN);
	// Capture the blue image
	captureImage(BLUE);
    imgPosition++;
	return 0;
}

int ImageCaptureController::captureImage(ImageType type)
{
    try
    {
        // Ensure the camera is open and ready to grab images
        if (!camera.IsOpen())
        {
			cout << "Opening camera..." << endl;
            camera.Open();
        }
        // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
        //camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);
        camera.GrabOne(5000, ptrGrabResult, TimeoutHandling_ThrowException);

        // Image grabbed successfully?
        if (ptrGrabResult->GrabSucceeded())
        {
            // Access the image data.
            /*cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
            cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;*/
            cout << "Grabbed image: " << imgPosition << endl;
            cout << "Image buffer size: " << ptrGrabResult->GetBufferSize() << endl;
            const uint8_t* pImageBuffer = (uint8_t*)ptrGrabResult->GetBuffer();
            /* cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << endl << endl;*/

            // Determine the file name based on the image type
            std::string color;
            switch (type)
            {
            case RED:
                color = "RED";
                break;
            case GREEN:
                color = "GREEN";
                break;
            case BLUE:
                color = "BLUE";
                break;
            }

            string filename = "img/image" + captureId + "_" + to_string(imgPosition) + "_" + color + ".tiff";

            // Save the image to a file
            Pylon::CImagePersistence::Save(Pylon::ImageFileFormat_Tiff, filename.c_str(), ptrGrabResult);


            #ifdef PYLON_WIN_BUILD
            // Display the grabbed image.
            Pylon::DisplayImage(1, ptrGrabResult);
            #endif
        }
        else
        {
            cout << "Error: " << std::hex << ptrGrabResult->GetErrorCode() << std::dec << " " << ptrGrabResult->GetErrorDescription() << endl;
        }

        return 0;
    }
    catch (const GenericException& e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl
            << e.GetDescription() << endl;
        return 1;
    }
}

