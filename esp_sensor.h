#ifndef ESPSENSOR_H
#define ESPSENSOR_H

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <vector>

#include <vector>

#define DHTTYPE DHT22  // DHT 22 (AM2302)

namespace carlight {

class EspSensor {
 public:
  EspSensor(uint16_t dallas_gpio, const std::vector<uint16_t>& dht_gpios,
            uint16_t light_gpio)
      : dallas_onewire_(dallas_gpio),
        dhts_(),
        da_(&dallas_onewire_),
        light_gpio_(light_gpio) {
    for (auto gpio : dht_gpios) {
      dhts_.push_back(DHT(gpio, DHTTYPE));
    }
  }

  void Begin() {
    for (auto& dht_ : dhts_) {
      dht_.begin();
    }
    da_.begin();
    num_of_dallas_ = da_.getDeviceCount();
    pinMode(light_gpio_, INPUT);
  }

  std::vector<float> ReadTemperature(bool f_temp = true) {
    std::vector<float> r;
    r.reserve(num_of_dallas_ + 1);

    for (auto& dht_ : dhts_) {
      auto dht_temp = dht_.readTemperature();
      if (!isnan(dht_temp)) {
        r.push_back(dht_.readTemperature(f_temp));
      }
    }
    da_.requestTemperatures();
    DeviceAddress da_address;
    for (auto i = 0; i < num_of_dallas_; i++) {
      if (da_.getAddress(da_address, i)) {
        if (f_temp) {
          r.push_back(da_.getTempF(da_address));
        } else {
          r.push_back(da_.getTempC(da_address));
        }
      }
    }
    return r;
  }

  float ReadHumidity() {
    int sensor_count = 0;
    float total_hums = 0;
    for (auto& dht_ : dhts_) {
      auto hum = dht_.readHumidity();
      if (!isnan(hum)) {
        total_hums += hum;
        sensor_count++;
      }
    }
    return total_hums / sensor_count;
  }

  uint16_t readBrightness() { return analogRead(light_gpio_); }

 private:
  OneWire dallas_onewire_;
  DallasTemperature da_;
  std::vector<DHT> dhts_;
  uint8_t num_of_dallas_;
  uint16_t light_gpio_;
};

}  // namespace carlight

#endif
