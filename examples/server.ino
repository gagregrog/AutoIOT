/*
  AutoIOT - Library for initiating a WiFi connection and managing a server.
  Created by Gary Gregory Rogers, December 18, 2023.
  Released into the public domain.
*/

#include <AutoIOT.h>

AutoIOT app;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool enabled = true;
const char appConfigPath[] = "/app.json";
DynamicJsonDocument appConfig(256);

void initConfig()
{
  if (!readConfig(appConfigPath, appConfig))
  {
    // config was not loaded, so init with defaults
    appConfig["enabled"] = enabled;
    writeConfig(appConfigPath, appConfig);
  }
  else
  {
    enabled = appConfig["enabled"].as<bool>();
  }
  Serial.print(F("App is: "));
  Serial.println(enabled ? "enabled" : "disabled");
}

void initRouteHandlers()
{
  server.on("/hostname", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", app.wifiManager.getWiFiHostname()); });

  server.on("/ip", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", WiFi.localIP().toString()); });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              request->send(200, "text/plain", "Success");
              delay(1000);
              app.resetAllSettings(); });
}

void handleToggle()
{
  enabled = !enabled;
  ws.textAll(enabled ? "enabled" : "disabled");
  appConfig["enabled"] = enabled;
  writeConfig(appConfigPath, appConfig);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    data[len] = 0;
    if (strcmp((char *)data, "toggle") == 0)
    {
      handleToggle();
    }
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

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup()
{
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  delay(1000);

  app.begin();
  initConfig();
  initRouteHandlers();
  Serial.println("Starting server!");
  initWebSocket();
  server.begin();
}

void loop()
{
  // since the server is async, this is just for OTA updates
  app.loop();
}
