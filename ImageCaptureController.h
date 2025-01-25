/*
*   ImageCaptureController.h
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*   kyle@kylem.org
*/

#pragma once

#include <OpenImageIO/imagebuf.h>
// Include files to use the pylon API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif
#include <pylon/BaslerUniversalInstantCamera.h>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "RGBImage.h"
#include "RGBImageQueue.h"

using namespace std;
using namespace Pylon;

class ImageCaptureController
{
	public:
		enum ImageType { RED, GREEN, BLUE }; // Define the enum for image types
		static void initializePylon(); // Static method to initialize Pylon
		void initializeCamera();
		ImageCaptureController(std::string id);
		~ImageCaptureController();
		int captureFrame(); // Will get all colors for 1 frame
		
	private:

		static bool pylonInitialized; // Static flag to check if Pylon is initialized

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;
		CInstantCamera camera;

		int lastImageId;
		std::string captureId;

		RGBImageQueue<RGBImage> imageQueue;
		std::thread workerThread;
		std::atomic<bool> stopWorker;
		std::condition_variable stopCondition;
		std::mutex stopMutex;

		std::vector<uint16_t> scale12BitTo16Bit(const uint16_t* pImageBuffer, int width, int height);
		void processQueue();
		OIIO::ImageBuf* captureImageAsBuffer();
		void manuallyStepThroughImage();

		Pylon::CPylonImageWindow window;
};

