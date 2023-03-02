#include <Arduino.h>
#include <string.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <AccelStepper.h>

// WiFi network name and password:
const char *networkName = "wifi";
const char *networkPswd = "pass";

// Set your Static IP address
IPAddress local_IP(192, 168, 153, 20);
// Set your Gateway IP address
IPAddress gateway(192, 168, 153, 177);

IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);   // optional
IPAddress secondaryDNS(8, 8, 4, 4); // optional

// IP address to send UDP data to:
//  either use the ip address of the server or
//  a network broadcast address
const char *udpAddress = "192.168.53.13";
const int udpPort = 13000;

// Are we currently connected?
boolean connected = false;

// The udp library class
WiFiUDP Udp;

#define UDP_BUF_SIZE 1024
// buffers for receiving and sending data
char packetBuffer[UDP_BUF_SIZE]; // buffer to hold incoming packet,

// Stepper library class
AccelStepper stepper(AccelStepper::FULL4WIRE, 26, 25, 33, 32, true);

// wifi event handler
void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    // When connected set
    Serial.print("WiFi connected! IP address: ");
    Serial.println(WiFi.localIP());
    // initializes the UDP state
    // This initializes the transfer buffer
    Serial.println("UDP Init...");
    Udp.begin(WiFi.localIP(), udpPort);
    connected = true;
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    // Serial.println("WiFi lost connection");
    connected = false;
    break;
  default:
    break;
  }
}

void connectToWiFi(const char *ssid, const char *pwd)
{
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);

  // Initiate connection
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
    Serial.println("STA Failed to configure");

  // register event handler
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

void setup()
{
  // Initilize hardware serial:
  Serial.begin(115200);
  Serial.println("Init Steppers...");
  stepper.setMaxSpeed(60);
  stepper.setAcceleration(50);
  // Connect to the WiFi network
  Serial.println("Init Wifi...");
  connectToWiFi(networkName, networkPswd);
}

void loop()
{
  // only send data when connected
  if (connected)
  {
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
      Serial.print("Received packet of size ");
      Serial.println(packetSize);

      Udp.read(packetBuffer, UDP_BUF_SIZE);
      Serial.println("Contents:");
      Serial.println(packetBuffer);
      if ((String)packetBuffer == "Spin")
      {
        Serial.println("Moving The Motor");
        stepper.enableOutputs();
        stepper.moveTo(10);
        while (stepper.currentPosition() != 10)
          stepper.run();
        stepper.stop();
        if (stepper.currentPosition() == 10)
        {
          Serial.println("Reseting the position");
          stepper.setCurrentPosition(0);
          // Disable Motor Driver
          // As the LEDs are connect to the Driver's 5V
          // we can get max amps to led when not using
          // the motor
          stepper.disableOutputs();
        }
        Serial.println("Sending udp....");
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.print("Done");
        Udp.endPacket(); // Close communication
        Serial.println("Sent: Done");
      }
      if ((String)packetBuffer == "Shutdown")
      {
        Serial.println("Going to Deep Sleep...");
        esp_deep_sleep_start();
      }
    }
  }
}
