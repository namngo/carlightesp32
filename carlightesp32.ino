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
typedef StaticJsonDocument<LED_CONFIG_CAP> LedSettingJson;

const size_t SENSOR_JSON_CAP =
    2 * JSON_ARRAY_SIZE(7) + JSON_OBJECT_SIZE(5) + 60;
typedef StaticJsonDocument<SENSOR_JSON_CAP> SensorJson;

const uint16_t SEAT_COUNT = 5;
const uint16_t LedOutGPIO = 22;  // D22
const uint16_t DallasGPIO = 16;
const std::vector<uint16_t> DHTGPIOs = {21, 19};
const uint16_t LightSensorGPIO = 18;

const char ap_name[] = "nango_car_led";
const char ap_password[] = "hondaaccord";
const IPAddress ip(192, 168, 4, 1);

EspSetting& setting = EspSetting::get();

RbgwLight light(LedOutGPIO, SEAT_COUNT);
EspSensor sensor(DallasGPIO, DHTGPIOs);

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
      "index": 2,
      "name": "Passenger front seat",
      "color": ""
    },
    {
      "index": 1,
      "name": "Glove box",
      "color": ""
    },
    {
      "index": 4,
      "name": "Driver back seat",
      "color": ""
    },
    {
      "index": 3,
      "name": "Passenger back seat",
      "color": ""
    }
  ]
}
)";

Controller controller;

// Loads the color of the led and fill them to the json
String GetLed() {
  LedSettingJson doc;
  deserializeJson(doc, CarLight_Config);
  doc["seat_total"] = SEAT_COUNT;

  auto led_arr = doc["leds"].as<JsonArray>();
  if (led_arr.size() != SEAT_COUNT) {
    return "mismatch SeatCount and led config";
  }

  for (auto i = 0; i < SEAT_COUNT; i++) {
    int16_t cur_index = led_arr[i]["index"];
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

  for (int i = 0; i < SEAT_COUNT; i++) {
    light.Update(i, setting.GetSeatColor(i));
  }

  sensor.Begin();

  controller.addServer(std::make_unique<SerialServer>());
  controller.addServer(
      std::make_unique<EspWifiServer>(ap_name, ap_password, ip));

  controller.onPostJson(
      "/api/led", [&](const String& url, IServer::ParamMap& params) -> String {
        int16_t seat = params["seat"].toInt();
        auto color_str = params["color"];
        light.Update(seat, color_str);
        setting.SetSeatColor(seat, color_str);
        return "{\"seat\":" + String(seat) + ",\"color\":\"" + color_str +
               "\"}";
      });

  controller.onGetJson(
      "/api/sensor",
      [&](const String& url, IServer::ParamMap& param) -> String {
        auto f_temps = sensor.ReadTemperature(true);
        auto c_temps = sensor.ReadTemperature(false);
        auto humidity = sensor.ReadHumidity();

        SensorJson doc;

        auto f_temp_json = doc.createNestedArray("f_temp");
        float f_temp_sum = 0;
        for (const auto& temp : f_temps) {
          f_temp_json.add(temp);
          f_temp_sum += temp;
        }
        auto f_temp_avg = f_temp_sum / f_temps.size();
        doc["f_temp_avg"] = serialized(String(f_temp_avg, 2));

        auto c_temp_json = doc.createNestedArray("c_temp");
        float c_temp_sum = 0;
        for (const auto& temp : c_temps) {
          c_temp_json.add(temp);
          c_temp_sum += temp;
        }
        auto c_temp_avg = c_temp_sum / c_temps.size();
        doc["c_temp_avg"] = serialized(String(c_temp_avg, 2));

        doc["humidity"] = humidity;

        String sensor_resp;
        serializeJson(doc, sensor_resp);

        return sensor_resp;
      });

  controller.onGetJson(
      "/api/ledsetting",
      [&](const String& url, const IServer::ParamMap& params) -> String {
        return GetLed();
      });

  controller.Begin();
}

void loop() { controller.Loop(); }
