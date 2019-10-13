#ifndef SENSORS_H
#define SENSORS_H


#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>

#include <DHT_U.h>
#include <DHT.h>

#define DHTTYPE    DHT22     // DHT 22 (AM2302)


namespace carlight {


class Sensor {
 public:
  Sensor(const uint16_t dallas_gpio, const uint16_t dht_gpio) : 
      dallas_onewire_(dallas_gpio), dht_(dht_gpio, DHTTYPE),
      dallas_(&dallas_onewire_)
     {}
  
  void Begin() {
    dallas_.begin();
    num_of_dallas_ = dallas_.getDeviceCount();
    dht_.begin();
  }

  std::vector<float> ReadDHT() {
    std::vector<float> r = {dht_.readTemperature(), dht_.readHumidity()};
    return r;
  }

  std::vector<float> ReadDallas() {
    std::vector<float> r;
    r.reserve(num_of_dallas_);
    dallas_.requestTemperatures();
    for(int i = 0; i < num_of_dallas_; i ++) {
      r.push_back(dallas_.getTempF())
    }

    return r;

  }

  void Loop() {

  }

 private:
  OneWire dallas_onewire_;
  DallasTemperature dallas_;
  DHT dht_;
  uint8_t num_of_dallas_;
  DeviceAddress

};

} // carlight

#endif
