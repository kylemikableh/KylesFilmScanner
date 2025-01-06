#pragma once

#include <OpenImageIO/imagebuf.h>
#include <iostream>

using namespace std;

class ImagesProcessor
{
	public:
		static OIIO::ImageBuf* mergeRGBImages(OIIO::ImageBuf* redChannel, OIIO::ImageBuf* greenChannel, OIIO::ImageBuf* blueChannel);
		static bool saveImage(OIIO::ImageBuf* image, std::string filename);
};

