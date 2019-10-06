#ifndef ESPNETWORK_H
#define ESPNETWORK_H

#include "Arduino.h"
#include <WiFi.h>
#include <DNSServer.h>

class EspNetwork {
 public:
  EspNetwork(const char* ap_name, const char* ap_password,
      const IPAddress& ip) : ap_name_(ap_name), ap_password_(ap_password), 
      ip_(ip), gate_way_(IPAddress(192, 168, 4, 9)), 
      subnet_(IPAddress(255, 255, 255, 0)) {}
  
  void Begin() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_name_, ap_password_);
  }
  
 private:
  DNSServer dnsServer;
  const char* ap_name_;
  const char* ap_password_;
  const IPAddress ip_;
  const IPAddress gate_way_;
  const IPAddress subnet_;
};

#endif
