/*
*   Scanner.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*   kyle@kylem.org
*/

#define ARDUINO_BAUD_RATE 9600
#define ARDUINO_PORT "COM6"
#define ARDUINO_MSG_START_DELIM '\r'
#define ARDUINO_MSG_END_DELIM '\n'

#include "SerialConn.h"
#include "ImageCaptureController.h"


int main(int argc, char* argv[])
{
    int exitCode = 0;


	ImageCaptureController::initializePylon();
	ImageCaptureController imageCaptureController = ImageCaptureController("EK00001");
	imageCaptureController.captureFrame();
    imageCaptureController.captureFrame();
    imageCaptureController.captureFrame();

    // Comment the following two lines to disable waiting on exit.
    cerr << endl << "Press enter to exit." << endl;
    while (cin.get() != '\n');

	SerialConn arduinoConnection = SerialConn(ARDUINO_BAUD_RATE, ARDUINO_PORT);
	char* buffer = arduinoConnection.readBetweenDelimiters(ARDUINO_MSG_START_DELIM, ARDUINO_MSG_END_DELIM);
    if (buffer != nullptr) {
        cout << buffer << endl;
    }
    else {
		cout << "Error reading from Arduino." << endl;
    }

    return exitCode;
}
