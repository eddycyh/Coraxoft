#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "DHT.h"
#include <SPI.h>
#include "MFRC522.h"

const char* ssid = "EddyC";
const char* password = "~0126618295!";
IPAddress ip(192, 168, 0, 200);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255,255,255,0);
WiFiServer server(8085);

WiFiUDP Udp;
unsigned int localUdpPort = 4210;
char incomingPacket[255];
char replyPacket[] = "Hi there!, A reply Message from NodeMCU :-)";

#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

/*MFRC522 Wiring to ESP8266
RST     = GPIO5
SDA(SS) = GPIO9
MOSI    = GPIO13
MISO    = GPIO12
SCK     = GPIO14
GND     = GND
3.3V    = 3.3V
*/

#define RST_PIN   5
#define SS_PIN    4

MFRC522 mfrc522(SS_PIN, RST_PIN);         //Create MFRC522 instance

//PIR Input
#define PIR_PIN   16

//Door Sensor Input
#define DS_PIN    15

//Output LED
//#define RED_PIN   15                  //Temporary removed RED_LED
#define WHT_PIN   0

//LDR Light Density Sensor
const int AnalogLightValue = A0;
float currentLight = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Booting....");

  pinMode(PIR_PIN, INPUT);
  //pinMode(RED_PIN, OUTPUT);
  pinMode(WHT_PIN, OUTPUT);
  pinMode(DS_PIN, INPUT_PULLUP);

  SPI.begin();                            //Init SPI bus
  mfrc522.PCD_Init();                     //Init MFRC522
  
  //Connect to WiFi Network 
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  //Start server 
  server.begin();
  Serial.println("Server started");

  //Print IP address
  Serial.println("Use this URL: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  //Setup UDP Connection
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  Serial.println(F("Ready!"));
  Serial.println(F("======================================================")); 
  Serial.println(F("Scan for Card and print UID:"));

//  digitalWrite(RED_PIN, HIGH);
  digitalWrite(WHT_PIN, LOW);
}

int isReadPIR = 0;
int PIR_Val = 0;
byte CardUID[4] = {0,0,0,0};
int MonStatus = 0;
int DoorStatus = 0;

void loop() {
  
  // put your main code here, to run repeatedly:
/*  if(isReadPIR >= 5)
  {
    //Read LDR Value
    int stepLDR = analogRead(AnalogLightValue);
    float LDRTemp = (stepLDR * 3.3)/1024;
    currentLight = LDRTemp;              //10mV per step

    //Read Door Sensor Value
    DoorStatus = digitalRead(DS_PIN);
    
    //Read PIR Value
    PIR_Val = digitalRead(PIR_PIN);
    isReadPIR = 0;
    //MFRC522 --> Look for new cards
    if(mfrc522.PICC_IsNewCardPresent())
    {
      //Now select one of the cards
      if(mfrc522.PICC_ReadCardSerial())
      {
        /*Serial.print(F("Card UID:"));
        dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
        Serial.println();*/
 /*       CardUID[0] = mfrc522.uid.uidByte[0];
        CardUID[1] = mfrc522.uid.uidByte[1];
        CardUID[2] = mfrc522.uid.uidByte[2];
        CardUID[3] = mfrc522.uid.uidByte[3];
      }
    }
  }
  else
  {
    isReadPIR = isReadPIR + 1;
  }*/
  
  /*if(PIR_Val == 1)
  {
    digitalWrite(WHT_PIN, HIGH);
  }
  else
  {
    digitalWrite(WHT_PIN, LOW);
  }*/

  /*if(currentLight >= 2.5)
  {
    digitalWrite(WHT_PIN, HIGH);
  }
  else
  {
    digitalWrite(WHT_PIN, LOW);
  }*/
  
  //Process UDP packet
  int packetSize = Udp.parsePacket();
  if(packetSize)
  {
    //received incoming UDP Packets
     Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if(len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    if(incomingPacket[0] == 'T')
    {
      //Display temperature value
      float temp = dht.readTemperature();
      char tempReply[255];
      String stempReply = String(temp, 1);
      stempReply.toCharArray(tempReply, 255);
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(tempReply);
      Udp.endPacket();
    }
    else if(incomingPacket[0] == 'H')
    {
      //Display Humidity value
      float humi = dht.readHumidity();
      char humiReply[255];
      String shumiReply = String(humi, 1);
      shumiReply.toCharArray(humiReply, 255);
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(humiReply);
      Udp.endPacket();
    }
    else if(incomingPacket[0] == 'P')
    {
      char PIRReply[255];
      String sPIRReply = String(PIR_Val, DEC);
      sPIRReply.toCharArray(PIRReply, 255);
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(PIRReply);
      Udp.endPacket();
    }
/*   else if(incomingPacket[0] == 'D')
    {
      int DS_Val = digitalRead(DS_PIN);
      char DSReply[255];
      String sDSReply = String(DS_Val, DEC);
      sDSReply.toCharArray(DSReply, 255);
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(DSReply);
      Udp.endPacket();
    }*/
    else
    {
      //Send back a reply to IP address and port we got the packet from
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(replyPacket);
      Udp.endPacket();
    }
  } 
  
  //Check if a client has connected 
  WiFiClient client = server.available();
  if(!client){
    return;
  }

  //Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }

  //Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  //Return the response 
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connnection: close");
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<meta http-equiv=\"refresh\" content=\"10\">");        //Refresh every 5 seconds
  client.println("<br />"); 
  client.println("<head>");
  client.println("<style>");
  client.println("p.outset {border-style: outset};");
  client.println("</style>");
  client.println("</head>");
  client.println("<p class=outset style=font-size:36px>Edlane Home Automation</p>");
  client.println(""); //  do not forget this one

  float h = 67.8;//dht.readHumidity();
  float t = 28.0;//dht.readTemperature();
  float f = 55.0;//dht.readTemperature(true);

  if(isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("Failed to read from DHT Sensor");
    return;
  }

    // Match the request  
  if (request.indexOf("/MON=on") != -1)  {
    digitalWrite(WHT_PIN, HIGH);
    MonStatus = 1;
  }
  if (request.indexOf("/MON=off") != -1)  {
    digitalWrite(WHT_PIN, LOW);
    MonStatus = 0;
  }

//Display Temperature
  client.print("Temperature (C): ");
  client.println(t, 1);
  client.println("<br />");

//Display Humidity
  client.print("Humidity (%): ");
  client.println(h, 0);
  client.println("<br />");

//Display last scan number
  client.print("Last Scan Number: ");
  client.print(CardUID[0],HEX);
  client.print(CardUID[1],HEX);
  client.print(CardUID[2],HEX);
  client.print(CardUID[3],HEX);
  client.println("<br />");

//Display LDR value
  client.print("Light Density: ");
  client.println(currentLight, 2);
  client.println("<br />");

  client.println("<br />");
  client.println("<br />");
  client.println("<br />");

//Display PIR Status  
if(PIR_Val == 1 || DoorStatus == 1)
{
  client.print("Intruder Detected!!! ALARM ON");
  client.println("<br />");
} 
else
{
  client.print("Alarm Monitoring Status: Normal");
  client.println("<br />");
}

  client.println("<br />");
  client.println("Monitoring system is now: ");
  if(MonStatus == 1)
  {
    client.print("ON");
  }
  else
  {
    client.print("OFF");
  }
  client.println("<br><br>");
  client.println("<a href=\"/MON=on\"\"><button>Monitoring On </button></a>");
  client.println("<a href=\"/MON=off\"\"><button>Monitoring Off </button></a><br />");  
  client.println("</html>");
  delay(1);
  Serial.println("Client disconnected");
}

//Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize){
  for(byte i = 0; i < bufferSize; i++){
    Serial.print( buffer[i] < 0x10 ? " 0": " ");
    Serial.print(buffer[i], HEX);
    //CardUID[i] = buffer[i];
  }
}

