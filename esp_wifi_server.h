#ifndef ESPWIFISERVER_H
#define ESPWIFISERVER_H

#include <DNSServer.h>
#include <NeoPixelBus.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiServer.h>

#include <functional>
#include <iterator>
#include <map>
#include <sstream>
#include <string>

#include "Arduino.h"
#include "iserver.h"
#include "util.h"

namespace carlight {

namespace {

const byte DNS_PORT = 53;
const byte SERVER_PORT = 80;

}  // namespace

class EspWifiServer : public IServer {
 public:
  typedef std::function<String(WebServer &)> TRequestHandler;

  EspWifiServer(const char *ap_name, const char *ap_password,
                const IPAddress &ip)
      : ap_name_(ap_name),
        ap_password_(ap_password),
        ip_(ip),
        subnet_(255, 255, 255, 0),
        server_(SERVER_PORT) {}

  void Begin() {
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(ap_name_, ap_password_, 1, 0, 10)) {
      Serial.println("Cannot setup softAP.");
    }
    delay(500);  // required after v1.0.4
    WiFi.softAPConfig(ip_, ip_, subnet_);
    auto ap_ip = WiFi.softAPIP();

    Serial.println("AP_IP: " + ap_ip.toString());

    dnsServer_ = DNSServer();
    dnsServer_.start(DNS_PORT, "*", ap_ip);

    SPIFFS.begin();
    server_.serveStatic("/static/", SPIFFS, "/static/");
    server_.serveStatic("/index.htm", SPIFFS, "/index.htm");

    server_.begin();
  }

  void Loop() {
    dnsServer_.processNextRequest();
    server_.handleClient();
  }

  void on(const String &uri, HTTPMethod method, TRequestHandler fn) {
    server_.on(uri, method, [=]() -> void {
      Serial.println("request: " + uri);
      auto respond = fn(server_);
      server_.send(200, "Application/json", respond);
      Serial.println("->");
      Serial.println(respond);
    });
  }

  void onGetJson(const String &url, TJsonGettHandler handler) {
    server_.on(url, HTTP_GET, [=]() -> void {
      const auto& current_url = server_.uri();
      Serial.println("request_get: " + current_url);
      std::map<String, String> params;
      for (auto i = 0; i < server_.args(); i++) {
        params.insert(std::make_pair(server_.argName(i), server_.arg(i)));
      }
      auto response = handler(current_url, params);
      Serial.println(response);
      server_.send(200, "Application/json", response);
    });
  }

  void onPostJson(const String &url, TJsonPostHandler handler) {}

 private:
  WebServer server_;
  DNSServer dnsServer_;
  const char *ap_name_;
  const char *ap_password_;
  const IPAddress ip_;
  const IPAddress subnet_;
};

}  // namespace carlight

#endif
