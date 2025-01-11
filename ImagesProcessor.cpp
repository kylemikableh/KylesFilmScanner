/*
*   ImagesProcessor.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*   kyle@kylem.org
*/

#include "ImagesProcessor.h"

/*
* Create a master full color/bitdepth from the 3 mono16 ImageBufs,
* complete with all post-processing
*/
OIIO::ImageBuf* ImagesProcessor::createProcessedRGBImage(OIIO::ImageBuf* redChannel, OIIO::ImageBuf* greenChannel, OIIO::ImageBuf* blueChannel) {
    // Check for null pointers
    if (!redChannel || !greenChannel || !blueChannel) {
        std::cerr << "Error: One or more input image buffers are null." << std::endl;
        return nullptr;
    }

    // Ensure all channels have the same dimensions
    if (redChannel->spec().width != greenChannel->spec().width || redChannel->spec().width != blueChannel->spec().width ||
        redChannel->spec().height != greenChannel->spec().height || redChannel->spec().height != blueChannel->spec().height) {
        std::cerr << "Error: Input image buffers have different dimensions." << std::endl;
        return nullptr;
    }

    // Create an empty image buffer for the final RGB image
    OIIO::ImageSpec spec = redChannel->spec();
    spec.nchannels = 3; // Set the number of channels to 3 (RGB)
    OIIO::ImageBuf* rgbImage = new OIIO::ImageBuf(spec);


    // Check if the output image buffer was created successfully
    if (!rgbImage) {
        std::cerr << "Error: Failed to create output image buffer." << std::endl;
        return nullptr;
    }

    // Get pointers to the image data arrays
    const void* redData = redChannel->localpixels();
    const void* greenData = greenChannel->localpixels();
    const void* blueData = blueChannel->localpixels();
    void * rgbData = (float*)rgbImage->localpixels();

    // Check for null pointers in the data arrays
    if (!redData || !greenData || !blueData || !rgbData) {
        std::cerr << "Error: One or more image data arrays are null." << std::endl;
        delete rgbImage;
        return nullptr;
    }

    // Merge the channels by copying the data directly based on the data type
    mergeChannels(static_cast<const uint16_t*>(redData), static_cast<const uint16_t*>(greenData), static_cast<const uint16_t*>(blueData), static_cast<uint16_t*>(rgbData), spec.width, spec.height);

    // TODO: More Image Processing here, to the rgbData array now

    std::cout << "Combined image buffer successfully." << std::endl;

    return rgbImage;
}

/*
* Take 3 arrays of the image data (16bit scaled) and merge them into the master rgbData 
*/
void ImagesProcessor::mergeChannels(const uint16_t* redData, const uint16_t* greenData, const uint16_t* blueData, uint16_t* rgbData, int width, int height) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            rgbData[index * 3 + 0] = redData[index];
            rgbData[index * 3 + 1] = greenData[index];
            rgbData[index * 3 + 2] = blueData[index];
        }
    }
}

/*
* Save an image to a file, with the extension determining the file type
*/
bool ImagesProcessor::saveImage(OIIO::ImageBuf* image, std::string filename) {
	if (!image->write(filename)) {
		std::cerr << "Error writing image to file." << std::endl;
		return false;
	}
	return true;
}
