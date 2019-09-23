#ifndef ARDUINO_ARCH_ESP32
#define ARDUINO_ARCH_ESP32
#endif

#include <NeoPixelAnimator.h>
#include <NeoPixelBrightnessBus.h>
#include <NeoPixelBus.h>

/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://www.arduino.cc/en/Main/Products

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Blink
*/

#include <sstream>
#include "Arduino.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "FS.h"
#include <string>
#include <SPIFFS.h>

using namespace std;

const byte DNS_PORT = 53;

const uint16_t PixelCount = 6;
const uint16_t LedOutGPIO = 22; // D22

const char WiFiAPPID[] = "nango_car_led";
const char WiFiAPPSK[] = "hondaaccord";
const char indexHtmlPath[] = "/index.html";

NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> strip(PixelCount, LedOutGPIO);
RgbwColor black(0);

IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer server(80);

String responseHTML = ""
"<!DOCTYPE html><html><head><title>CaptivePortal</title></head><body>"
"<p>No index.html found.</p></body></html>";

template <class T>
T getMin(T a, T b, T c)
{
    return std::min(a, std::min(b, c));
}

template <class T>
T getMax(T a, T b, T c)
{
    return std::max(a, std::max(b, c));
}

template <class T>
T boundValue(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// https://stackoverflow.com/questions/21117842/converting-an-rgbw-color-to-a-standard-rgb-hsb-rappresentation
RgbwColor rgbwFromRgb(long ir, long ig, long ib)
{
    long tempMax = getMax(ir, ig, ib);

    if (tempMax == 0) {
        return RgbwColor(0);
    }

    float mul = 255.0f / (float)tempMax;
    float hR = ir * mul;
    float hG = ig * mul;
    float hB = ib * mul;

    float max = getMax(hR, hG, hB);
    float min = getMin(hR, hG, hB);
    float lum =  ((max + min) / 2.0f - 127.5f) * (255.0f/127.5f) / mul;

    long w = (long)lum;
    long r = floor(ir - lum);
    long g = floor(ig - lum);
    long b = floor(ib - lum);
    
    w = boundValue(w, 0L, 255L);
    r = boundValue(r, 0L, 255L);
    g = boundValue(g, 0L, 255L);
    b = boundValue(b, 0L, 255L);
    
    return RgbwColor(r, g, b, w);
}

String buildJsonResponse(long red, long blue, long green, long alpha)
{
    return "{\"red\":\"" + String(red) + "\",\"blue\":\"" + String(blue) + "\",\"green\":\"" + String(green) + "\",\"alpha\":\"" + String(alpha) + "\"}";
}

void handleSerialRequest()
{
    while(Serial.available()) {
        //Serial.r
        auto str = Serial.readString();
        
        if (str.length() == 6) {
            long r = strtol(&str.substring(0, 2)[0], NULL, 16);
            long g = strtol(&str.substring(2, 4)[0], NULL, 16);
            long b = strtol(&str.substring(4, 6)[0], NULL, 16);

            RgbwColor color = rgbwFromRgb(r, g, b);

            strip.ClearTo(color);
            strip.Show();

            Serial.println();
        }
    }
}

void handleColorChangeRequest()
{
    long red = server.arg("red").toInt();
    long blue = server.arg("blue").toInt();
    long green = server.arg("green").toInt();
    
    RgbwColor color = rgbwFromRgb(red, green, blue);

    strip.ClearTo(color);
    strip.Show();

    server.send(200, "application/json", buildJsonResponse(color.R, color.B, color.G, color.W));
}

#define LED_BUILTIN 2
// the setup function runs once when you press reset or power the board
void setup() {
  
    SPIFFS.begin();
    Serial.begin(115200);
  
    strip.Begin();
    strip.ClearTo(black);
    strip.Show();

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(WiFiAPPID, WiFiAPPSK);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  // setup the index.html
    if (SPIFFS.begin() && SPIFFS.exists(indexHtmlPath))
    {
        File f = SPIFFS.open(indexHtmlPath, "r");
        if (f)
        {
            responseHTML = f.readString();
        }
    }

    //server.serveStatic("/static", SPIFFS, "/static", "max-age=86400");
    server.serveStatic("/static/jquery-3.2.1.min.js", SPIFFS, "/static/jquery-3.2.1.min.js", "max-age=86400");
    server.serveStatic("/static/colorpicker.min.js", SPIFFS, "/static/colorpicker.min.js", "max-age=86400");
    server.serveStatic("/static/colorpicker.min.css", SPIFFS, "/static/colorpicker.min.css", "max-age=86400");
    server.serveStatic("/static/bootstrap.min.js", SPIFFS, "/static/bootstrap.min.js", "max-age=86400");
    server.serveStatic("/static/bootstrap.min.css", SPIFFS, "/static/bootstrap.min.css", "max-age=86400");

    server.on("/led", handleColorChangeRequest);

    server.on("/index.html", [] () {
        server.send(200, "text/html", responseHTML);
    });

    // server.onNotFound([]() {
        
    // });

    server.begin();

    Serial.println();
}

// the loop function runs over and over again forever
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  handleSerialRequest();
}
