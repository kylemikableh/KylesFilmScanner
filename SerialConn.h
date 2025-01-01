/*
*   Scanner.cpp
*	Film Scanner Master PC Control Software by Kyle Mikolajczyk
*   kyle@kylem.org
*/

#pragma once
#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;

class SerialConn
{
	public:
		SerialConn(int baudRate, const char* portId);
		~SerialConn();
		char* readBetweenDelimiters(const char startDelim, const char endDelim);
	private:
		io_service io;
		serial_port serial;

		char getCharFromConn();

};

