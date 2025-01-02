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
	/*
	* Message Types FROM Arduino
	* ACK- Acknowledge command from PC
	* READY- Arduino is ready to receive a command
	* READY:RED- Arduino is ready to scan RED color
	* READY:GREEN- Arduino is ready to scan GREEN color
	* READY:BLUE- Arduino is ready to scan BLUE color
	* READY:FRAME- Frame ready to be captured (all colors)
	* CURRENT_FRAME_ID:0- Frame ID
	* STEPPER_POS:0- Stepper position
	*/

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
	public:
		enum Arduino_Message_Type { ACK, READY, READY_RED, READY_GREEN, READY_BLUE, READY_FRAME, CURRENT_FRAME_ID, STEPPER_POS };
		enum Arduino_Command_Type { SET_COLOR, GOTO_FRAME_ID, FRAME_STEP, GOTO_STEPPER_POS, GET_FRAME_ID, GET_STEPPER_POS, SET_FRAME_OFFSET, RESET_FRAME_ID };
		SerialConn(int baudRate, const char* portId);
		~SerialConn();
		char* readMessage(const char startDelim, const char endDelim);
		void parseMessage(const char* message);
		void handleArduinoMessage(Arduino_Message_Type messageType, int value);
		void sendCommand(const char* command);
	private:
		io_service io;
		serial_port serial;

		char getCharFromConn();

};

