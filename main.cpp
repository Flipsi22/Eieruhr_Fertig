#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include "WebServerHandler.h"
#include <Adafruit_NeoPixel.h>

const char* ssid = "FRITZ!Box 7520 JV";
const char* password = "55509636339168166636";
void updateTimer(); 

// Neopixel configuration
#define LED_PIN 4
#define NUM_LEDS 8
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);


void setup() {
  Serial.begin(115200);
  strip.show(); 
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Verbindung zum WiFi-Netzwerk herstellen...");
  }

  Serial.println("WiFi verbunden");
  Serial.print("IP-Adresse: ");
  Serial.println(WiFi.localIP());

  initWebServer();
}

void loop() {
  updateTimer();
}
