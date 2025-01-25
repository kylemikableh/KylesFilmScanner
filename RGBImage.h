#pragma once

#include <OpenImageIO/imagebuf.h>
#include <iostream>
#include "ImagesProcessor.h"

class RGBImage
{
	public:
		RGBImage();
		~RGBImage();
		void setRedImage(OIIO::ImageBuf* redImage);
		void setGreenImage(OIIO::ImageBuf* greenImage);
		void setBlueImage(OIIO::ImageBuf* blueImage);

		OIIO::ImageBuf* getRedImage() { return redImage; }
		OIIO::ImageBuf* getGreenImage() { return greenImage; }
		OIIO::ImageBuf* getBlueImage() { return blueImage; }

		void setImageId(int id) { imageId = id; }
		void setCaptureId(std::string id) { captureId = id; }
		int getImageId() { return imageId; }
		std::string getCaptureId() { return captureId; }

		bool isReadyToMerge();

		void fillWithSampleImages();
	private:
		int imageId;
		std::string captureId;
		// These will contain the 3 images needed to make the master color image
		OIIO::ImageBuf* redImage;
		OIIO::ImageBuf* greenImage;
		OIIO::ImageBuf* blueImage;
};

