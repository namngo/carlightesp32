#ifndef RGBWLIGHT_H
#define RGBWLIGHT_H

#define MAX_SEATS 8
#define LED_PER_SEAT 2
#define APP_NAME "CarLight"

#include "Arduino.h"
#include <EEPROM.h>
#include <NeoPixelBus.h>
#include <Preferences.h>
#include <sstream>
#include <string>

namespace carlight {

class RbgwLight {

 public:
  typedef NeoPixelBus<NeoGrbwFeature, NeoSk6812Method> Strip;

  RbgwLight(uint16_t gpio, uint16_t seat_count) :
      seat_count_(seat_count),
      strip_(Strip(seat_count * LED_PER_SEAT, gpio)) {
  }

  ~RbgwLight() {
  }

  void Begin() {
    strip_.Begin();

    Serial.println("creating light");

    preferences.begin(APP_NAME, false);
    char key[] = {0, 0};
    for(int i = 0; i < seat_count_ * LED_PER_SEAT; i ++) {
      key[0] = i;
      key[1] = 'r';
      auto r = preferences.getUChar(key);
      key[1] = 'g';
      auto g = preferences.getUChar(key);
      key[1] = 'b';
      auto b = preferences.getUChar(key);
      key[1] = 'w';
      auto w = preferences.getUChar(key);
      strip_.SetPixelColor(i, RgbwColor(r, g, b, w));
    }
    preferences.end();
    strip_.Show();
  }

  std::unique_ptr<RgbwColor> Update(uint16_t seat, uint16_t r, uint16_t g, uint16_t b) {
    std::ostringstream ss;
    ss << "Updating: r:" << r << ", g:" << g
       << ", b:" << b << ", seat:" << seat;
    Serial.println(ss.str().c_str());
    auto color = rgbwFromRgb(r, g, b);
    SaveColor(seat * 2, *color);
    strip_.SetPixelColor(seat * 2, *color);
    SaveColor(seat * 2 + 1, *color);
    strip_.SetPixelColor(seat * 2 + 1, *color);
    strip_.Show();
    return color;
  }

 private:

  void SaveColor(uint16_t led_pos, const RgbwColor& color) {
    preferences.begin(APP_NAME, false);
    char key[] = {led_pos, 0};
    key[1] = 'r';
    preferences.putUChar(key, color.R);
    key[1] = 'g';
    preferences.putUChar(key, color.G);
    key[1] = 'b';
    preferences.putUChar(key, color.B);
    key[1] = 'w';
    preferences.putUChar(key, color.W);
    preferences.end();
  }

  template <class T>
  T getMin(const T a, const T b, const T c) const
  {
    return std::min(a, std::min(b, c));
  }

  template <class T>
  T getMax(const T a, const T b, const T c) const
  {
    return std::max(a, std::max(b, c));
  }

  template <class T>
  T boundValue(T value, T min, T max) const {
      if (value < min) return min;
      if (value > max) return max;
      return value;
  }

  // https://stackoverflow.com/questions/21117842/converting-an-rgbw-color-to-a-standard-rgb-hsb-rappresentation
  std::unique_ptr<RgbwColor> rgbwFromRgb(const long ir, const long ig,
                                                   const long ib) const {
    long tempMax = getMax(ir, ig, ib);

    if (tempMax == 0) {
        return std::unique_ptr<RgbwColor>(new RgbwColor(0));
    }

    float mul = 255.0f / (float)tempMax;
    float hR = ir * mul;
    float hG = ig * mul;
    float hB = ib * mul;

    float max = getMax(hR, hG, hB);
    float min = getMin(hR, hG, hB);
    float lum =  ((max + min) / 2.0f - 127.5f) * (255.0f/127.5f) / mul;

    long w = (long)lum;
    long r = floor(ir - lum);
    long g = floor(ig - lum);
    long b = floor(ib - lum);

    w = boundValue(w, 0L, 255L);
    r = boundValue(r, 0L, 255L);
    g = boundValue(g, 0L, 255L);
    b = boundValue(b, 0L, 255L);

    return std::unique_ptr<RgbwColor>(new RgbwColor(r, g, b, w));
  }

  Strip strip_;
  Preferences preferences;
  const uint16_t seat_count_;
};

} //carlight

#endif
