/*Outdoor weather box and porsch light control*/
/*IP Address: 192.168.0.200********************/
/*HTTP PORT: 8085******************************/
/*UDP PORT: 4210*******************************/
/*YHChin - 20170917****************************/
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
//#include <TelegramBot.h>
//#include <WiFi101.h>
#include <ESP8266TelegramBOT.h>
#include "WiFiClientSecure.h"
#include "DHT.h"

const char* ssid = "EddyC";
const char* password = "~0126618295!";
IPAddress ip(192,168,0,200);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
WiFiServer server(8085);
WiFiUDP Udp;

const char* BotToken = "473107925:AAHKy3XLz-ZyLjWW1u3YCNR2xEOaLhMCRws";
WiFiClientSecure client;
TelegramBOT bot(BotToken,client);

unsigned int localUdpPort = 4210;
char incomingPacket[255];
char replyPacket[] = "Edlane Smart Home!, Outdoor Unit";

//DHT Input
#define DHTPIN D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
//Door Sensor
#define DS1_PIN                   14
#define DS2_PIN                   12
#define DS3_PIN                   13
//Porsch Light 
#define PL1_PIN                    15      //D8
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
  pinMode(RED_PIN, OUTPUT);
  digitalWrite(RED_PIN, LOW);
  pinMode(PL1_PIN, OUTPUT);
  digitalWrite(PL1_PIN, LOW);
  bot.begin();
}

//Declare global variables
int isReadPIR = 0;
int isReadLDR = 0;
int PIR_Val = 0;
int MonStatus = 0;
int DoorStatus1 = 0;
int DoorStatus2 = 0;
int DoorStatus3 = 0;
int SSR_Status = 0;
float h,t,f;

void loop() {
  // put your main code here, to run repeatedly:
  message m = bot.getUpdates();
  if(m.text.equals("On"))
  {
    MonStatus = 1;
  }
  else if(m.text.equals("Off"))
  {
    MonStatus = 0;
  }
  
  if (isReadLDR >= 1000)
  {
    //Read LDR Value
    int stepLDR = analogRead(AnalogLightValue);
    float LDRTemp = (stepLDR * 3.3) / 1024;
    currentLight = LDRTemp;
    isReadLDR = 0;

    if(currentLight >= 2.75 && SSR_Status == 0)
    {
      digitalWrite(PL1_PIN, HIGH);
      SSR_Status = 1;
    }
    else if (currentLight <= 1.70 && SSR_Status == 1)
    {
      digitalWrite(PL1_PIN, LOW);
      SSR_Status = 0;
    }
  }
  else
  {
    isReadLDR = isReadLDR + 1;
  }

  if(isReadPIR >= 50)
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
  ProcessUDPPacket();
  
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
  client.println("<p class=outset style=font-size:36px>Edlane Home Automation (Weather Box)</p>");
  client.println(""); //  do not forget this one

  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("Failed to read from DHT Sensor");
    return;
  }

  // Match the request
  if (request.indexOf("/MON=on") != -1)  {
    digitalWrite(PL1_PIN, HIGH);
    //MonStatus = 1;
  }
  if (request.indexOf("/MON=off") != -1)  {
    digitalWrite(PL1_PIN, LOW);
    //MonStatus = 0;
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
  client.println("<br />");

  //Display PIR Status
  if (PIR_Val == 1)
  {
    
    client.print("Intruder Detected!!! ALARM ON");
    client.println("<br />");
  }
  else
  {
    client.print("Alarm Monitoring Status: Normal");
    client.println("<br />");
  }
  client.println("Porsch Light is now: ");
  if (MonStatus == 1)
  {
    //client.print("ON");
      //Display PIR Status
    if (PIR_Val == 1)
    {
      bot.sendMessage(m.chat_id, "Intruder Detected!!!");
      client.print("Intruder Detected!!! ALARM ON");
      client.println("<br />");
    }
    else
    {
      client.print("Alarm Monitoring Status: Normal");
      client.println("<br />");
    }
  }
  else
  {
    //client.print("OFF");
    client.print("Alarm Monitoring Status: Normal");
    client.println("<br />");
  }
  client.println("<br><br>");
  client.println("<a href=\"/MON=on\"\"><button>Porsch Light On </button></a>");
  client.println("<a href=\"/MON=off\"\"><button>Porsch Light Off </button></a><br />");
  client.println("</html>");
  delay(1);
  Serial.println("Client disconnected");
}

void ProcessUDPPacket(void)
{
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
}
