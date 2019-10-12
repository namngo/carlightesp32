#ifndef CARLIGHTUTIL_H
#define CARLIGHTUTIL_H

#include "Arduino.h"

#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>
#include <sstream>
#include <string>
#include <Preferences.h>
#define CAR_APP_NAME "CarLight"

namespace carlight {

class util {
 public:
  static std::string ColorToJson(const RgbwColor& c, const uint8_t seat = -1) {
    std::stringstream ss;
    ss << "{\"r\":\"" << (int)c.R << "\",\"b\":\""
       << (int)c.B << "\",\"g\":\"" << (int)c.G
       << "\",\"w\":\"" << (int)c.W << "\"";
    if (seat != -1) {
      ss << ",\"seat\":\"" << (int)seat << "\"";
    }
    ss << "}";
    return ss.str();
  }

  static RgbColor StringToColor(const String& str) {
    if (str.length() < 6) {
      return RgbColor(0);
    }
    long r = strtol(&str.substring(0, 2)[0], NULL, 16);
    long g = strtol(&str.substring(2, 4)[0], NULL, 16);
    long b = strtol(&str.substring(4, 6)[0], NULL, 16);
    return RgbColor(r, g, b);
  }

  static void SaveColor(const uint8_t r, const uint8_t g, const uint8_t b,
                   const uint8_t led_pos) {
    Preferences preferences;
    preferences.begin(APP_NAME, false);
    char key[] = {led_pos, 0};
    key[1] = 'r';
    preferences.putUChar(key, r);
    key[1] = 'g';
    preferences.putUChar(key, g);
    key[1] = 'b';
    preferences.putUChar(key, g);
    preferences.end();
  }

  static RgbColor GetSavedColor(uint8_t led_pos) {
    Preferences preferences;
    preferences.begin(APP_NAME, false);
    char key[] = {led_pos, 0};
    key[1] = 'r';
    auto r = preferences.getUChar(key);
    key[1] = 'g';
    auto g = preferences.getUChar(key);
    key[1] = 'b';
    auto b = preferences.getUChar(key);
    preferences.end();

    return RgbColor(r, g, b);
  }

};

} //carlight

#endif
