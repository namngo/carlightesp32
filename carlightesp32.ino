#include "Arduino.h"

#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelBus.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "FS.h"
#include <string>
#include <SPIFFS.h>
#include "rgbwlight.h"
#include "espnetwork.h"

const byte DNS_PORT = 53;

const uint16_t PixelCount = 4;
const uint16_t LedOutGPIO = 22; // D22
const uint16_t TemperatureGPIO = 2;

const char WiFiAPPID[] = "nango_car_led";
const char WiFiAPPSK[] = "hondaaccord";
const IPAddress ip(192, 168, 4, 1);

const char indexHtmlPath[] = "/index.html";

RbgwLight light(LedOutGPIO, 2);

const IPAddress AP_IP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer server(80);

String responseHTML = ""
"<!DOCTYPE html><html><head><title>CaptivePortal</title></head><body>"
"<p>No index.html found.</p></body></html>";

OneWire oneWire(TemperatureGPIO);
DallasTemperature sensors(&oneWire);

// Number of temperature devices found
int numberOfTempSensor;

// We'll use this variable to store a found device address
DeviceAddress tempDeviceAddress;

String buildJsonResponse(const RgbwColor& c)
{
  return "{\"red\":\"" + String(c.R) + "\",\"blue\":\"" + String(c.B) + "\",\"green\":\"" + String(c.G) + "\",\"white\":\"" + String(c.W) + "\"}";
}

void handleSerialRequest()
{
  while(Serial.available()) {
    auto str = Serial.readString();
    
    if (str.length() == 8) {
      long r = strtol(&str.substring(0, 2)[0], NULL, 16);
      long g = strtol(&str.substring(2, 4)[0], NULL, 16);
      long b = strtol(&str.substring(4, 6)[0], NULL, 16);
      long seat = strtol(&str.substring(6, 8)[0], NULL, 16);

      auto color = light.Update(seat, r, b, g);
      Serial.println(buildJsonResponse(color));
    }
  }
}

void handleColorChangeRequest()
{
  long r = server.arg("red").toInt();
  long b = server.arg("blue").toInt();
  long g = server.arg("green").toInt();
  long seat = server.arg("seat").toInt();
  
  auto color = light.Update(seat, r, b, g);

  server.send(200, "application/json", buildJsonResponse(color));
}

// the setup function runs once when you press reset or power the board
void setup() {
  light.Begin();
  SPIFFS.begin();
  Serial.begin(115200);

  // WiFi.mode(WIFI_AP);
  // WiFi.softAPConfig(apIP, apGateWay, IPAddress(255, 255, 255, 0));
  // WiFi.softAP(WiFiAPPID, WiFiAPPSK);

  // dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  // if (!dnsServer.start(DNS_PORT, "*", WiFi.softAPIP())) {
  //   Serial.println("Cannot start dns server");
  // };

// setup the index.html
  if (SPIFFS.begin() && SPIFFS.exists(indexHtmlPath))
  {
    File f = SPIFFS.open(indexHtmlPath, "r");
    if (f)
    {
      responseHTML = f.readString();
    }
  }

  //server.serveStatic("/static", SPIFFS, "/static", "max-age=86400");
  server.serveStatic("/static/jquery-3.2.1.min.js", SPIFFS, "/static/jquery-3.2.1.min.js", "max-age=86400");
  server.serveStatic("/static/colorpicker.min.js", SPIFFS, "/static/colorpicker.min.js", "max-age=86400");
  server.serveStatic("/static/colorpicker.min.css", SPIFFS, "/static/colorpicker.min.css", "max-age=86400");
  server.serveStatic("/static/bootstrap.min.js", SPIFFS, "/static/bootstrap.min.js", "max-age=86400");
  server.serveStatic("/static/bootstrap.min.css", SPIFFS, "/static/bootstrap.min.css", "max-age=86400");

  server.on("/led", handleColorChangeRequest);

  server.on("/index.html", [] () {
      server.send(200, "text/html", responseHTML);
  });

  server.begin();
  sensors.begin();
  numberOfTempSensor = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(numberOfTempSensor, DEC);
  Serial.println("Devices");

  Serial.println();
}

// the loop function runs over and over again forever
void loop() {

  sensors.requestTemperatures();
  
  for(auto i = 0; i < numberOfTempSensor; i ++) {
    if (sensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("Temperature for device: ");
      Serial.println(i,DEC);
      // Print the data
      float tempC = sensors.getTempC(tempDeviceAddress);
      Serial.print("Temp C: ");
      Serial.print(tempC);
      Serial.print(" Temp F: ");
      Serial.println(DallasTemperature::toFahrenheit(tempC));
    }
  }
  dnsServer.processNextRequest();
  server.handleClient();
  handleSerialRequest();
}
