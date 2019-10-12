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
#include <sstream>
#include <string>
#include "util.h"

namespace carlight {

namespace {

const byte DNS_PORT = 53;
const byte SERVER_PORT = 80;

} //namespace

class EspNetwork {
 public:
  typedef std::function<std::unique_ptr<RgbwColor>(
      uint8_t r, uint8_t g, uint8_t b, uint8_t seat)> TLedChangeHandler;

  typedef std::function<std::string(WebServer&)> TRequestHandler;

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

  void StartWebServer() {
    server_.begin();
  }

  void on(const String& uri, HTTPMethod method, TRequestHandler fn) {
    server_.on(uri, method, [=] () -> void {
      Serial.println("request: " + uri);
      auto respond = fn(server_);
      server_.send(200, "Application/json", respond.c_str());
      Serial.println("->");
      Serial.println(respond.c_str());
    });
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

} //carlight

#endif
