
#define PC_BAUD_RATE 9600

#define MSG_SIZE 256
#define MSG_START_DELIM '\r'
#define MSG_END_DELIM '\n'

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

void loop() {
  printToSerialWithDelimiters("PHOTO EXECUTE");
}
