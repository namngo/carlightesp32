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
  
  void Setup() {

  }

  void Loop() {

  }

 private:
  OneWire dallas_onewire_;
  DallasTemperature dallas_;
  DHT dht_;

};

} // carlight

#endif
