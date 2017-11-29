#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "DHT.h"

const char* ssid = "EddyC";
const char* password = "~0126618295!";
IPAddress ip(192, 168, 0, 200);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
WiFiServer server(8085);
WiFiUDP Udp;
unsigned int localUdpPort = 4210;
char incomingPacket[255];
char replyPacket[] = "Hi there!, A reply message from NodeMCU :-)";

//DHT Input
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

//Door Sensor
#define DS1_PIN                   14
#define DS2_PIN                   12
#define DS3_PIN                   13

//Output to relay
#define SSR1_PIN                  0       //D3
#define SSR2_PIN                  15      //D8

//Output to running light
#define RED_PIN                   5       //D2

//PIR Input
#define PIR_PIN                   16      //D0

//LDR Input
const int AnalogLightValue = A0;
float currentLight = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Booting....");
  //Define Input
  pinMode(PIR_PIN, INPUT);
  pinMode(DS1_PIN, INPUT);
  pinMode(DS2_PIN, INPUT);
  pinMode(DS3_PIN, INPUT);
  //Connect to WiFi Network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  //Start webserver
  server.begin();
  Serial.println("Server started");
  //Print IP Address
  Serial.println("Use this URL: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  //Start UDP Connection
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  digitalWrite(RED_PIN, LOW);
    //Define output
  pinMode(SSR1_PIN, OUTPUT);
  digitalWrite(SSR1_PIN, LOW);
  pinMode(SSR2_PIN, OUTPUT);
  digitalWrite(SSR2_PIN, LOW);
  pinMode(RED_PIN, OUTPUT);
}

//Declare global variables
int isReadPIR = 0;
int isReadLDR = 0;
int PIR_Val = 0;
int MonStatus = 0;
int DoorStatus1 = 0;
int DoorStatus2 = 0;
int DoorStatus3 = 0;
float h,t,f;
void loop() {
  // put your main code here, to run repeatedly:
  if (isReadLDR >= 500)
  {
    //Read LDR Value
    int stepLDR = analogRead(AnalogLightValue);
    float LDRTemp = (stepLDR * 3.3) / 1024;
    currentLight = LDRTemp;
    isReadLDR = 0;
  }
  else
  {
    isReadLDR = isReadLDR + 1;
  }

  if (isReadPIR >= 50)
  {
    //Read PIR Value //always read
    PIR_Val = digitalRead(PIR_PIN);
    if (PIR_Val == 1)
    {
      digitalWrite(RED_PIN, HIGH);
    }
    else
    {
      digitalWrite(RED_PIN, LOW);
    }
    
    //Read DoorSensor Value
    DoorStatus1 = digitalRead(DS1_PIN);
    DoorStatus2 = digitalRead(DS2_PIN);
    DoorStatus3 = digitalRead(DS3_PIN);

    h = dht.readHumidity();
    t = dht.readTemperature();
    f = dht.readTemperature(true);
    isReadPIR = 0;
  }
  else
  {
    isReadPIR = isReadPIR + 1;
  }

  //Process UDP packet
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    //received incoming UDP Packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    if (incomingPacket[0] == 'T')
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
    else if (incomingPacket[0] == 'H')
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
    else if (incomingPacket[0] == 'P')
    {
      char PIRReply[255];
      String sPIRReply = String(PIR_Val, DEC);
      sPIRReply.toCharArray(PIRReply, 255);
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(PIRReply);
      Udp.endPacket();
    }
    else if (incomingPacket[0] == 'D' && incomingPacket[1] == '1')
    {
      char DSReply[255];
      String sDSReply = String(DoorStatus1, DEC);
      sDSReply.toCharArray(DSReply, 255);
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(DSReply);
      Udp.endPacket();
    }
    else if (incomingPacket[0] == 'D' && incomingPacket[1] == '2')
    {
      char DSReply[255];
      String sDSReply = String(DoorStatus2, DEC);
      sDSReply.toCharArray(DSReply, 255);
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(DSReply);
      Udp.endPacket();
    }
    else if (incomingPacket[0] == 'D' && incomingPacket[1] == '3')
    {
      char DSReply[255];
      String sDSReply = String(DoorStatus3, DEC);
      sDSReply.toCharArray(DSReply, 255);
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(DSReply);
      Udp.endPacket();
    }
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
  if (!client) {
    return;
  }
  //Wait until the client sends some data
  Serial.println("new client");
  while (!client.available()) {
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
  //client.println("<meta http-equiv=\"refresh\" content=\"10\">");        //Refresh every 5 seconds
  client.println("<br />");
  client.println("<head>");
  client.println("<style>");
  client.println("p.outset {border-style: outset};");
  client.println("</style>");
  client.println("</head>");
  client.println("<p class=outset style=font-size:36px>Edlane Home Automation</p>");
  client.println(""); //  do not forget this one

  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("Failed to read from DHT Sensor");
    return;
  }

  // Match the request
  if (request.indexOf("/MON=on") != -1)  {
    digitalWrite(SSR2_PIN, HIGH);
    MonStatus = 1;
  }
  if (request.indexOf("/MON=off") != -1)  {
    digitalWrite(SSR2_PIN, LOW);
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


  //Display LDR value
  client.print("Light Density: ");
  client.println(currentLight, 2);
  client.println("<br />");

  //Display DS1 value
  client.print("Door Sensor 1: ");
  if(DoorStatus1 == 1)
    client.println("Door is opened!");
  else
    client.println("Door is closed!");
  client.println("<br />");

  //Display DS2 value
  client.print("Door Sensor 2: ");
  if(DoorStatus2 == 1)
    client.println("Door is opened!");
  else
    client.println("Door is closed!");
  client.println("<br />");

  //Display DS3 value
  client.print("Door Sensor 3: ");
  if(DoorStatus3 == 1)
    client.println("Door is opened!");
  else
    client.println("Door is closed!");
  client.println("<br />");
  client.println("<br />");

  //Display PIR Status
  if (PIR_Val == 1)// || DoorStatus1 == 1 || DoorStatus2 == 1 || DoorStatus3 == 1)
  {
    client.print("Intruder Detected!!! ALARM ON");
    client.println("<br />");
  }
  else
  {
    client.print("Alarm Monitoring Status: Normal");
    client.println("<br />");
  }
  client.println("Monitoring system is now: ");
  if (MonStatus == 1)
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
