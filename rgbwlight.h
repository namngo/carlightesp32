#ifndef RGBWLIGHT_H
#define RGBWLIGHT_H

#define MAX_SEATS 8
#define LED_PER_SEAT 2
#define APP_NAME "CarLight"

#include <EEPROM.h>
#include <NeoPixelBus.h>
#include <Preferences.h>

#include <sstream>
#include <string>

#include "Arduino.h"
#include "util.h"

namespace carlight {

class RbgwLight {
 public:
  typedef NeoPixelBus<NeoGrbwFeature, NeoSk6812Method> Strip;

  RbgwLight(uint16_t gpio, uint16_t seat_count)
      : seat_count_(seat_count),
        led_count(seat_count * LED_PER_SEAT),
        strip_(Strip(seat_count * LED_PER_SEAT, gpio)) {}

  ~RbgwLight() {}

  void Begin() {
    strip_.Begin();
    for (int i = 0; i < seat_count_ * LED_PER_SEAT; i++) {
      auto c = util::GetSavedColor(i);
      Update(i / 2, c.R, c.G, c.B);
    }

    strip_.Show();
  }

  RgbwColor Update(uint16_t seat, const RgbColor& c) {
    return Update(seat, c.R, c.G, c.B);
  }

  RgbwColor Update(uint16_t seat, uint16_t r, uint16_t g, uint16_t b) {
    auto color = rgbwFromRgb(r, g, b);
    strip_.SetPixelColor(seat * 2, color);
    strip_.SetPixelColor(seat * 2 + 1, color);
    strip_.Show();
    return color;
  }

  RgbwColor Update(uint16_t seat, const String& color_str) {
    HtmlColor h_c;
    auto parse_result = h_c.Parse<HtmlColorNames>(color_str);
    if (parse_result == 0) {
      return RgbwColor();
    }
    RgbColor c(h_c);
    return Update(seat, c.R, c.G, c.B);
  }

  const uint16_t led_count;

 private:
  template <class T>
  T getMin(const T a, const T b, const T c) const {
    return std::min(a, std::min(b, c));
  }

  template <class T>
  T getMax(const T a, const T b, const T c) const {
    return std::max(a, std::max(b, c));
  }

  template <class T>
  T boundValue(T value, T min, T max) const {
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  // https://stackoverflow.com/questions/21117842/converting-an-rgbw-color-to-a-standard-rgb-hsb-rappresentation
  RgbwColor rgbwFromRgb(const long ir, const long ig, const long ib) const {
    long tempMax = getMax(ir, ig, ib);

    if (tempMax == 0) {
      return RgbwColor(0);
    }

    float mul = 255.0f / (float)tempMax;
    float hR = ir * mul;
    float hG = ig * mul;
    float hB = ib * mul;

    float max = getMax(hR, hG, hB);
    float min = getMin(hR, hG, hB);
    float lum = ((max + min) / 2.0f - 127.5f) * (255.0f / 127.5f) / mul;

    long w = (long)lum;
    long r = floor(ir - lum);
    long g = floor(ig - lum);
    long b = floor(ib - lum);

    w = boundValue(w, 0L, 255L);
    r = boundValue(r, 0L, 255L);
    g = boundValue(g, 0L, 255L);
    b = boundValue(b, 0L, 255L);

    return RgbwColor(r, g, b, w);
  }

  Strip strip_;
  const uint16_t seat_count_;
};

}  // namespace carlight

#endif
