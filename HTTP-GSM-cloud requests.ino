//////////////////////////
// Header Files Section //

#include <SoftwareSerial.h>
#include <String.h>

////////////////////////////////////////
// Initialization of Global variables //

SoftwareSerial mySerial(7, 8);
char buf[220];
String buf1;
int value;
int i;

////////////////////////////
// Program Setup Function //

void setup()
{
  mySerial.begin(9600);                                // the GPRS baud rate
  Serial.begin(9600);                                  // the GPRS baud rate
  delay(500);
}

///////////////////////////
// Program Loop Function //

void loop()
{
  for (i = 1; i < 55; i++)                             // loop to declare the number of samples it holds in the array
  {
    delay(100);
    value = analogRead(A0);                            // reading analog value from pin A0
    sprintf(buf, "%s %d", buf, value);                 // parsing-deparsing command to convert array into string
  }
  mySerial.println(buf);
  delay(100);
  buf1 = buf;                                          // storing char value into String variable
  Send2Pachube();                                      // Function Call
  buf[0] = '\0';                                       // Null termination to clear the char variable
  if (mySerial.available())
  Serial.write(mySerial.read());
}

/////////////////////////////////////
// Functions Call for Send2Pachube //

void Send2Pachube()
{
  mySerial.println("AT");                              // Attention command for GSM module
  delay(1000);

  mySerial.println("AT+CPIN?");                        // Query for asking PIN required or not
  delay(1000);

  mySerial.println("AT+CREG?");                        // Query the status of Network registration
  delay(1000);

  mySerial.println("AT+CGATT?");                       // Query whether the GPRS is attached or detached
  delay(1000);

  mySerial.println("AT+CIPSHUT");                      // Closes GPRS PDP context to IP INITIAL
  delay(1000);

  mySerial.println("AT+CIPSTATUS");                    // Query current connection status
  delay(2000);

  mySerial.println("AT+CIPMUX=0");                     // Startup single IP connection
  delay(2000);
  ShowSerialData();

  mySerial.println("AT+CSTT=\"zonginternet\"");        // start task and setting the APN
  delay(1000);
  ShowSerialData();

  mySerial.println("AT+CIICR");                        // bring up wireless connection
  delay(2000);
  ShowSerialData();

  mySerial.println("AT+CIFSR");                        // get local IP adress
  delay(2000);
  ShowSerialData();

  mySerial.println("AT+CIPSPRT=0");                    // send prompt when module sends data
  delay(2000);
  ShowSerialData();

  mySerial.println("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"");                          // start up the connection with thingspeak channel
  delay(2000);
  ShowSerialData();

  mySerial.println("AT+CIPSEND");                      //begin send data to remote server
  delay(4000);
  ShowSerialData();

  String str = "GET http://api.thingspeak.com/update?api_key=POWWNFLAIARHZL10&field1=" + buf1;
  mySerial.println(str);                               //begin send data to remote server
  delay(4000);
  ShowSerialData();

  mySerial.println("\r\n\r\n");
  mySerial.println((char)26);                          //sending
  delay(2000);                                         //waitting for reply, important! the time is base on the condition of internet
  ShowSerialData();

  mySerial.println("AT+CIPSHUT");                      //close the connection
  delay(1000);
  ShowSerialData();
}

///////////////////////////////////////
// Functions call for ShowSerialData //

void ShowSerialData()
{
  while (mySerial.available() != 0)
  Serial.write(mySerial.read());
}
