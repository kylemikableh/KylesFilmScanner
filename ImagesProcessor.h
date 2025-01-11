/*
*   ImageProcessor.h
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*   kyle@kylem.org
*/

#pragma once

#include <OpenImageIO/imagebuf.h>
#include <iostream>

using namespace std;

class ImagesProcessor
{
	public:
		static OIIO::ImageBuf* createProcessedRGBImage(OIIO::ImageBuf* redChannel, OIIO::ImageBuf* greenChannel, OIIO::ImageBuf* blueChannel);
		static bool saveImage(OIIO::ImageBuf* image, std::string filename);
	private:
		static void mergeChannels(const uint16_t* redData, const uint16_t* greenData, const uint16_t* blueData, uint16_t* rgbData, int width, int height);
};

