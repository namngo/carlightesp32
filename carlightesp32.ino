#include <ArduinoJson.h>
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

#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Arduino.h"
#include "FS.h"
#include "esp_controller.h"
#include "esp_sensor.h"
#include "esp_setting.h"
#include "esp_wifi_server.h"
#include "iserver.h"
#include "rgbwlight.h"
#include "serial_server.h"
#include "util.h"

using namespace carlight;

const size_t LED_CONFIG_CAP = JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(2) +
                              10 * JSON_OBJECT_SIZE(3) +
                              220;  // reserve for 10 seats
typedef StaticJsonDocument<LED_CONFIG_CAP> SettingJson;

const uint16_t SeatCount = 5;
const uint16_t LedOutGPIO = 22;  // D22
const uint16_t TemperatureGPIO = 16;
const uint16_t DHTGPIO = 21;
const uint16_t LightSensorGPIO = 19;

const char ap_name[] = "nango_car_led";
const char ap_password[] = "hondaaccord";
const IPAddress ip(192, 168, 4, 1);

EspSetting& setting = EspSetting::get();

RbgwLight light(LedOutGPIO, SeatCount);
EspSensor sensor(TemperatureGPIO, DHTGPIO);

const char* CarLight_Config = R"(
{
  "seat_total": 0,
  "leds": [
    {
      "index": 0,
      "name": "Driver front seat",
      "color": ""
    },
    {
      "index": 1,
      "name": "Passenger front seat",
      "color": ""
    },
    {
      "index": 2,
      "name": "Glove box",
      "color": ""
    },
    {
      "index": 3,
      "name": "Driver back seat",
      "color": ""
    },
    {
      "index": 4,
      "name": "Passenger back seat",
      "color": ""
    }
  ]
}
)";

Controller controller;

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

// Loads the color of the led and fill them to the json
String GetLed() {
  SettingJson doc;
  deserializeJson(doc, CarLight_Config);
  doc["seat_total"] = SeatCount;

  auto led_arr = doc["leds"].as<JsonArray>();
  if (led_arr.size() != SeatCount) {
    return "mismatch SeatCount and led config";
  }

  for (auto i = 0; i < SeatCount; i++) {
    int16_t cur_index = led_arr[i]["index"];
    if (cur_index != i) {
      return "mismatch current_index and led config index";
    }
    led_arr[i]["color"] = setting.GetSeatColor(i);
  }

  String led_setting;
  serializeJson(doc, led_setting);

  return led_setting;
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  light.Begin();

  for (int i = 0; i < SeatCount; i++) {
    light.Update(i, setting.GetSeatColor(i));
  }

  sensor.Begin();

  controller.addServer(std::make_unique<SerialServer>());
  controller.addServer(
      std::make_unique<EspWifiServer>(ap_name, ap_password, ip));

  controller.onGetJson(
      "/api/ledchange",
      [&](const String& url, IServer::ParamMap& params) -> String {
        return "foo";
      });

  controller.onPostJson(
      "/api/led", [&](const String& url, IServer::ParamMap& params) -> String {
        int16_t seat = params["seat"].toInt();
        auto color_str = params["color"];
        light.Update(seat, color_str);
        setting.SetSeatColor(seat, color_str);
        return "{\"seat\":" + String(seat) + ",\"color\":\"" + color_str +
               "\"}";
      });

  // network.on("/sensor", HTTP_GET, [&](WebServer &server_) -> String {
  //   bool f_temp = server_.arg("f_temp") == "true";
  //   auto temps = sensor.ReadTemperature(f_temp);
  //   auto humidity = sensor.ReadHumidity();
  //   std::stringstream ss;
  //   ss << "{\"temps\":[";
  //   for (const auto &temp : temps) {
  //     ss << temp << ",";
  //   }
  //   ss << "],\"humidity\":" << humidity << "}";
  //   return ss.str().c_str();
  // });

  controller.onGetJson(
      "/api/setting",
      [&](const String& url, const IServer::ParamMap& params) -> String {
        return GetLed();
      });

  controller.Begin();
}

void loop() { controller.Loop(); }
