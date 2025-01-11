#include "RGBImage.h"

/*
* RGB Image is used to hold the 3 images needed to make the master color image
* Once the 3 images are ready, the getMergedImage() method will combine them into a single image
* and return a pointer to it. The caller is responsible for freeing the memory.
*/
RGBImage::RGBImage()
{
	redImage = nullptr;
	greenImage = nullptr;
	blueImage = nullptr;
	imageId = 0;
	captureId = "undefined_id";
}

void RGBImage::setRedImage(OIIO::ImageBuf* redImage)
{
	this->redImage = redImage;
}

void RGBImage::setGreenImage(OIIO::ImageBuf* greenImage)
{
	this->greenImage = greenImage;
}

void RGBImage::setBlueImage(OIIO::ImageBuf* blueImage)
{
	this->blueImage = blueImage;
}

// For testing only, load sample images
void RGBImage::fillWithSampleImages() {
	OIIO::ImageBuf* redChannel = new OIIO::ImageBuf("rgbsample/red.tif");
	OIIO::ImageBuf* greenChannel = new OIIO::ImageBuf("rgbsample/green.tif");
	OIIO::ImageBuf* blueChannel = new OIIO::ImageBuf("rgbsample/blue.tif");

	redImage = redChannel;
	greenImage = greenChannel;
	blueImage = blueChannel;
}

/*
* Check if all 3 images are ready to be merged
*/
bool RGBImage::isReadyToMerge()
{
	return redImage != nullptr && greenImage != nullptr && blueImage != nullptr;
}

RGBImage::~RGBImage()
{
	if (redImage != nullptr) {
		delete redImage;
	}
	if (greenImage != nullptr) {
		delete greenImage;
	}
	if (blueImage != nullptr) {
		delete blueImage;
	}
}
