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
        da_(&dallas_onewire_),
        da_addr_{} {}

  void Begin() {
    da_.begin();
    num_of_dallas_ = da_.getDeviceCount();
    da_addr_.reserve(num_of_dallas_);

    for (auto i = 0; i < num_of_dallas_; i++) {
      DeviceAddress addr;
      if (da_.getAddress(addr, i)) {
        da_addr_.push_back(addr);
      }
    }
    dht_.begin();
  }

  std::vector<float> ReadTemperature(bool f_temp = true) {
    std::vector<float> r;
    r.reserve(num_of_dallas_ + 1);

    r.push_back(dht_.readTemperature(f_temp));
    da_.requestTemperatures();
    for (const auto* addr : da_addr_) {
      if (f_temp) {
        r.push_back(da_.getTempF(addr));
      } else {
        r.push_back(da_.getTempC(addr));
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
  std::vector<uint8_t*> da_addr_;
};

}  // namespace carlight

#endif
