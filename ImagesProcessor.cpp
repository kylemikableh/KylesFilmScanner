#include "ImagesProcessor.h"

OIIO::ImageBuf* ImagesProcessor::mergeRGBImages(OIIO::ImageBuf* redChannel, OIIO::ImageBuf* greenChannel, OIIO::ImageBuf* blueChannel) {

    // Print image specifications
    std::cout << "Red Channel: " << redChannel->spec().width << "x" << redChannel->spec().height << ", " << redChannel->spec().nchannels << " channels" << std::endl;
    std::cout << "Green Channel: " << greenChannel->spec().width << "x" << greenChannel->spec().height << ", " << greenChannel->spec().nchannels << " channels" << std::endl;
    std::cout << "Blue Channel: " << blueChannel->spec().width << "x" << blueChannel->spec().height << ", " << blueChannel->spec().nchannels << " channels" << std::endl;

    // Create an empty image buffer for the final RGB image
    OIIO::ImageSpec spec = redChannel->spec();
    spec.nchannels = 3; // Set the number of channels to 3 (RGB)
    OIIO::ImageBuf* rgbImage = new OIIO::ImageBuf(spec);

    // Iterate over each pixel and combine the channels
    for (int y = 0; y < spec.height; ++y) {
        for (int x = 0; x < spec.width; ++x) {
            float r, g, b;
			r = redChannel->getchannel(x, y, 0, 0); // The 0th channel is the first channel, in this case in a color image its RED
			g = greenChannel->getchannel(x, y, 0, 0); // Since the image is expected to be monochrome, the 0th channel is the only channel
            b = blueChannel->getchannel(x, y, 0, 0);
            float rgb[3] = { r, g, b };
            //cout << "RGB: " << r << ", " << g << ", " << b << endl;
            rgbImage->setpixel(x, y, rgb);
        }
    }

    // Write the combined image to a .tiff file
    /*if (!rgbImage.write("rgbsample/combined_image.jpeg")) {
        std::cerr << "Error writing combined image." << std::endl;
        return 1;
    }
    */

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
