#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#else
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

// Replace with your network credentials
#define ssid "Blue"
#define password "ZJcCgrNRdyqXy9ab"
#define dryValue  650
#define wetValue  280
#define wateringInterval  5 * 24 * 60 * 60 * 1000
#define wateringPeriod 8 * 1000
#define waterPumpPin D1
unsigned long previousWatering = 0;
bool waterPumpStatus = 0;

AsyncWebServer server(80);
AsyncWebSocket webSocket("/ws");

String jsonString;

void notifyClients()
{
  StaticJsonDocument<100> doc;
  JsonObject object = doc.to<JsonObject>();
  Serial.println(waterPumpStatus);
  object["pumpState"] = String(waterPumpStatus);
  object["humidity"] = map(analogRead(A0), wetValue, dryValue, 100, 0);
  object["timerValue"] = (wateringInterval - (millis() - previousWatering)) / 1000;
  serializeJson(doc, jsonString); // serialize the object and save teh result to teh string variable.
  Serial.println(jsonString);     // print the string for debugging.
  webSocket.textAll(jsonString);         // send the JSON object through the websocket
  jsonString = "";                // clear the String.
}

void turnOnPump()
{
  Serial.println("Turn On");
  waterPumpStatus = true;
  digitalWrite(waterPumpPin, HIGH);
  notifyClients();
}

void turnOffPump()
{
  Serial.println("Turn Off");
  waterPumpStatus = false;
  previousWatering = millis();
  digitalWrite(waterPumpPin, LOW);
  notifyClients();
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if (strcmp((char *)data, "Turn On") == 0)
      turnOnPump();
    else if (strcmp((char *)data, "Turn Off") == 0)
      turnOffPump();
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initilizeWebSocket()
{
  webSocket.onEvent(onEvent);
  server.addHandler(&webSocket);
}

String processor(const String &var)
{
  if (var == "PUMPSTATE")
    if (waterPumpStatus)
      return "ON";
    else
      return "OFF";
  if (var == "HUMIDITY")
    return String(map(analogRead(A0), wetValue, dryValue, 100, 0));
  if (var == "TIMER")
    return String((wateringInterval - (millis() - previousWatering)) / 1000);
  return String();
}

void initilizeSPIFFS()
{
  while (!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    delay(1000);
  }
  Serial.println("SPIFFS initilized!");
}

void initilizeWifi()
{
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
}

void initilizeOTA()
{
  ArduinoOTA.onStart([]()
                     { Serial.println("Start"); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
  ArduinoOTA.begin();
}

void initilizeServer()
{
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/home.html", "text/html", false, processor); });
  // Start server
  server.begin();
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println("serial is open");
  pinMode(A0, INPUT);
  pinMode(waterPumpPin, OUTPUT);
  digitalWrite(waterPumpPin, LOW);
  initilizeSPIFFS();
  initilizeWifi();
  initilizeOTA();
  initilizeWebSocket();
  initilizeServer();
}

void loop()
{
  webSocket.cleanupClients();
  if (!waterPumpStatus && millis() - previousWatering > wateringInterval)
   turnOnPump();
  else if (waterPumpStatus && millis() - previousWatering > wateringPeriod)
    turnOffPump();
  delay(1000);
}
