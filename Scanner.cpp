/*
*   Scanner.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*   kyle@kylem.org
*/

#define ARDUINO_BAUD_RATE 115200
#define ARDUINO_PORT "COM6"
#define ARDUINO_MSG_START_DELIM '('
#define ARDUINO_MSG_END_DELIM ')'

#include "SerialConn.h"
#include "ImageCaptureController.h"

#include <OpenImageIO/imagebuf.h>

bool useCamera = false;

ImageCaptureController* initializeImageController() {
    ImageCaptureController::initializePylon();
    ImageCaptureController* imageCaptureController = new ImageCaptureController("EK00001");
    return imageCaptureController;
}

int testRgbMerge() {
    // Load the individual color channel images
    OIIO::ImageBuf redChannel("rgbsample/red.tif");
    OIIO::ImageBuf greenChannel("rgbsample/green.tif");
    OIIO::ImageBuf blueChannel("rgbsample/blue.tif");

    cout << "here";

    // Check if the images are loaded correctly
    if (!redChannel.init_spec("rgbsample/red.tif", 0, 0) ||
        !greenChannel.init_spec("rgbsample/green.tif", 0, 0) ||
        !blueChannel.init_spec("rgbsample/blue.tif", 0, 0)) {
        std::cerr << "Error loading images." << std::endl;
        return 1;
    }

    // Print image specifications
    std::cout << "Red Channel: " << redChannel.spec().width << "x" << redChannel.spec().height << ", " << redChannel.spec().nchannels << " channels" << std::endl;
    std::cout << "Green Channel: " << greenChannel.spec().width << "x" << greenChannel.spec().height << ", " << greenChannel.spec().nchannels << " channels" << std::endl;
    std::cout << "Blue Channel: " << blueChannel.spec().width << "x" << blueChannel.spec().height << ", " << blueChannel.spec().nchannels << " channels" << std::endl;


    cout << "here2";

    // Create an empty image buffer for the final RGB image
    OIIO::ImageSpec spec = redChannel.spec();
    spec.nchannels = 3; // Set the number of channels to 3 (RGB)
    OIIO::ImageBuf rgbImage(spec);

    // Initialize the image buffer with zeros

    cout << "here3";

    // Iterate over each pixel and combine the channels
    for (int y = 0; y < spec.height; ++y) {
        for (int x = 0; x < spec.width; ++x) {
            float r, g, b;
            r = redChannel.getchannel(x, y, 0, 0);
            g = greenChannel.getchannel(x, y, 0, 1);
            b = blueChannel.getchannel(x, y, 0, 2);
            float rgb[3] = { r, g, b };
			//cout << "RGB: " << r << ", " << g << ", " << b << endl;
            rgbImage.setpixel(x, y, rgb);
        }
    }

    cout << "here4";

    // Write the combined image to a .tiff file
    if (!rgbImage.write("rgbsample/combined_image.jpeg")) {
        std::cerr << "Error writing combined image." << std::endl;
        return 1;
    }

    std::cout << "Combined image created successfully." << std::endl;
    return 0;
}


int main(int argc, char* argv[])
{
    int exitCode = 0;

    testRgbMerge();

    // Comment the following two lines to disable waiting on exit.
    /*cerr << endl << "Press enter to exit." << endl;
    while (cin.get() != '\n');*/

	SerialConn arduinoConnection = SerialConn(ARDUINO_BAUD_RATE, ARDUINO_PORT);

    ImageCaptureController* imageCaptureController;

    if (useCamera) {
        imageCaptureController = initializeImageController();
        imageCaptureController->captureFrame();
        imageCaptureController->captureFrame();
        imageCaptureController->captureFrame();
    }

    int i = 0;
    while (true) {
        if (i > 10) { break; }
        cout << "Sent command to set color to RED" << endl;
        // Test out sending command to arduino with pause
		if (i == 0) {
			arduinoConnection.sendCommand(SerialConn::SET_COLOR_RED);
		}
		else if (i == 1) {
			arduinoConnection.sendCommand(SerialConn::SET_COLOR_BLUE);
		}
		else if (i == 2) {
			arduinoConnection.sendCommand(SerialConn::GET_FRAME_ID);
		}
		else if (i == 3) {
			arduinoConnection.sendCommand(SerialConn::RESET_FRAME_ID);
		}
		else {
			arduinoConnection.sendCommand(SerialConn::SET_COLOR_GREEN);
		}
		arduinoConnection.sendCommand(SerialConn::SET_COLOR_RED);

		cout << "Waiting for response" << endl;
        char* buffer = arduinoConnection.readMessage(ARDUINO_MSG_START_DELIM, ARDUINO_MSG_END_DELIM);
		cout << "Response recieved" << endl;
        if (buffer != nullptr) {
            cout << buffer << endl;
            arduinoConnection.parseMessage(buffer);
        }
        else {
            cout << "No response recieved for try " << i << endl;
        }

		Sleep(25);
        i++;
    }

	if (useCamera) {
		delete imageCaptureController; // Clean up the dynamically allocated memory
	}

    return exitCode;
}
