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
};

}  // namespace carlight

#endif
