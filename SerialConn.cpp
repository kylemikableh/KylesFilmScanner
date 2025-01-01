#include "SerialConn.h"

/*
*   SerialConn.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*	kyle@kylem.org
*/

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
char* SerialConn::readBetweenDelimiters(const char startDelim, const char endDelim)
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


SerialConn::~SerialConn()
{
    // Destructor body can be empty as the io_service and serial_port
    // will automatically clean up their resources.
}
