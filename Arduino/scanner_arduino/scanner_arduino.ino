
#define PC_BAUD_RATE 115200 //9600

#define MSG_SIZE 256
#define MSG_START_DELIM '('
#define MSG_END_DELIM ')'

/*
* Message Types FROM Arduino
* ACK- Acknowledge command from PC
* READY- Arduino is ready to receive a command
* READY_RED- Arduino is ready to scan RED color
* READY_GREEN- Arduino is ready to scan GREEN color
* READY_BLUE- Arduino is ready to scan BLUE color
* READY_FRAME- Frame ready to be captured (all colors)
* CURRENT_FRAME_ID:0- Frame ID
* STEPPER_POS:0- Stepper position
*/

/*
* Messag Types TO Arduino
* SET_COLOR_RED
* SET_COLOR_GREEN
* SET_COLOR_BLUE
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
    CURRENT_STEPPER_POS,
    UNKNOWN
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

char messageReceivedBuffer[MSG_SIZE]; // Initialize the buffer to hold the raw message from PC
int frameId = 0;
int stepperPos = 0;

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

/*
* Get a message string to return that can be passed to PC from Arduino_Message_Type
*/
const char* getMessageTypeString(Arduino_Message_Type messageType) {
    switch (messageType) {
        case ACK: return "ACK";
        case READY: return "READY";
        case READY_RED: return "READY_RED";
        case READY_GREEN: return "READY_GREEN";
        case READY_BLUE: return "READY_BLUE";
        case READY_FRAME: return "READY_FRAME";
        case CURRENT_FRAME_ID: return "CURRENT_FRAME_ID:";
        case CURRENT_STEPPER_POS: return "CURRENT_STEPPER_POS:";
        default: return "UNKNOWN_CMD";
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

void handleCommand(Arduino_Command_Type command, int value) {
    switch (command) {
        case SET_COLOR_RED:
            // Set the color to RED
            printMessageToSerial(READY_RED);
            break;
        case SET_COLOR_GREEN:
            // Set the color to GREEN
            printMessageToSerial(READY_GREEN);
            break;
        case SET_COLOR_BLUE:
            // Set the color to BLUE
            printMessageToSerial(READY_BLUE);
            break;
        case GOTO_FRAME_ID:
            // Move to the given frame ID
            printMessageToSerial(CURRENT_FRAME_ID, frameId);
            break;
        case FRAME_STEP:
            // Move the given number of frames
            printMessageToSerial(CURRENT_FRAME_ID, frameId);
            break;
        case GOTO_STEPPER_POS:
            // Move to the given stepper position
            printMessageToSerial(CURRENT_STEPPER_POS, stepperPos);
            break;
        case GET_FRAME_ID:
            // Get the current frame ID
            printMessageToSerial(CURRENT_FRAME_ID, frameId);
            break;
        case GET_STEPPER_POS:
            // Get the current stepper position
            printMessageToSerial(CURRENT_STEPPER_POS, stepperPos);
            break;
        case SET_FRAME_OFFSET:
            // Set the frame offset
            printMessageToSerial(ACK);
            break;
        case RESET_FRAME_ID:
            // Reset the frame ID
            printMessageToSerial(CURRENT_FRAME_ID, frameId);
            break;
        default:
            // printf("Unknown command received from PC\n");
            printMessageToSerial(UNKNOWN);
            break;
    }
}

bool readMessageFromSerial(const char startDelim, const char endDelim) {
    int i = 0;
    bool messageStarted = false;
    char lastChar = 'z'; // Debuger helper

    // Initialize the buffer
    memset(messageReceivedBuffer, 0, MSG_SIZE);

    while (true) {
        while (Serial.available() == 0);
        char c = Serial.read();

        // Serial.print(c); // Enable to debug

        if (c == startDelim) {
            messageStarted = true;
            continue;
        }
        if (c == endDelim) {
            break;
        }
        if (messageStarted && i < MSG_SIZE - 1) { // Ensure we don't overflow the buffer
            messageReceivedBuffer[i] = c;
            lastChar = c;
            i++;
        } else if (i > MSG_SIZE -1){
            break;
        }

    }
    // Null-terminate the message
    messageReceivedBuffer[i] = '\0';
    lastChar = lastChar;
    printf("%s", lastChar);
    if(messageStarted) {
      return true;
    }
    return false;
}

void waitForCommandFromPC() {
    delay(10);
    bool gotMessage = readMessageFromSerial(MSG_START_DELIM, MSG_END_DELIM);
    if(gotMessage) {
      handleCommandFromString();
    } else {
      Serial.print(MSG_START_DELIM);
      Serial.print("READY_FOR_COMMAND");
      Serial.print(MSG_END_DELIM);
    }
}

/*
* Decode string to Arduino_Command_Type which we received from the PC
*/
void handleCommandFromString() {
    // Determine the message type from the message string
    Arduino_Command_Type commandType = static_cast<Arduino_Command_Type>(-1);
    int value = -1;

    if (strcmp(messageReceivedBuffer, "SET_COLOR_RED") == 0) {
        commandType = SET_COLOR_RED;
    } else if (strcmp(messageReceivedBuffer, "SET_COLOR_GREEN") == 0) {
        commandType = SET_COLOR_GREEN;
    } else if (strcmp(messageReceivedBuffer, "SET_COLOR_BLUE") == 0) {
        commandType = SET_COLOR_BLUE;
    } else if (strcmp(messageReceivedBuffer, "FRAME_STEP") == 0) {
        commandType = FRAME_STEP;
    } else if (strcmp(messageReceivedBuffer, "GET_FRAME_ID") == 0) {
        commandType = GET_FRAME_ID;
    } else if (strcmp(messageReceivedBuffer, "GET_STEPPER_POS") == 0) {
        commandType = GET_STEPPER_POS;
    } else if (strcmp(messageReceivedBuffer, "RESET_FRAME_ID") == 0) {
        commandType = RESET_FRAME_ID;
    } else if (strncmp(messageReceivedBuffer, "GOTO_FRAME_ID:", 14 ) == 0) {
        commandType = GOTO_FRAME_ID;
        char* endPtr;
        value = strtol(messageReceivedBuffer + 14, &endPtr, 10); // Extract the number after "FRAME_ID:", 10 here is base10 number system
        if (*endPtr != '\0') {
            // printf("Invalid number format in message: %s", command);
        }
    } else if (strncmp(messageReceivedBuffer, "GOTO_STEPPER_POS:", 17) == 0) {
        commandType = GOTO_STEPPER_POS;
        char* endPtr;
        value = strtol(messageReceivedBuffer + 17, &endPtr, 10); // Extract the number after "STEPPER_POS:", 10 here is base10 number system
        if (*endPtr != '\0') {
            // printf("Invalid number format in message: %s", command);
        }
    } else if (strncmp(messageReceivedBuffer, "SET_FRAME_OFFSET:", 17) == 0) {
        commandType = SET_FRAME_OFFSET;
        char* endPtr;
        value = strtol(messageReceivedBuffer + 17, &endPtr, 10); // Extract the number after "SET_FRAME_OFFSET:", 10 here is base10 number system
        if (*endPtr != '\0') {
            // printf("Invalid number format in message: %s", command);
        }
    }
    else {
        // printf("Unknown command received from PC: %s", command);
        Serial.print(MSG_START_DELIM);
        Serial.print("UNKNOWN_CMD");
        // Serial.print(messageReceivedBuffer);
        Serial.print(MSG_END_DELIM);
    }

    // Return if the commandType was never set
    if (commandType == -1) {
        return;
    }

    handleCommand(commandType, value);

}

void loop() {
  // printMessageToSerial(STEPPER_POS, 888999000);
  delay(10); // Wait for 1 second
  // printf("Waiting for command from PC...");
  waitForCommandFromPC();
}
