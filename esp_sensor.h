#ifndef ESPSENSOR_H
#define ESPSENSOR_H

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include <vector>

#define DHTTYPE DHT22  // DHT 22 (AM2302)

namespace carlight {

class EspSensor {
 public:
  EspSensor(const uint16_t dallas_gpio, const uint16_t dht_gpio)
      : dallas_onewire_(dallas_gpio),
        dht_(dht_gpio, DHTTYPE),
        da_(&dallas_onewire_) {}

  void Begin() {
    dht_.begin();
    da_.begin();
    num_of_dallas_ = da_.getDeviceCount();
  }

  std::vector<float> ReadTemperature(bool f_temp = true) {
    std::vector<float> r;
    r.reserve(num_of_dallas_ + 1);

    auto dht_temp = dht_.readTemperature();
    if (!isnan(dht_temp)) {
      r.push_back(dht_.readTemperature(f_temp));
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

  float ReadHumidity() { return dht_.readHumidity(); }

 private:
  OneWire dallas_onewire_;
  DallasTemperature da_;
  DHT dht_;
  uint8_t num_of_dallas_;
};

}  // namespace carlight

#endif
