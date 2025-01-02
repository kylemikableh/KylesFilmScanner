/*
*   SerialConn.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*	kyle@kylem.org
*/

#include "SerialConn.h"

/*
* Constructor for the SerialConn class.
*/
SerialConn::SerialConn(int baudRate, const char* portId) : io(), serial(io, portId)
{
    try {
        serial.set_option(serial_port_base::baud_rate(baudRate));
        serial.set_option(serial_port_base::character_size(8));
        serial.set_option(serial_port_base::parity(serial_port_base::parity::none));
        serial.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        serial.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
    }
    catch (boost::system::system_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
    }
}

/*
* Reads a single character from the serial connection.
*/
char SerialConn::getCharFromConn()
{
	char c;
	boost::asio::read(serial, buffer(&c, 1));
	return c;
}

/*
* Reads from the serial connection until the start and end delimiters are found.
*/
char* SerialConn::readMessage(const char startDelim, const char endDelim)
{
    try {
        char* buffer = new char[256](); // Initialize buffer to zero
        int i = 0;
        bool started = false;

        while (true) {
            char c = getCharFromConn();
            if (c == startDelim) {
                started = true; // Start reading when start delimiter is found
                continue;
            }

            if (started) {
                if (isprint(static_cast<unsigned char>(c)) || c == '\n' || c == '\r') { // Check for valid characters
                    buffer[i] = c;
                    if (buffer[i] == endDelim) {
                        buffer[i] = '\0';
                        break;
                    }
                    i++;
                    if (i >= 255) { // Prevent buffer overflow
                        buffer[255] = '\0';
                        break;
                    }
                }
            }
        }
        return buffer;
    }
    catch (boost::system::system_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return nullptr;
}

void SerialConn::parseMessage(const char* message)
{
    // Determine the message type from the message string
    Arduino_Message_Type messageType;
    int value = -1;

    if (strcmp(message, "ACK") == 0)
    {
        messageType = ACK;
    }
    else if (strcmp(message, "READY") == 0)
    {
        messageType = READY;
    }
    else if (strcmp(message, "READY:RED") == 0)
    {
        messageType = READY_RED;
    }
    else if (strcmp(message, "READY:GREEN") == 0)
    {
        messageType = READY_GREEN;
    }
    else if (strcmp(message, "READY:BLUE") == 0)
    {
        messageType = READY_BLUE;
    }
    else if (strcmp(message, "READY:FRAME") == 0)
    {
        messageType = READY_FRAME;
    }
    else if (strncmp(message, "CURRENT_FRAME_ID:", 17) == 0)
    {
        messageType = CURRENT_FRAME_ID;
        char* endPtr;
        value = strtol(message + 17, &endPtr, 10); // Extract the number after "CURRENT_FRAME_ID:", 10 here is base10 number system
        if (*endPtr != '\0') {
            std::cerr << "Invalid number format in message: " << message << std::endl;
            return;
        }
    }
    else if (strncmp(message, "STEPPER_POS:", 12) == 0)
    {
        messageType = STEPPER_POS;
        char* endPtr;
        value = strtol(message + 12, &endPtr, 10); // Extract the number after "CURRENT_FRAME_ID:", 10 here is base10 number system
        if (*endPtr != '\0') {
            std::cerr << "Invalid number format in message: " << message << std::endl;
            return;
        }
    }
    else
    {
        std::cerr << "Unknown message received from Arduino: " << message << std::endl;
        return;
    }

    // Handle the message
    handleArduinoMessage(messageType, value);
}

void SerialConn::handleArduinoMessage(Arduino_Message_Type messageType, int value)
{
    switch (messageType)
    {
    case ACK:
        std::cout << "Received ACK from Arduino" << std::endl;
        // Handle ACK message
        break;
    case READY:
        std::cout << "Arduino is ready to receive a command" << std::endl;
        // Handle READY message
        break;
    case READY_RED:
        std::cout << "Arduino is ready to scan RED color" << std::endl;
        // Handle READY_RED message
        break;
    case READY_GREEN:
        std::cout << "Arduino is ready to scan GREEN color" << std::endl;
        // Handle READY_GREEN message
        break;
    case READY_BLUE:
        std::cout << "Arduino is ready to scan BLUE color" << std::endl;
        // Handle READY_BLUE message
        break;
    case READY_FRAME:
        std::cout << "Frame ready to be captured (all colors)" << std::endl;
        // Handle READY_FRAME message
        break;
    case CURRENT_FRAME_ID:
        std::cout << "Current Frame ID received: " << value << std::endl;
        // Handle CURRENT_FRAME_ID message with frameId
        break;
    case STEPPER_POS:
        std::cout << "Stepper position received: " << value << std::endl;
        // Handle STEPPER_POS message
        break;
    default:
        std::cerr << "Unknown message type received from Arduino" << std::endl;
        break;
    }
}


SerialConn::~SerialConn()
{
    // Destructor body can be empty as the io_service and serial_port
    // will automatically clean up their resources.
}
