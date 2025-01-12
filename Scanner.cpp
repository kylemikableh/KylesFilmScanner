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

bool useCamera = true;
bool enableSerialComms = false;

/*
* Create the main Image Controller to handle the scans for this ID. 
* (ID will we configurable later)
*/
ImageCaptureController* initializeImageController() {
    ImageCaptureController* imageCaptureController = nullptr;
    ImageCaptureController::initializePylon();
    imageCaptureController = new ImageCaptureController("EK00001");
    
    return imageCaptureController;
}

/*
* If we have enabled the serial connection, initialize it.
* If its not available, bail out
*/
SerialConn* getArudinoConnection() {
    SerialConn* arduinoConnection = nullptr;
    if (enableSerialComms) {
        try {
            arduinoConnection = new SerialConn(ARDUINO_BAUD_RATE, ARDUINO_PORT);
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to open serial port: " << ARDUINO_PORT << ". Please check connection or change port." << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
    return arduinoConnection;
}

int main(int argc, char* argv[])
{
    int exitCode = 0;

    SerialConn* arduinoConnection = getArudinoConnection();

    ImageCaptureController* imageCaptureController;

    if (useCamera) {
        imageCaptureController = initializeImageController();
        imageCaptureController->captureFrame();
        imageCaptureController->captureFrame();
        imageCaptureController->captureFrame();
        imageCaptureController->captureFrame();
        imageCaptureController->captureFrame();
        imageCaptureController->captureFrame();
        imageCaptureController->captureFrame();
        imageCaptureController->captureFrame();
        imageCaptureController->captureFrame();
    }

    int i = 0;
    while (enableSerialComms) {
        if (i > 10) { break; }
        cout << "Sent command to set color to RED" << endl;
        // Test out sending command to arduino with pause
		if (i == 0) {
			arduinoConnection->sendCommand(SerialConn::SET_COLOR_RED);
		}
		else if (i == 1) {
			arduinoConnection->sendCommand(SerialConn::SET_COLOR_BLUE);
		}
		else if (i == 2) {
			arduinoConnection->sendCommand(SerialConn::GET_FRAME_ID);
		}
		else if (i == 3) {
			arduinoConnection->sendCommand(SerialConn::RESET_FRAME_ID);
		}
		else {
			arduinoConnection->sendCommand(SerialConn::SET_COLOR_GREEN);
		}
		arduinoConnection->sendCommand(SerialConn::SET_COLOR_RED);

		cout << "Waiting for response" << endl;
        char* buffer = arduinoConnection->readMessage(ARDUINO_MSG_START_DELIM, ARDUINO_MSG_END_DELIM);
		cout << "Response recieved" << endl;
        if (buffer != nullptr) {
            cout << buffer << endl;
            arduinoConnection->parseMessage(buffer);
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
