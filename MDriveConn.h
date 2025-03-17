/*
*   MDriveConn.h
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*	kyle@kylem.org
*/

#include <boost/asio.hpp>
#include <iostream>
#include <string>

class MDriveConn
{
    public:
        MDriveConn(const std::string& port, unsigned int baud_rate);
        void write(const std::string& data);
        std::string read();

        void initializeAndHome();

    private:
        boost::asio::io_service io;
        boost::asio::serial_port serial;
};