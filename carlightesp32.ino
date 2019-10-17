

#include <DNSServer.h>
#include <DallasTemperature.h>
#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelBus.h>
#include <OneWire.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiUdp.h>

#include <sstream>
#include <string>
#include <vector>

#include "Arduino.h"
#include "FS.h"
#include "esp_sensor.h"
#include "esp_wifi_server.h"
#include "rgbwlight.h"
#include "util.h"

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
EspWifiServer network(ap_name, ap_password, ip);
EspSensor sensor(TemperatureGPIO, DHTGPIO);

const char* CarLight_Default_Setting = R"(
{
  "led_total": 5,
  "leds": [
    {
      "index": 0,
      "name": "Driver front seat",
      "color": 0
    },
    {
      "index": 1,
      "name": "Passenger front seat",
      "color": 0

    },
    {
      "index": 2,
      "name": "Glove box",
      "color": 0
    },
    {
      "index": 3,
      "name": "Driver back seat",
      "color": 0
    },
    {
      "index": 4,
      "name": "Passenger back seat",
      "color": 0
    }
  ]
}
)";

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

String GetSetting() {
  Preferences p;
  p.begin(CAR_APP_NAME, false);
  auto setting = p.getString("setting", "");
  if (setting == "") {
    p.putString("setting", CarLight_Default_Setting);
    setting = CarLight_Default_Setting;
  }
  p.end();

  return setting;
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  network.Begin();
  light.Begin();
  sensor.Begin();

  network.on("/ledchange", HTTP_GET, [&](WebServer& server_) -> String {
    uint8_t r = server_.arg("red").toInt();
    uint8_t b = server_.arg("blue").toInt();
    uint8_t g = server_.arg("green").toInt();
    uint8_t seat = server_.arg("seat").toInt();
    RgbColor rgb_color(r, g, b);

    auto c = light.Update(seat, rgb_color);
    util::SaveColor(rgb_color, seat * 2);
    util::SaveColor(rgb_color, seat * 2 + 1);
    auto respond = util::ColorToJson(c, seat);
    return respond.c_str();
  });

  network.on("/led", HTTP_GET, [&](WebServer& server_) -> String {
    std::string respond("[");
    for (int i = 0; i < light.led_count; i++) {
      auto c = util::GetSavedColor(i);
      respond = respond + util::ColorToJson(c, i) + ",";
    }
    respond = respond + "]";
    return respond.c_str();
  });

  network.on("/sensor", HTTP_GET, [&](WebServer& server_) -> String {
    bool f_temp = server_.arg("f_temp") == "true";
    auto temps = sensor.ReadTemperature(f_temp);
    auto humidity = sensor.ReadHumidity();
    std::stringstream ss;
    ss << "{\"temps\":[";
    for (const auto& temp : temps) {
      ss << temp << ",";
    }
    ss << "],\"humidity\":" << humidity << "}";
    return ss.str().c_str();
  });

  network.on("/setting", HTTP_GET,
             [&](WebServer& server_) -> String { return GetSetting(); });

  network.StartWebServer();
}

// the loop function runs over and over again forever
void loop() {
  network.Loop();
  handleSerialRequest();
}
