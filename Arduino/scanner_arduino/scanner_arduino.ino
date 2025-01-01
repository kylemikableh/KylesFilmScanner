

int cheese = 7;
int cheese_arr[5];

void setup() {
  // put your setup code here, to run once:
  cheese_arr[1] = 12;
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  cheese++;
  Serial.write(45); // send a byte with the value 45

  int bytesSent = Serial.write("hello");  //send the string "hello" and return the length of the string.
}
