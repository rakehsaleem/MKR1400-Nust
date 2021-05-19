// Created by M. Rakeh Saleem
// Dates: 12-Apr-2018

#include <MKRGSM.h>
#include <stdio.h>
#include <string.h>

const char PINNUMBER[]     ="";
const char GPRS_APN[]      ="zonginternet";
const char GPRS_LOGIN[]    ="";
const char GPRS_PASSWORD[] ="";
char buf[20];
String path1,path2,path3,path4,path5,path6,path7,path8;
String x1,abc;
int value;
int i;

// initialize the library instance
GSMSSLClient client;
GPRS gprs;
GSM gsmAccess;

// URL, path and port (for example: arduino.cc)
char server[] = "api.thingspeak.com";
int port = 443; // port 443 is the default for HTTPS
int c=0;
int z=0;
void setup()
{
  Serial.begin(9600);         // initialize serial communications and wait for port to open:
  while (!Serial)
  {
    ;                         // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Enter number of signals = ");
  while (Serial.available())
  {
    Serial.read();
  }
  while (!Serial.available());
  c = Serial.read();
  int b = c-48;
  Serial.println(b);
  for(int k=0;k<b;k++)
  {
    char t_result[sizeof(value)/sizeof(int)];

    for(int i=0;i<432;i++)
    {
      value = analogRead(A0);
      itoa(value, t_result,10);
      x1=x1+t_result+" ";
    }
    Serial.println(x1);
    path1= "/update?api_key=POWWNFLAIARHZL10&field1=" + x1.substring(0,215);
    path2= "/update?api_key=POWWNFLAIARHZL10&field2=" + x1.substring(216,431);
    path3= "/update?api_key=POWWNFLAIARHZL10&field3=" + x1.substring(432,647);
    path4= "/update?api_key=POWWNFLAIARHZL10&field4=" + x1.substring(648,863);
    path5= "/update?api_key=POWWNFLAIARHZL10&field5=" + x1.substring(864,1079);
    path6= "/update?api_key=POWWNFLAIARHZL10&field6=" + x1.substring(1080,1295);
    path7= "/update?api_key=POWWNFLAIARHZL10&field7=" + x1.substring(1296,1511);
    path8= "/update?api_key=POWWNFLAIARHZL10&field8=" + x1.substring(1512,1735);

    Serial.println(path1);
    Serial.println(path2);
    Serial.println(path3);
    Serial.println(path4);
    Serial.println(path5);
    Serial.println(path6);
    Serial.println(path7);
    Serial.println(path8);
    Web();
    x1= "";
  }
}

void Web()
{
    Serial.println("Starting Arduino web client.");
    boolean connected = false;  // connection state

    // After starting the modem with GSM.begin()
    // attach the shield to the GPRS network with the APN, login and password
    while (!connected)
    {
      if ((gsmAccess.begin(PINNUMBER) == GSM_READY) && (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY))
    {
      connected = true;
    }
    else
    {
      Serial.println("Not connected");
      delay(1000);
    }
  }
  Serial.println("connecting...");
  if (client.connect(server, port))      // if you get a connection, report back via serial:
  {
    Serial.println("connected");
    client.print("GET ");                // Make a HTTP request:
    client.print(path1);
    client.print(path2);
    client.print(path3);
    client.print(path4);
    client.print(path5);
    client.print(path6);
    client.print(path7);
    client.print(path8);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("Connection: close");
    client.println();
    delay(1000);
  }
  else
  {
    Serial.println("connection failed");  // if you didn't get a connection to the server:
  }
}

void loop()
{
  if (client.available())                 // if there are incoming bytes available from the server, read them and print them:
  {
    char c = client.read();
    Serial.print(c);
  }
  if (!client.available() && !client.connected())        // if the server's disconnected, stop the client:
  {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    for (;;)                              // do nothing forevermore:
    ;
  }
}
