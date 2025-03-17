/*
*   MDriveConn.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*	kyle@kylem.org
*/

#include "MDriveConn.h"

MDriveConn::MDriveConn(const std::string& port, unsigned int baud_rate)
    : io(), serial(io, port)
{
    serial.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
    serial.set_option(boost::asio::serial_port_base::character_size(8)); // 8 data bits
    serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one)); // 1 stop bit
    serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none)); // No parity
}

void MDriveConn::write(const std::string& data)
{
    boost::asio::write(serial, boost::asio::buffer(data + "\r\n"));
}

std::string MDriveConn::read()
{
    char c;
    std::string result;
    for (;;)
    {
        boost::asio::read(serial, boost::asio::buffer(&c, 1));

        if (c == '?' || c == '>')
            break;
        result += c;
    }
    return result;
}

void MDriveConn::initializeAndHome()
{
    write("PR PN"); // PRint PN (PN = Product Number)

    std::string mDriveModel = read();
    std::cout << "MDrive Detected: " << std::endl << mDriveModel << std::endl;

    std::cout << "Calibrating MDrive..." << std::endl;
    write("EX SS"); // EXecute program SS (SS = Our home position program)

    for (;;) {
        std::string response = read();

        std::cout << "Response: " << response << std::endl;

        // Check if the response contains "Ready."
        if (response.find("Ready.") != std::string::npos)
        {
            std::cout << "MDrive Calibrated and at Home Position." << std::endl;
            return; // Exit the loop if "Ready." is found
        }
    }
}