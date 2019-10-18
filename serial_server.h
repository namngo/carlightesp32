#ifndef SERIALSERVER_H
#define SERIALSERVER_H

#include <functional>
#include <map>
#include <string>
#include "iserver.h"

#include "Arduino.h"

namespace carlight {

class SerialServer : public IServer {
  void Begin() {}

  void onGetJson(const String& url, TJsonGettHandler handler) {
    Serial.println("setting up get:" + url);
  }

  void onPostJson(const String& url, TJsonPostHandler handler) {
    Serial.println("setting up post:" + url);
  }

  void Loop() {
    while (Serial.available()) {
      auto str = Serial.readString();
      Serial.println("Got text1:" + str);
    }
  }
};

}  // namespace carlight

#endif
