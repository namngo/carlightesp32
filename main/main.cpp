#include "Arduino.h"

// #include <NeoPixelBus.h>
// #include <NeoPixelBrightnessBus.h>
// #include <NeoPixelAnimator.h>

#define LED_BUILTIN 2

void setup(){
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop(){
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(10000);                       // wait for a second
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(10000);  
}
