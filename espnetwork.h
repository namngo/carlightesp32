#ifndef ESPNETWORK_H
#define ESPNETWORK_H

#include "Arduino.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFiServer.h>

namespace {

const byte DNS_PORT = 53;
const byte SERVER_PORT = 80;

} //namespace

class EspNetwork {
 public:
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

    SPIFFS.begin();

    //server_ = WebServer(SERVER_PORT);
    server_.serveStatic("/", SPIFFS, "/");
    // server_.serveStatic("/static/jquery-3.2.1.min.js", SPIFFS, 
    //                   "/static/jquery-3.2.1.min.js", "max-age=86400");
    // server_.serveStatic("/static/colorpicker.min.js", SPIFFS,
    //                   "/static/colorpicker.min.js", "max-age=86400");
    // server_.serveStatic("/static/colorpicker.min.css", SPIFFS, 
    //                   "/static/colorpicker.min.css", "max-age=86400");
    // server_.serveStatic("/static/bootstrap.min.js", SPIFFS,
    //                   "/static/bootstrap.min.js", "max-age=86400");
    // server_.serveStatic("/static/bootstrap.min.css", SPIFFS, 
    //                   "/static/bootstrap.min.css", "max-age=86400");

    // server_.on("/led", handleColorChangeRequest);

    // server_.on("/index.html", [] () {
    //   server_.send(200, "text/html", responseHTML);
    // });
    server_.begin();
    return ap_ip;
  }

  void Loop() {
    dnsServer_.processNextRequest();
    server_.handleClient();
  }
  
 private:
  WebServer server_;
  DNSServer dnsServer_;
  const char* ap_name_;
  const char* ap_password_;
  const IPAddress ip_;
  const IPAddress subnet_;
};

#endif
