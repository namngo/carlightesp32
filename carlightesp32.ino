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

const uint16_t PixelCount = 4;
const uint16_t LedOutGPIO = 22; // D22
const uint16_t TemperatureGPIO = 2;

const char ap_name[] = "nango_car_led";
const char ap_password[] = "hondaaccord";
const IPAddress ip(192, 168, 4, 1);

const char indexHtmlPath[] = "/index.html";

RbgwLight light(LedOutGPIO, 2);
EspNetwork network(ap_name, ap_password, ip);

//WebServer server(80);

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

      auto c = light.Update(seat, r, b, g);
      Serial.println(buildJsonResponse(*c));
    }
  }
}

void handleColorChangeRequest()
{
  // long r = server.arg("red").toInt();
  // long b = server.arg("blue").toInt();
  // long g = server.arg("green").toInt();
  // long seat = server.arg("seat").toInt();
  
  // auto color = light.Update(seat, r, b, g);

  // server.send(200, "application/json", buildJsonResponse(color));
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  network.Begin();
  light.Begin();

  network.onLedChange([&] (uint8_t r, uint8_t g, uint8_t b, uint8_t seat) { 
    Serial.println("led_request");
    auto c = light.Update(seat, r, g, b);
    //return std::unique_ptr<RgbwColor>(new RgbwColor(0));
    return c;
  });
  // network.onLedChange([] () {
  //   Serial.println("nothing");
  // });
  
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
  network.Loop();
  handleSerialRequest();
}
