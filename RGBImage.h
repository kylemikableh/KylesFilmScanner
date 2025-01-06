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
		bool isReadyToMerge();

		void fillWithSampleImages();
	private:
		// These will contain the 3 images needed to make the master color image
		OIIO::ImageBuf* redImage;
		OIIO::ImageBuf* greenImage;
		OIIO::ImageBuf* blueImage;
};

