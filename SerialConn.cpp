/*
*   SerialConn.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*	kyle@kylem.org
*/

#include "SerialConn.h"

/*
* Constructor for the SerialConn class.
*/
SerialConn::SerialConn(int baudRate, const char* portId) : io(), work(io), serial(io, portId)
{
    try {
        serial.set_option(serial_port_base::baud_rate(baudRate));
        serial.set_option(serial_port_base::character_size(8));
        serial.set_option(serial_port_base::parity(serial_port_base::parity::none));
        serial.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        serial.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));

        // Start the io_service in a separate thread
        ioThread = std::thread([this]() { io.run(); });
    }
    catch (boost::system::system_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
    }
}

void SerialConn::checkDeadline(boost::asio::deadline_timer* timer, boost::asio::serial_port* serial)
{
    if (timer->expires_at() <= boost::asio::deadline_timer::traits_type::now())
    {
        timed_out = true;
        boost::system::error_code ignored_ec;
        serial->cancel(ignored_ec);
        timer->expires_at(boost::posix_time::pos_infin);
    }

    timer->async_wait([this, timer, serial](const boost::system::error_code& ec)
        {
            if (!ec)
            {
                checkDeadline(timer, serial);
            }
        });
}


/*
* Reads a single character from the serial connection.
*/
char SerialConn::getCharFromConn()
{
    //std::cout << "Reading char" << std::endl;
    readComplete = false;
    timed_out = false;
    boost::asio::deadline_timer timer(io);
    timer.expires_from_now(boost::posix_time::seconds(5)); // Set timeout duration

    checkDeadline(&timer, &serial);

    serial.async_read_some(boost::asio::buffer(&readChar, 1),
        [this, &timer](const boost::system::error_code& ec, std::size_t bytes_transferred)
        {
            if (!ec && bytes_transferred > 0)
            {
                timer.expires_at(boost::posix_time::pos_infin); // Cancel the timer
                readComplete = true;
            }
            else if (ec != boost::asio::error::operation_aborted)
            {
                std::cerr << "Error: " << ec.message() << std::endl;
            }
        });
    //std::cout << "Done reading char" << std::endl;

    // Run the I/O service to process the asynchronous operation
    while (!readComplete && !timed_out)
    {
        //std::cout << "In while loop" << std::endl;
        if (io.poll_one() == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
	//std::cout << "Out of while loop" << std::endl;

    // Ensure the timer is canceled and no longer active
    boost::system::error_code ignored_ec;
    timer.cancel(ignored_ec);

    if (timed_out)
    {
        std::cerr << "Timeout reading from serial connection." << std::endl;
        return '\t'; // Return a null character in case of timeout
    }

    if (readComplete)
    {
        // NOTE: When debugging you can remove the comment to see the raw chars being received
        //std::cout << "Finished reading char: " << readChar << std::endl;
        return readChar;
    }

    return '\t'; // Return a null character in case of error
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

        //std::cout << "Starting read message..." << std::endl;

        while (true) {
			//std::cout << "Reading char" << std::endl;
            char c = getCharFromConn(); // NOTE: Can hang here
			//std::cout << c << std::endl;
            
			if (c == '\t') {
				std::cerr << "Error reading from serial connection." << std::endl;
				return nullptr;
			}

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
        std::cout << "error happened";
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
    else if (strcmp(message, "READY_RED") == 0)
    {
        messageType = READY_RED;
    }
    else if (strcmp(message, "READY_GREEN") == 0)
    {
        messageType = READY_GREEN;
    }
    else if (strcmp(message, "READY_BLUE") == 0)
    {
        messageType = READY_BLUE;
    }
    else if (strcmp(message, "READY_FRAME") == 0)
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
        messageType = CURRENT_STEPPER_POS;
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
    case CURRENT_STEPPER_POS:
        std::cout << "Stepper position received: " << value << std::endl;
        // Handle STEPPER_POS message
        break;
    default:
        std::cerr << "Unknown message type received from Arduino" << std::endl;
        break;
    }
}

void SerialConn::printToSerialWithDelimiters(const char* message)
{
    // Buffer to hold the formatted message
    char formattedMessage[MSG_SIZE];
    for (int i = 0; i < MSG_SIZE; i++) {
        formattedMessage[i] = 0;
    }
    // Add the start delimiter to the message
    formattedMessage[0] = MSG_START_DELIM;
    int i = 1;
    // Copy the message to the buffer
    for (int j = 0; j < strlen(message); j++) {
        formattedMessage[i] = message[j];
        i++;
    }
    // Add the end delimiter to the message
    formattedMessage[i] = MSG_END_DELIM;
    // Write the message to the serial connection
    boost::asio::write(serial, buffer(formattedMessage, MSG_SIZE));
}

/*
    * Messag Types TO Arduino
    * SET_COLOR:RED
    * SET_COLOR:GREEN
    * SET_COLOR:BLUE
    * GOTO_FRAME_ID:0- Frame ID; This is to advance (or rewind) to the given frame ID
    * FRAME_STEP:0- Frame step; This is to advance (or rewind) the given number of frames
    * GOTO_STEPPER_POS:0- Stepper position; This is to move the stepper to the given position
    * GET_FRAME_ID:0- Get frame ID; This is to get the current frame ID
    * GET_STEPPER_POS:0- Get stepper position; This is to get the current stepper position
    * SET_FRAME_OFFSET:- Set frame offset; This is to set the frame offset
    * RESET_FRAME_ID- Reset frame ID; This is to reset the frame ID to the given value
    */

void SerialConn::sendCommand(Arduino_Command_Type command)
{
	// Simple overload to send a command without a required value
	sendCommand(command, 0);
}

void SerialConn::sendCommand(Arduino_Command_Type command, int value)
{
    std::string enrichedCommand;
    switch (command)
    {
    case SET_COLOR_RED:
        printToSerialWithDelimiters("SET_COLOR_RED");
        break;
    case SET_COLOR_GREEN:
        printToSerialWithDelimiters("SET_COLOR_GREEN");
        break;
    case SET_COLOR_BLUE:
        printToSerialWithDelimiters("SET_COLOR_BLUE");
        break;
    case GOTO_FRAME_ID:
        enrichedCommand = "GOTO_FRAME_ID:" + std::to_string(value);
        printToSerialWithDelimiters(enrichedCommand.c_str());
        break;
    case FRAME_STEP:
       enrichedCommand = "FRAME_STEP:" + std::to_string(value);
        printToSerialWithDelimiters(enrichedCommand.c_str());
        break;
    case GOTO_STEPPER_POS:
        enrichedCommand = "GOTO_STEPPER_POS:" + std::to_string(value);
        printToSerialWithDelimiters(enrichedCommand.c_str());
        break;
    case GET_FRAME_ID:
        printToSerialWithDelimiters("GET_FRAME_ID");
        break;
    case GET_STEPPER_POS:
        printToSerialWithDelimiters("GET_STEPPER_POS");
        break;
    case SET_FRAME_OFFSET:
        enrichedCommand = "SET_FRAME_OFFSET:" + std::to_string(value);
        printToSerialWithDelimiters(enrichedCommand.c_str());
        break;
    case RESET_FRAME_ID:
        printToSerialWithDelimiters("RESET_FRAME_ID");
        break;
    default:
        std::cerr << "Unknown command type received" << std::endl;
        break;
    }

}


SerialConn::~SerialConn()
{
    // Destructor body can be empty as the io_service and serial_port
    // will automatically clean up their resources.
    io.stop();
    if (ioThread.joinable()) {
        ioThread.join();
    }
}
