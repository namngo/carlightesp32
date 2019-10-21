#ifndef ESPSETTING
#define ESPSETTING

#include <ArduinoJson.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelBus.h>
#include <Preferences.h>
#include "Arduino.h"

#define CAR_SETTING_APP_NAME "CarLight"

const size_t Json_Setting_Capacity = JSON_ARRAY_SIZE(5) + JSON_OBJECT_SIZE(2) +
                                     10 * JSON_OBJECT_SIZE(3) +
                                     220;  // reserve for 10 seats

namespace carlight {

class EspSetting {
 public:
  typedef StaticJsonDocument<Json_Setting_Capacity> SettingJson;

  static EspSetting& get() {
    static EspSetting instance_;
    return instance_;
  }

  void operator=(EspSetting const&) = delete;
  EspSetting(EspSetting const&) = delete;

  String GetSeatColor(int16_t seat) {
    String key = "SeatColor_" + String(seat);
    return GetKey(key, "#00000");
  }

  void SetSeatColor(int16_t seat, const String& color) {
    String key = "SeatColor_" + String(seat);
    SetKey(key, color);
  }

  // String GetLedConfig(const String& def) {
  //   auto led_setting = p_.getString("LedConfig", def);
  //   SettingJson json;
  //   deserializeJson(json, led_setting);
  //   json["led_total"] = GetTotalSeat();
  //   String output;
  //   serializeJson(json, output);
  //   return output;
  // }

  // String GetConfig(const String& def) { return p_.getString("Config", def); }

  // SettingJson GetJsonConfig(const String& def) {
  //   auto setting = GetConfig(def);
  //   SettingJson json;
  //   deserializeJson(json, setting);
  //   return json;
  // }

 private:
  String GetKey(const String& key, String defaultValue = String()) {
    Preferences p_;
    p_.begin(CAR_SETTING_APP_NAME, true);
    const auto ret = p_.getString(key.c_str());
    p_.end();
    return ret;
  }

  void SetKey(const String& key, const String value) {
    Preferences p_;
    p_.begin(CAR_SETTING_APP_NAME, false);
    const auto ret = p_.putString(key.c_str(), value);
    p_.end();
  }

  EspSetting() {}
};

}  // namespace carlight

#endif