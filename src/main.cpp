#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>

// Replace with your network credentials
const char *ssid = "***";
const char *password = "***";
const int dryValue = 650;
const int wetValue = 280;
const int wateringInterval = 5 * 24 * 60 * 60 * 1000;
const int wateringPeriod = 8 * 1000;
const int waterPumpPin = D1;
unsigned long previousWatering = 0;

AsyncWebServer server(80);

String outputState()
{
  if (digitalRead(waterPumpPin))
    return "checked";
  return "";
}

String processor(const String &var)
{
  if (var == "BUTTONPLACEHOLDER")
    return "<p><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputState() + "><span class=\"slider\"></span></label></p>";
  else if (var == "TIMERVALUE")
    return String(wateringInterval - (millis() - previousWatering));
  else if (var == "HUMIDITY")
    return String(map(analogRead(A0), wetValue, dryValue, 100, 0));
  return String();
}

void setup()
{
  Serial.begin(115200);
  if (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  pinMode(A0, INPUT);
  pinMode(waterPumpPin, OUTPUT);
  digitalWrite(waterPumpPin, LOW);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  ArduinoOTA.onStart([](){ Serial.println("Start"); });
  ArduinoOTA.onEnd([](){ Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
  ArduinoOTA.begin();
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/home.html", "text/html", false, processor); });
  server.on("/waterpump", HTTP_GET, [](AsyncWebServerRequest *request) {
    String inputMessage;
    if (request->hasParam("state")) 
      digitalWrite(waterPumpPin, request->getParam("state")->value().toInt());
    else 
      inputMessage = "No message sent";
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK"); });
  server.begin();
}

void turnOffMotor()
{
  previousWatering = millis();
  digitalWrite(waterPumpPin, LOW);
}
void loop()
{
  ArduinoOTA.handle();
  if (digitalRead(waterPumpPin) == LOW && millis() - previousWatering > wateringInterval)
    digitalWrite(waterPumpPin, HIGH);
  else if (digitalRead(waterPumpPin) == HIGH && millis() - previousWatering > wateringPeriod)
    turnOffMotor();
  delay(1000);
}
