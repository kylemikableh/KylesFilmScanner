#include "ImagesProcessor.h"

OIIO::ImageBuf* ImagesProcessor::mergeRGBImages(OIIO::ImageBuf* redChannel, OIIO::ImageBuf* greenChannel, OIIO::ImageBuf* blueChannel) {
    // Check for null pointers
    if (!redChannel || !greenChannel || !blueChannel) {
        std::cerr << "Error: One or more input image buffers are null." << std::endl;
        return nullptr;
    }

    // Print image specifications
    std::cout << "Red Channel: " << redChannel->spec().width << "x" << redChannel->spec().height << ", " << redChannel->spec().nchannels << " channels" << std::endl;
    std::cout << "Green Channel: " << greenChannel->spec().width << "x" << greenChannel->spec().height << ", " << greenChannel->spec().nchannels << " channels" << std::endl;
    std::cout << "Blue Channel: " << blueChannel->spec().width << "x" << blueChannel->spec().height << ", " << blueChannel->spec().nchannels << " channels" << std::endl;

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

    //cout << "Image type in image processor: " << spec.format << endl;

    // Check if the output image buffer was created successfully
    if (!rgbImage) {
        std::cerr << "Error: Failed to create output image buffer." << std::endl;
        return nullptr;
    }

    // Get pointers to the image data arrays
    const void* redData = (const float*)redChannel->localpixels();
    const void* greenData = (const float*)greenChannel->localpixels();
    const void* blueData = (const float*)blueChannel->localpixels();
    void * rgbData = (float*)rgbImage->localpixels();

    // // Iterate over each pixel and combine the channels
    // // Very slow method, manipulate memory directly instead, but good reference
    // for (int y = 0; y < spec.height; ++y) {
    //     for (int x = 0; x < spec.width; ++x) {
    //         float r, g, b;
			//r = redChannel->getchannel(x, y, 0, 0); // The 0th channel is the first channel, in this case in a color image its RED
			//g = greenChannel->getchannel(x, y, 0, 0); // Since the image is expected to be monochrome, the 0th channel is the only channel
    //         b = blueChannel->getchannel(x, y, 0, 0);
    //         float rgb[3] = { r, g, b };
    //         //cout << "RGB: " << r << ", " << g << ", " << b << endl;
    //         rgbImage->setpixel(x, y, rgb);
    //     }
    // }

    // Check for null pointers in the data arrays
    if (!redData || !greenData || !blueData || !rgbData) {
        std::cerr << "Error: One or more image data arrays are null." << std::endl;
        delete rgbImage;
        return nullptr;
    }

    // Merge the channels by copying the data directly based on the data type
    // NOTE: uint8 is used here as the image buffs are all 8bit images
    const uint8_t* redData8 = static_cast<const uint8_t*>(redData);
    const uint8_t* greenData8 = static_cast<const uint8_t*>(greenData);
    const uint8_t* blueData8 = static_cast<const uint8_t*>(blueData);
    uint8_t* rgbData8 = static_cast<uint8_t*>(rgbData);

    for (int y = 0; y < spec.height; ++y) {
        for (int x = 0; x < spec.width; ++x) {
            int index = y * spec.width + x;
            rgbData8[index * 3 + 0] = redData8[index];
            rgbData8[index * 3 + 1] = greenData8[index];
            rgbData8[index * 3 + 2] = blueData8[index];
        }
    }

    std::cout << "Combined image buffer successfully." << std::endl;

    return rgbImage;
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
