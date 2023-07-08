#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "WebServerHandler.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "de.pool.ntp.org");

AsyncWebServer server(80);

extern Adafruit_NeoPixel strip;
const int NUM_LEDS = 8;

int targetValue = 0; // Target value received from the webpage
int currentValue = 0; // Current value for the Neopixel animation
unsigned long startTime = 0; // Start time of the timer

void initWebServer() {
  timeClient.setTimeOffset(7200); // Zeitversatz für Mitteleuropäische Zeit (MEZ)
  timeClient.begin();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request){
    const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background-color: #f2f2f2;
    }

    h1 {
      color: #333333;
    }

    label {
      font-weight: bold;
    }

    input[type="number"] {
      padding: 5px;
      width: 100px;
    }

    button {
      padding: 8px 16px;
      background-color: #4CAF50;
      color: #ffffff;
      border: none;
      cursor: pointer;
      margin-top: 10px;
    }

    button:hover {
      background-color: #45a049;
    }

    p {
      margin-top: 20px;
    }

    #currentTime {
      font-size: 24px;
      font-weight: bold;
    }
  </style>

  <script>
    function setTargetValue() {
      var targetValue = document.getElementById("targetValue").value;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/setTargetValue?value=" + targetValue, true);
      xhr.send();
    }

    function startTimer() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/startTimer", true);
      xhr.send();
    }

    function updateTime() {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var time = this.responseText;
          document.getElementById("currentTime").textContent = time;
        }
      };
      xhr.open("GET", "/getTime", true);
      xhr.send();
    }

    setInterval(updateTime, 1000);
  </script>
</head>
<body>
  <h1>Online abrufbarer Timer</h1>
  <label for="targetValue">Zeit auswählen (in seconds):</label>
  <input type="number" id="targetValue" name="targetValue">
  <button onclick="setTargetValue()">Set</button>
  <button onclick="startTimer()">Start</button>
  <p>Restliche Zeit: <span id="currentTime"></span></p>
</body>
</html>
)rawliteral";

    request->send(200, "text/html", html);
  });

  server.on("/setTargetValue", HTTP_GET, [](AsyncWebServerRequest* request){
    String valueStr = request->getParam("value")->value();
    targetValue = valueStr.toInt();
    request->send(200, "text/plain", "OK");
  });

  server.on("/startTimer", HTTP_GET, [](AsyncWebServerRequest* request){
    startTime = millis();
    currentValue = targetValue;
    request->send(200, "text/plain", "OK");
  });

  server.on("/getTime", HTTP_GET, [](AsyncWebServerRequest* request){
    unsigned long currentTime = millis();
    unsigned long elapsedTime = (currentTime >= startTime) ? (currentTime - startTime) : 0;
    unsigned long remainingTime = (elapsedTime >= targetValue * 1000) ? 0 : (targetValue * 1000 - elapsedTime);
    String time = String(remainingTime / 1000);
    request->send(200, "text/plain", time);
  });

  server.begin();
}

void handleWebServer(unsigned long remainingTime) {
  timeClient.update();

  // Neopixel animation
  if (currentValue > 0) {
    currentValue--;
  }

  int red = 0;
  int green = 0;
  int blue = 0;
  
  int ledIndex = (targetValue > 0) ? NUM_LEDS - (remainingTime / (targetValue * 1000 / NUM_LEDS)) : 0;
  if (ledIndex >= NUM_LEDS) {
    ledIndex = NUM_LEDS - 1;
  }
  
  // Prüfen, ob der Timer gestartet wurde
  if (targetValue > 0 && ledIndex >= 0) {
    if (ledIndex < 2) {
      red = 255;
    } else if (ledIndex < 6) {
      red = 255;
      green = 165; // Orange
    } else {
      green = 255;
    }
  }
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i <= ledIndex) {
      strip.setPixelColor(i, red, green, blue);
    } else {
      strip.setPixelColor(i, 0, 0, 0); // Ausschalten der nicht aktivierten LEDs
    }
  }
  strip.show();
}


void updateTimer() {
  if (startTime > 0) {
    unsigned long currentTime = millis();
    unsigned long elapsedTime = (currentTime >= startTime) ? (currentTime - startTime) : 0;
    unsigned long remainingTime = (elapsedTime >= targetValue * 1000) ? 0 : (targetValue * 1000 - elapsedTime);
    currentValue = remainingTime / 1000;
    handleWebServer(remainingTime);
  }
}
