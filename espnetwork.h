#ifndef ESPNETWORK_H
#define ESPNETWORK_H

#include "Arduino.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFiServer.h>
#include <NeoPixelBus.h>
#include <functional>

namespace {

const byte DNS_PORT = 53;
const byte SERVER_PORT = 80;

} //namespace

class EspNetwork {
 public:
  typedef std::function<void()> TLedChangeHandler;

  EspNetwork(const char* ap_name, const char* ap_password,
      const IPAddress& ip) : ap_name_(ap_name), ap_password_(ap_password), 
      ip_(ip), subnet_(255, 255, 255, 0),
      server_(SERVER_PORT) {}
  
  IPAddress Begin() {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(ap_name_, ap_password_, 1, 0, 10)) {
      Serial.println("Cannot setup softAP.");
    }
    delay(500); // required after v1.0.4
    WiFi.softAPConfig(ip_, ip_, subnet_);
    auto ap_ip = WiFi.softAPIP();

    Serial.println("AP_IP: " + ap_ip.toString());

    dnsServer_ = DNSServer();
    dnsServer_.start(DNS_PORT, "*", ap_ip);

    SetupServer();
    return ap_ip;
  }

  void Loop() {
    dnsServer_.processNextRequest();
    server_.handleClient();
  }

  void onLedChange(TLedChangeHandler handler) {
    server_.on("/led", [&] () {
      uint8_t r = server_.arg("red").toInt();
      uint8_t b = server_.arg("blue").toInt();
      uint8_t g = server_.arg("green").toInt();
      uint8_t seat = server_.arg("seat").toInt();
      Serial.println("r" + String(r) + ":b" + String(b) + ":g" + String(g));
      
//      handler(r, g, b, seat);
      handler();
      /*
      
PC: 0x3ffc1754
EXCVADDR: 0x3ffc1754

Decoding stack results
0x400d1591: std::_Function_handler  )::{lambda()#1}>::_M_invoke(std::_Any_data const&) at c:\users\nam\appdata\local\arduino15\packages\esp32\tools\xtensa-esp32-elf-gcc\1.22.0-80-g6c4433a-5.2.0\xtensa-esp32-elf\include\c++\5.2.0/functional line 1871
0x400d83a3: std::function ::operator()() const at c:\users\nam\appdata\local\arduino15\packages\esp32\tools\xtensa-esp32-elf-gcc\1.22.0-80-g6c4433a-5.2.0\xtensa-esp32-elf\include\c++\5.2.0/functional line 2271
0x400d8449: FunctionRequestHandler::handle(WebServer&, HTTPMethod, String) at C:\Users\nam\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\libraries\WebServer\src\detail/RequestHandlersImpl.h line 81
0x400d84b6: WebServer::_handleRequest() at C:\Users\nam\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\libraries\WebServer\src\WebServer.cpp line 620
0x400d861f: WebServer::handleClient() at C:\Users\nam\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\libraries\WebServer\src\WebServer.cpp line 309
0x400d1eba: loop() at f:\repo\carlightesp32\output\sketch/espnetwork.h line 49
0x400db6a9: loopTask(void*) at C:\Users\nam\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\cores\esp32\main.cpp line 19
0x40088bf5: vPortTaskWrapper at /home/runner/work/esp32-arduino-lib-builder/esp32-arduino-lib-builder/esp-idf/components/freertos/port.c line 143

      */
        Serial.println("done");
      // String respond = "{\"red\":\"" + String(c->R) + "\",\"blue\":\""
      //         + String(c->B) + "\",\"green\":\""
      //         + String(c->G) + "\",\"white\":\""
      //         + String(c->W) + "\", \"seat\":\""
      //         + String(seat) + "\"}";
      // Serial.println(respond);
      //return respond;
    });
    server_.begin();
  }
  
 private:
  void SetupServer() {
    SPIFFS.begin();
    server_.serveStatic("/static/", SPIFFS, "/static/");
    server_.serveStatic("/index.htm", SPIFFS, "/index.htm");
  }
  WebServer server_;
  DNSServer dnsServer_;
  const char* ap_name_;
  const char* ap_password_;
  const IPAddress ip_;
  const IPAddress subnet_;
};

#endif
