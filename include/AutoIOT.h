/*
  AutoIOT.h - Library for initiating a WiFi connection and managing a server.
  Created by Gary Gergory Rogers, December 19, 2023.
  Released into the public domain.
*/

#ifndef AutoIOT_h
#define AutoIOT_h

#include "Arduino.h"
#include "AutoIOTConfig.h"

#ifdef ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#elif defined(ESP8266)
// this #define prevents an incompatibility between ESPAsyncWebServer and WifiManager
#define WEBSERVER_H
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// WiFiManager MUST be included *before* including ESPAsyncWebServer or there will be compile errors
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>

typedef void (*voidCallback)();

#define __CONNECT_TIMEOUT__ 60
#define __AP_TIMEOUT__ 60

extern char hostname[CONFIG_MAX_LENGTH];
extern char otaPassword[CONFIG_MAX_LENGTH];

class AutoIOT
{
public:
  AutoIOT();
  AutoIOT(bool enableOTA);
  AutoIOT(char *defaultHostname, char *defaultPassword);
  AutoIOT(char *defaultHostname, char *defaultPassword, bool enableOTA);

  WiFiManager wifiManager;

  void loop();
  void begin();

  void disableLED();

  void resetAllSettings();
  void resetWiFiCredentials();

  void setOnConnect(voidCallback);
  void setOnDisconnect(voidCallback);
  void setOnEnterConfig(voidCallback);

private:
  void _setup(bool enableOTA);
  void _ledOn();
  void _ledOff();
  void _digitalWrite(int value);

  void (*_onConnect)() = NULL;
  void (*_onDisconnect)() = NULL;

  bool _ledEnabled;
  bool _otaEnabled;
  bool _lastWiFiStatus;

#ifdef ESP32
  uint8_t _LED_ON = HIGH;
  uint8_t _LED_OFF = LOW;
  uint8_t _LED = 2;
#elif defined(ESP8266)
  uint8_t _LED_ON = LOW;
  uint8_t _LED_OFF = HIGH;
  uint8_t _LED = LED_BUILTIN;
#endif
};

#endif
