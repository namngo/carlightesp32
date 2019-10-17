

#include <DNSServer.h>
#include <DallasTemperature.h>
#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelBus.h>
#include <OneWire.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include <string>
#include <vector>
#include <sstream>
#include <string>

#include "Arduino.h"
#include "FS.h"
#include "espnetwork.h"
#include "rgbwlight.h"
#include "util.h"
#include "EspSensor.h"

using namespace carlight;

const uint16_t SeatCount = 2;
const uint16_t LedOutGPIO = 22;  // D22
const uint16_t TemperatureGPIO = 16;
const uint16_t DHTGPIO = 21;
const uint16_t LightSensorGPIO = 19;

const char ap_name[] = "nango_car_led";
const char ap_password[] = "hondaaccord";
const IPAddress ip(192, 168, 4, 1);

RbgwLight light(LedOutGPIO, SeatCount);
EspNetwork network(ap_name, ap_password, ip);
EspSensor sensor(TemperatureGPIO, DHTGPIO);

void handleSerialRequest() {
  while (Serial.available()) {
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
  sensor.Begin();

  network.on("/ledchange", HTTP_GET, [&](WebServer& server_) -> std::string {
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

  network.on("/led", HTTP_GET, [&](WebServer& server_) -> std::string {
    std::string respond("[");
    for (int i = 0; i < light.led_count; i++) {
      auto c = util::GetSavedColor(i);
      respond = respond + util::ColorToJson(c, i) + ",";
    }
    respond = respond + "]";
    return respond;
  });

  network.on("/sensors", HTTP_GET, [&](WebServer& server_) -> std::string {
    bool f_temp = server_.arg("f_temp") == "true";
    auto temps = sensor.ReadTemperature(f_temp);
    auto humidity = sensor.ReadHumidity();
    std::stringstream ss;
    ss << "{\"temps\":[";
    for(const auto& temp: temps) {
      ss << temp << ",";
    }
    ss << "],\"humidity\":" << humidity << "}";
    return ss.str();
  });

  network.StartWebServer();
}

// the loop function runs over and over again forever
void loop() {
  network.Loop();
  handleSerialRequest();
}
