#ifndef SENSORS_H
#define SENSORS_H

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define DHTTYPE DHT22  // DHT 22 (AM2302)

namespace carlight {

class Sensor {
 public:
  Sensor(const uint16_t dallas_gpio, const uint16_t dht_gpio)
      : dallas_onewire_(dallas_gpio),
        dht_(dht_gpio, DHTTYPE),
        da_(&dallas_onewire_),
        d_addr_() {}

  void Begin() {
    da_.begin();
    num_of_dallas_ = da_.getDeviceCount();
    d_addr_.reserve(num_of_dallas_);
    for (auto i = 0; i < num_of_dallas_; i++) {
      DeviceAddress addr;
      if (da_.getAddress(addr, i)) {
        d_addr_.push_back(addr);
      }
    }
    dht_.begin();
  }

  std::vector<float> ReadTemperature(bool f_temp = true) {
    std::vector<float> r;
    r.reserve(num_of_dallas_ + 1);

    r.push_back(dht_.readTemperature(f_temp));

    da_.requestTemperatures();
    for (const auto* addr : d_addr_) {
      if (f_temp) {
        r.push_back(da_.getTempF(addr));
      } else {
        r.push_back(da_.getTempC(addr));
      }
    }
    return r;
  }

  float ReadHumidity() { return dht_.readHumidity(); }

  void Loop() {}

 private:
  OneWire dallas_onewire_;
  DallasTemperature da_;
  DHT dht_;
  uint8_t num_of_dallas_;
  std::vector<DeviceAddress> d_addr_;
};

}  // namespace carlight

#endif
