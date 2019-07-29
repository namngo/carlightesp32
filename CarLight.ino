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

const byte DNS_PORT = 53;

const uint16_t PixelCount = 6;
const uint16_t LedOutGPIO = 22; // D22

const char WiFiAPPID[] = "nango_car_led";
const char WiFiAPPSK[] = "hondaaccord";
const char indexHtmlPath[] = "/index.html";

NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> strip(PixelCount, LedOutGPIO);
RgbwColor black(128, 128, 128, 128);

IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
WebServer server(80);

String responseHTML = ""
"<!DOCTYPE html><html><head><title>CaptivePortal</title></head><body>"
"<p>No index.html found.</p></body></html>";

long min(long a, long b, long c)
{
    return std::min(a, std::min(b, c));
}

long max(long a, long b, long c)
{
    return std::max(a, std::max(b, c));
}

// https://stackoverflow.com/questions/21117842/converting-an-rgbw-color-to-a-standard-rgb-hsb-rappresentation
RgbwColor rgbwFromRgb(long ir, long ig, long ib)
{
   /* long M = max(ir, ig, ib);
    long m = min(ir, ig, ib);

    long w = M;
    if (2 * m < M)
    {
        w = (m * M) / (M - m);
    }

    long Q = 64;
    long K = (w + M) / m;

    long r = floor((K * ir - w) / Q);
    long g = floor((K * ig - w) / Q);
    long b = floor((K * ib - w) / Q);

    return RgbwColor(r, g, b, w);*/

    long m = min(ir, ig, ib);
    return RgbwColor(ir, ig, ib, m);
}

String buildJsonResponse(long red, long blue, long green, long alpha)
{
    return "{\"red\":\"" + String(red) + "\",\"blue\":\"" + String(blue) + "\",\"green\":\"" + String(green) + "\",\"alpha\":\"" + String(alpha) + "\"}";
}

void handleColorChangeRequest()
{
    long red = server.arg("red").toInt();
    long blue = server.arg("blue").toInt();
    long green = server.arg("green").toInt();
    
    

    RgbwColor color = rgbwFromRgb(red, green, blue);

    //Serial.println("ir=" + String(red) + "ig=" + String(green) + "ib=" + String(blue) + "nr=" + String(color.R) + "ng=" + String(color.G) + "nb=" + String(color.B) + "nw=" + String(color.W));

    strip.ClearTo(color);
    strip.Show();

    server.send(200, "application/json", buildJsonResponse(color.R, color.B, color.G, color.W));
}

#define LED_BUILTIN 2
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
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

    server.serveStatic("/static", SPIFFS, "/static", "max-age=86400");

    server.on("/led", handleColorChangeRequest);

    server.onNotFound([]() {
        server.send(200, "text/html", responseHTML);
    });

    server.begin();
}

// the loop function runs over and over again forever
void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}
