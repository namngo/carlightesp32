

#include "Arduino.h"

#include <ArduinoJson.h>
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
#include "util.h"
#include "sensors.h"

using namespace carlight;

const uint16_t SeatCount = 2;
const uint16_t LedOutGPIO = 22; // D22
const uint16_t TemperatureGPIO = 2;

const char ap_name[] = "nango_car_led";
const char ap_password[] = "hondaaccord";
const IPAddress ip(192, 168, 4, 1);


RbgwLight light(LedOutGPIO, SeatCount);
EspNetwork network(ap_name, ap_password, ip);

OneWire oneWire(TemperatureGPIO);
DallasTemperature sensors(&oneWire);

// Number of temperature devices found
int numberOfTempSensor;

// We'll use this variable to store a found device address
DeviceAddress tempDeviceAddress;

void handleSerialRequest()
{
  while(Serial.available()) {
    auto str = Serial.readString();
    Serial.println("Got text:" + str);
    if (str.length() == 8) {
      long r = strtol(&str.substring(0, 2)[0], NULL, 16);
      long g = strtol(&str.substring(2, 4)[0], NULL, 16);
      long b = strtol(&str.substring(4, 6)[0], NULL, 16);
      long seat = strtol(&str.substring(6, 8)[0], NULL, 16);

      auto c = light.Update(seat, r, b, g);
      Serial.println(util::ColorToJson(c).c_str());
    }
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  network.Begin();
  light.Begin();

  network.on("/ledchange", HTTP_GET, [&] (WebServer& server_) -> std::string {
    uint8_t r = server_.arg("red").toInt();
    uint8_t b = server_.arg("blue").toInt();
    uint8_t g = server_.arg("green").toInt();
    uint8_t seat = server_.arg("seat").toInt();
    RgbColor rgb_color(r, g, b);

    auto c = light.Update(seat, rgb_color);
    util::SaveColor(rgb_color, seat * 2);
    util::SaveColor(rgb_color, seat * 2 + 1);
    auto respond = util::ColorToJson(c, seat);
    return respond;
  });

  network.on("/led", HTTP_GET, [&] (WebServer& server_) -> std::string {
    std::string respond("[");
    for(int i = 0; i < light.led_count; i ++) {
      auto c = util::GetSavedColor(i);
      respond = respond + util::ColorToJson(c, i) + ",";
    }
    respond = respond + "]";
    return respond;
  });

  network.StartWebServer();

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
