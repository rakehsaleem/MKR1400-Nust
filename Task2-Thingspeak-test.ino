/*
  Web client
 This sketch connects to the Thingspeak website using SSL through a MKR GSM 1400 board. Specifically, this example
 performs a write and read operation to the thingspeak channel specified in the APN data. You can Change the fields
 it writes to on Thingspeak as you wish. This sketch prints output to the Serial monitor.
 Circuit:
 * MKR GSM 1400 board
 * Antenna
 * SIM card with a data plan
 created 05 Feb 2018
 by SMGS
 modified by Rakeh- 23 Apr 2018
*/

// libraries
#include <MKRGSM.h>
#include <stdio.h>
#include <string.h>
//#include "arduino_secrets.h"

// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// PIN Number
const char PINNUMBER[]     = "";
// APN data
const char GPRS_APN[]      = "zonginternet";
const char GPRS_LOGIN[]    = "";
const char GPRS_PASSWORD[] = "";

// Thingspeak Secret Settings
char writeapikey[]      = "POWWNFLAIARHZL10";
char readapikey[]       = "43F8VBLWVJP4Y2FN";
char channelid[]        = "455094";

// initialize the library instance
GSMSSLClient client;
GPRS gprs;
GSM gsmAccess;
GSMLocation location;

// Domain name and port that we will connect too (for example: api.thingspeak.com)
char server[] = "api.thingspeak.com";
int port = 443; // port 443 is the default for HTTPS
// Remainder of the URL is crafted together in the writeThingspeak() or readThingspeak() functions which take
// some arguments such as field and result, the crafted URL is returned from the function for later use.
char writeURLField[200];
char readURLField[200];

// Setup the Fields with meaningful names on Thingspeak Channels to make it easier to read.
// The Thingspeak field name is set to equal an Integer which is the Field Number...
// I believe Thingspeak usually only uses 8 fields by Default...
int t_light_sensor = 1; // field 1 - A Light Sensor TEMT6000
int t_led_button = 2;   // field 2 - Green LED
//int field3 = 3;         // field 3 - unutilised
//int field3 = 4;         // field 4 - unutilised
//int field3 = 5;         // field 5 - unutilised
//int field3 = 6;         // field 6 - unutilised
int t_latitude = 7;     // field 7 - to be utilised later with the GSM GPS Module
int t_longitude = 8;    // field 8 - to be utilised later with the GSM GPS Module

void setup() {

  pinMode(13, OUTPUT);          // sets the digital pin 13 as output
  digitalWrite(13, LOW);        // sets the digital pin 13 off

  // initialize serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting Arduino web client.");
  // connection state
  boolean connected = false;

  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (!connected) {
    if ((gsmAccess.begin(PINNUMBER) == GSM_READY) &&
        (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)) {
          connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }
}

void loop() {

   digitalWrite(13, LOW);        // sets the digital pin 13 off
   int result = A0; //temporarily set a result manually, leter we can set the result based on a reading from a light sensor

  //Initizalize the URL by calling the Function to do so...
  String connectionMethod = "Write";
  setupWriteThingspeakURL(t_led_button, result);
  connectToThingspeak(connectionMethod);

  // Now that we've performed a write into the t_led_button field, we can do a read

  connectionMethod = "Read";
  setupReadThingspeakURL(t_led_button);
  connectToThingspeak(connectionMethod);

  // Next trick will be getting the results from the Read section and turning an LED to High or LOW based on that field...
  // Code to be entered here :)

  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if the server's disconnected, stop the client:
  if (!client.available() && !client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // I've left this in so I don't smash a server accidentally
  // or use all my pre-paid SIM card data...
  Serial.println("Do Nothing forevermore");
  // do nothing forevermore:
  for (;;)
    ;
}

void connectToThingspeak(String c_meth) {
  /*
   *  In order to make the original Test code for GSM SSL Web Client more moduler this function has been created which
   *  takes an input of "Read" or "Write" and modifies the way it runs based on those values.
   *  i.e. if it recieves "Read" then it will do a GET connection and pass the readURLField value through
   *  if it recieves "Write" then it will do a PUT connection and pass the writeURLField value through...
   */

  Serial.println("connecting...");

  if (c_meth.equals("Read"))
  {
    // if you get a connection, report back via serial:
    if (client.connect(server, port)) {
      Serial.print(c_meth);
      Serial.println(" connected");
      // Make a HTTP request:
      client.print("GET ");
      client.print(readURLField);
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(server);
      client.println("Connection: close");
      client.println();
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    }
  }
  else if (c_meth.equals("Write"))
  {
    // if you get a connection, report back via serial:
    if (client.connect(server, port))
    {
      Serial.print(c_meth);
      Serial.println(" connected");
      // Make a HTTP request:
      client.print("PUT ");
      client.print(writeURLField);
      client.println(" HTTP/1.1");
      client.print("Host: ");
      client.println(server);
      client.println("Connection: close");
      client.println();
    } else {
      // if you didn't get a connection to the server:
      Serial.println("connection failed");
    }
  }
  delay(200);
}

void setupWriteThingspeakURL(int ifield, int iresult) {
  /*
   *  This Function is Designed to craft the URL required to write to a specific Thingspeak field when the user passes in an
   *  Integer variable of the field number and an integer of the result you wish to publish to your Thingspeak Channel...
   *  The crafted URL is written out to a global variable and can then be utilised in the connection function.
   *  This crafted URL includes data from the Arduino_secrets.h file which contains items such as the Thingspeak Channel number
   *  and write API key etc
   */

  // An Complete Example URL to Update Channel Field1 via Thingspeak
  // GET https://api.thingspeak.com/update?api_key=1234567890ABC&field1=0
  // Our URL is broken down below to give us the ability to update certain portions of the URL, such as the field number etc

  char t_fieldNumber[sizeof(ifield)/sizeof(int)]; //passed into the function as an int and converted to a char array
  char t_result[sizeof(iresult)/sizeof(int)]; //passed into the function as an int and converted to a char array
  char t_write_update[17] = "/update?api_key=";
  char t_write_field[8] = "&field";
  char t_equal[2] = "=";

  itoa(ifield, t_fieldNumber, 10); //where passed in field value gets converted from an int to a char array
  itoa(iresult, t_result, 10); //where passed in rsult value gets converted from an int to a char array


  //Build the custom Write URL to send data to my Thingspeak Channel via strcpy and strcat
  strcpy (writeURLField,t_write_update);
  strcat (writeURLField,writeapikey);
  strcat (writeURLField,t_write_field);
  strcat (writeURLField,t_fieldNumber);
  strcat (writeURLField,t_equal);
  strcat (writeURLField,t_result);

} //end of writeThingspeak

void setupReadThingspeakURL(int r_ifield) {
  /*
   *  This Function is Designed to craft the URL required to read a Thingspeak URL when the user passes in an
   *  Integer variable of the field number which they wish to read. The crafted URL is written out to a global
   *  variable and can then be utilised in the connection function. This crafted URL includes data from the Arduino_secrets.h
   *  file which contains items such as the Thingspeak Channel number and Read API key etc
   */

  // An Complete Example URL to Update Channel Field1 via Thingspeak
  // GET GET https://api.thingspeak.com/channels/123456/fields/1.json?api_key=1234567890987654321&results=2
  // Our URL is broken down below to give us the ability to update certain portions of the URL, such as the field number etc

  char t_fieldNumber[sizeof(r_ifield)/sizeof(int)]; //passed into the function as an int and converted to a char array
  char t_channel[11] = "/channels/";
  char t_read_field[9] = "/fields/";
  char t_json[20] = "/last.json?api_key=";
  char t_res[] = "&results=2";

  itoa(r_ifield, t_fieldNumber, 10); //where passed in field value gets converted from an int to a char array

  //Build the custom Write URL to send data to my Thingspeak Channel via strcpy and strcat
  strcpy (readURLField,t_channel);
  strcat (readURLField,channelid);
  strcat (readURLField,t_read_field);
  strcat (readURLField,t_fieldNumber);
  strcat (readURLField,t_json);
  strcat (readURLField,readapikey);
  strcat (readURLField,t_res);


} //end of readThingspeak
