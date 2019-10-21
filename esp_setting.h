#ifndef ESPSETTING
#define ESPSETTING

#include <ArduinoJson.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelBus.h>
#include <Preferences.h>
#include "Arduino.h"

#define CAR_APP_NAME "CarLightSetting"

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

  ~EspSetting() { p_.end(); }

  String GetSeatColor(int16_t seat) {
    const String key = "SeatColor_" + String(seat);
    return p_.getString(key.c_str(), "#000000");
  }

  void SetSeatColor(int16_t seat, const String& color) {
    const String key = "SeatColor_" + String(seat);
    p_.putString(key.c_str(), color);
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
  EspSetting() : p_() { p_.begin(CAR_APP_NAME, false); }
  Preferences p_;
};

}  // namespace carlight

#endif