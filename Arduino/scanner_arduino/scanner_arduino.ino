
#define PC_BAUD_RATE 9600

#define MSG_SIZE 256
#define MSG_START_DELIM '\r'
#define MSG_END_DELIM '\n'

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


enum Arduino_Message_Type {
    ACK,
    READY,
    READY_RED,
    READY_GREEN,
    READY_BLUE,
    READY_FRAME,
    CURRENT_FRAME_ID,
    STEPPER_POS
};

enum Arduino_Command_Type {
    SET_COLOR_RED,
    SET_COLOR_GREEN,
    SET_COLOR_BLUE,
    GOTO_FRAME_ID,
    FRAME_STEP,
    GOTO_STEPPER_POS,
    GET_FRAME_ID,
    GET_STEPPER_POS,
    SET_FRAME_OFFSET,
    RESET_FRAME_ID
};


void setup() {
  Serial.begin(PC_BAUD_RATE);
}

void printToSerialWithDelimiters(const char* message) {
  // Buffer to hold the formatted message
  char formattedMessage[MSG_SIZE];
  for(int i = 0; i < MSG_SIZE; i++) {
    formattedMessage[i] = 0;
  }

  // Format the message with \r at the front and \n at the end
  sprintf(formattedMessage, "%c%s%c", MSG_START_DELIM, message, MSG_END_DELIM);
  int bytesSent = Serial.write(formattedMessage);
}

const char* getMessageTypeString(Arduino_Message_Type messageType) {
    switch (messageType) {
        case ACK: return "ACK";
        case READY: return "READY";
        case READY_RED: return "READY:RED";
        case READY_GREEN: return "READY:GREEN";
        case READY_BLUE: return "READY:BLUE";
        case READY_FRAME: return "READY:FRAME";
        case CURRENT_FRAME_ID: return "CURRENT_FRAME_ID:";
        case STEPPER_POS: return "STEPPER_POS:";
        default: return "UNKNOWN";
    }
}

const char* getCommandTypeString(Arduino_Command_Type commandType) {
    switch (commandType) {
        case SET_COLOR_RED: return "SET_COLOR:RED";
        case SET_COLOR_GREEN: return "SET_COLOR:GREEN";
        case SET_COLOR_BLUE: return "SET_COLOR:BLUE";
        case GOTO_FRAME_ID: return "GOTO_FRAME_ID";
        case FRAME_STEP: return "FRAME_STEP";
        case GOTO_STEPPER_POS: return "GOTO_STEPPER_POS";
        case GET_FRAME_ID: return "GET_FRAME_ID";
        case GET_STEPPER_POS: return "GET_STEPPER_POS";
        case SET_FRAME_OFFSET: return "SET_FRAME_OFFSET";
        case RESET_FRAME_ID: return "RESET_FRAME_ID";
        default: return "UNKNOWN";
    }
}

// NOTE: Int on the Zero is signed 32bit, max size of 2,147,483,647. We should be fine with this max as we will never be scanning 2 billion frames :lol:
void printMessageToSerial(Arduino_Message_Type messageType, int number = -1) {
    const char* message = getMessageTypeString(messageType);
    char formattedMessage[MSG_SIZE];

    if (number >= 0) {
        snprintf(formattedMessage, MSG_SIZE, "%s%d", message, number);
    } else {
        snprintf(formattedMessage, MSG_SIZE, "%s", message);
    }

    printToSerialWithDelimiters(formattedMessage);
}

void printCommandToSerial(Arduino_Command_Type commandType) {
    const char* command = getCommandTypeString(commandType);
    printToSerialWithDelimiters(command);
}

void loop() {
  printMessageToSerial(STEPPER_POS, 888999000);
  delay(1000); // Wait for 1 second
}
