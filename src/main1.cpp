// #include <Arduino.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266mDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>

// const char *ssid = "10010";
// const char *password = "Polis=110";

// const int dry = 650; // value for dry sensor
// const int wet = 280; // value for wet sensor
// const int delayPeriod = 5 * 24 * 60 * 60 * 1000;
// const int delayPeriod = 30 * 1000;

// const int wateringPeriod = 8 * 1000;
// const int relayPin = D1;

// unsigned long previousMillis = 0;
// unsigned long currentMillis = millis();
// int relayState = LOW;

// WiFiServer server(80);
// String header;
// unsigned long previousTime = 0;
// const long timeoutTime = 2000;

// void setup()
// {
//   Serial.begin(115200);
//   Serial.println("Booting");
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(ssid, password);
//   while (WiFi.waitForConnectResult() != WL_CONNECTED)
//   {
//     Serial.println("Connection Failed! Rebooting...");
//     delay(5000);
//     ESP.restart();
//   }
//   ArduinoOTA.onStart([]()
//                      { Serial.println("Start"); });
//   ArduinoOTA.onEnd([]()
//                    { Serial.println("\nEnd"); });
//   ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
//                         { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
//   ArduinoOTA.onError([](ota_error_t error)
//                      {
//     Serial.printf("Error[%u]: ", error);
//     if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
//     else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
//     else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
//     else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
//     else if (error == OTA_END_ERROR) Serial.println("End Failed"); });
//   ArduinoOTA.begin();
//   Serial.print("IP address: ");
//   Serial.println(WiFi.localIP());
//   server.begin();
//   pinMode(A0, INPUT);
//   pinMode(relayPin, OUTPUT);
// }
// void updateMotorPin()
// {
//   previousMillis = currentMillis;
//   digitalWrite(relayPin, relayState);
// }
// void turnOnMotor()
// {
//   relayState = HIGH;
//   updateMotorPin();
//   Serial.println("motor on");
// }

// void turnOffMotor()
// {
//   relayState = LOW;
//   updateMotorPin();
//   Serial.println("motor off");
// }

// void loop()
// {
//   ArduinoOTA.handle();
//   int sensorVal = analogRead(A0);
//   Serial.println(sensorVal);
//   int percentageHumidity = map(sensorVal, wet, dry, 100, 0);
//   currentMillis = millis();
//   if (relayState == LOW && currentMillis - previousMillis > delayPeriod)
//     turnOnMotor();
//   else if (relayState == HIGH && currentMillis - previousMillis > wateringPeriod)
//     turnOffMotor();
//   WiFiClient client = server.accept(); // Listen for incoming clients

//   if (client)
//   {                                // If a new client connects,
//     Serial.println("New Client."); // print a message out in the serial port
//     String currentLine = "";       // make a String to hold incoming data from the client
//     currentMillis = millis();
//     previousTime = currentMillis;
//     while (client.connected() && currentMillis - previousTime <= timeoutTime)
//     {
//       currentMillis = millis();
//       if (client.available())
//       {
//         char c = client.read();
//         Serial.write(c);
//         header += c;
//         if (c == '\n')
//         { // if the byte is a newline character
//           if the current line is blank, you got two newline characters in a row.
//           that's the end of the client HTTP request, so send a response:
//           if (currentLine.length() == 0)
//           {
//             HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
//             and a content-type so the client knows what's coming, then a blank line:
//             client.println("HTTP/1.1 200 OK");
//             client.println("Content-type:text/html");
//             client.println("Connection: close");
//             client.println();

//             if (relayState == LOW && header.indexOf("GET /motor/on") >= 0)
//               turnOnMotor();
//             else if (relayState == HIGH && header.indexOf("GET /motor/off") >= 0)
//               turnOffMotor();
//             Display the HTML web page
//             client.println("<!DOCTYPE html><html>");
//             client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
//             client.println("<link rel=\"icon\" href=\"data:,\">");
//             CSS to style the on/off buttons
//             Feel free to change the background-color and font-size attributes to fit your preferences
//             client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
//             client.println(".button { background-color: #ffffff; border: none; color: blue; padding: 16px 40px;");
//             client.println("text-decoration: none; font-size: 50px; margin: 2px; cursor: pointer;}");
//             client.println(".button2 {background-color: #77878A;}</style></head>");

//             Web Page Heading
//             client.println("<body><h1>ESP8266 Web Server</h1>");

//             client.println("<p>motor state " + String(relayState) + "</p>");
//             client.println("<p>Current time " + String(currentMillis) + "</p>");
//             client.println("<p>Time left " + String(delayPeriod - (currentMillis - previousMillis)) + "</p>");
//             Serial.println(String(delayPeriod - (currentMillis - previousMillis)));
//             client.println("<p>Current humidity " + String(percentageHumidity) + "%</p>");

//             if (relayState)
//               client.println("<p><a href=\"/motor/off\"><button class=\"button button2\">OFF</button></a></p>");
//             else
//               client.println("<p><a href=\"/motor/on\" ><button class=\"button\">ON</button></a></p>");
//             client.println("</body></html>");

//             The HTTP response ends with another blank line
//             client.println();
//             Break out of the while loop
//             break;
//           }
//           else
//           { // if you got a newline, then clear currentLine
//             currentLine = "";
//           }
//         }
//         else if (c != '\r')
//         {                   // if you got anything else but a carriage return character,
//           currentLine += c; // add it to the end of the currentLine
//         }
//       }
//     }
//     Clear the header variable
//     header = "";
//     Close the connection
//     client.stop();
//     Serial.println("Client disconnected.");
//     Serial.println("");
//   }
//   delay(1000);
// }
