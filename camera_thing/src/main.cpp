#include <Arduino.h>
#include <string.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_camera.h>

#include "camera_app.hpp"

// WiFi network name and password:
const char *networkName = "wifi";
const char *networkPswd = "pass";

// Set your Static IP address
IPAddress local_IP(192, 168, 153, 21); // Camera 1
// IPAddress local_IP(192, 168, 153, 22); // camera 2
// IPAddress local_IP(192, 168, 153, 23); // camera 3

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

// Camera Objects
#define CHUNK_LENGTH 1460
Camera_app *cam;
camera_fb_t *photo;

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

void sendPacketData(const char* buf, uint16_t len, uint16_t chunkLength) {
  uint8_t buffer[chunkLength];
  size_t blen = sizeof(buffer);
  size_t rest = len % blen;

  for (uint8_t i = 0; i < len / blen; ++i) {
    memcpy(buffer, buf + (i * blen), blen);
    Udp.beginPacket(Udp.remoteIP(), 13001);
    Udp.write(buffer, chunkLength);
    Udp.endPacket();
  }

  if (rest) {
    memcpy(buffer, buf + (len - rest), rest);
    Udp.beginPacket(Udp.remoteIP(), 13001);
    Udp.write(buffer, rest);
    Udp.endPacket();
  }
}

void setup()
{
  // Initilize hardware serial:
  Serial.begin(115200);
  // Connect to the WiFi network
  Serial.println("Init Wifi...");
  connectToWiFi(networkName, networkPswd);
  // Initilize Camera Hardware
  cam = new Camera_app();
  cam->init();
}

void loop()
{
  // Keep Taking Picture. so that Camera Stays updated
  photo = cam->capture();
  esp_camera_fb_return(photo);
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
      if ((String)packetBuffer == "Capture")
      {
        Serial.println("Taking Picture");
        photo = cam->capture();
        if (photo == NULL)
          return;
        Serial.println("Sending Data to UDP port 13001");
        sendPacketData((const char*)photo->buf, photo->len, CHUNK_LENGTH);
        Serial.println("Reusing buffer...");
        esp_camera_fb_return(photo);
        Serial.println("Sending udp....");
        Udp.beginPacket(Udp.remoteIP(), 13000);
        Udp.print("Done");
        Udp.endPacket(); // Close communication
        Serial.println("Sent: Done");
        // Wait for 1 second
        delay(1000);
      }
      if ((String)packetBuffer == "Shutdown")
      {
        Serial.println("Going to Deep Sleep...");
        delete cam;
        esp_deep_sleep_start();
      }
    }
  }
}
