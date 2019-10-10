#ifndef CARLIGHTUTIL_H
#define CARLIGHTUTIL_H

#include "Arduino.h"

#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>
#include <sstream>
#include <string>

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

};

} //carlight

#endif
