#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "WebServerHandler.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "de.pool.ntp.org");

AsyncWebServer server(80);

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

    .timer-container {
      margin: 20px auto;
      width: 300px;
      background-color: #ffffff;
      border-radius: 10px;
      padding: 20px;
      box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);
    }

    .timer-value {
      font-size: 36px;
      font-weight: bold;
      margin-bottom: 20px;
      color: #333333;
    }

    .timer-label {
      font-size: 18px;
      color: #666666;
    }
  </style>
</head>
<body>
  <h1>Online abrufbarer Timer</h1>

  <div class="timer-container">
    <p class="timer-value" id="currentMinutes"></p>
    <p class="timer-label">Minuten</p>
    <p class="timer-value" id="currentSeconds"></p>
    <p class="timer-label">Sekunden</p>
  </div>

  <script>
    setInterval(getRemainingTime, 1000);

    function getRemainingTime() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var response = this.responseText.split(":");
          document.getElementById("currentMinutes").innerHTML = response[0];
          document.getElementById("currentSeconds").innerHTML = response[1];
        }
      };
      xhttp.open("POST", "/zeit", true);
      xhttp.send();
    }
  </script>
</body>
</html>
)rawliteral";

    request->send(200, "text/html", html);
  });

  server.on("/zeit", HTTP_POST, [](AsyncWebServerRequest* request){
    
    unsigned long minutes = 10;
    unsigned long seconds = 50;
    String response = String(minutes) + ":" + String(seconds);
    request->send(200, "text/plain", response);
  });

  server.begin();
}

void handleWebServer(unsigned long remainingTime) {
  timeClient.update();
  // Keine Neopixel-Animation oder andere Funktionen erforderlich
}
