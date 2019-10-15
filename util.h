#ifndef CARLIGHTUTIL_H
#define CARLIGHTUTIL_H

#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>
#include <Preferences.h>

#include <sstream>
#include <string>

#include "Arduino.h"
#define CAR_APP_NAME "CarLight"

namespace carlight {

class util {
 public:
  static std::string ColorToJson(const RgbColor& c,
                                 const uint8_t led_pos = -1) {
    std::stringstream ss;
    ss << "{\"r\":\"" << (int)c.R << "\",\"b\":\"" << (int)c.B << "\",\"g\":\""
       << (int)c.G << "\"";
    if (led_pos != -1) {
      ss << ",\"led_pos\":\"" << (int)led_pos << "\"";
    }
    ss << "}";
    return ss.str();
  }

  static std::string ColorToJson(const RgbwColor& c,
                                 const uint8_t led_pos = -1) {
    std::stringstream ss;
    ss << "{\"r\":\"" << (int)c.R << "\",\"b\":\"" << (int)c.B << "\",\"g\":\""
       << (int)c.G << "\",\"w\":\"" << (int)c.W << "\"";
    if (led_pos != -1) {
      ss << ",\"led_pos\":\"" << (int)led_pos << "\"";
    }
    ss << "}";
    return ss.str();
  }

  static void SaveColor(const RgbColor& c, const uint8_t led_pos) {
    HtmlColor hc(c);

    String pos = "led_" + String(led_pos);
    Serial.println("saving: " + pos + ", value: " + String(hc.Color));

    Preferences preferences;
    preferences.begin(APP_NAME, false);

    preferences.putUInt(pos.c_str(), hc.Color);
    preferences.end();
  }

  static RgbColor GetSavedColor(uint8_t led_pos) {
    String pos = "led_" + String(led_pos);

    Preferences preferences;
    preferences.begin(APP_NAME, false);

    uint32_t color_int = preferences.getUInt(pos.c_str());

    preferences.end();

    Serial.println("getting: " + pos + ", value: " + String(color_int));
    HtmlColor hc(color_int);

    return RgbColor(hc);
  }
};

}  // namespace carlight

#endif
