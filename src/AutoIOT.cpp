/*
  AutoIOT.cpp - Library for initiating a WiFi connection and managing a server.
  Created by Gary Gregory Rogers, December 19, 2023.
  Released into the public domain.
*/

#include "AutoIOT.h"

bool __shouldSaveConfig__ = false;
void (*__onEnterConfig__)() = NULL;

void __handleOnEnterConfig__(WiFiManager *myWiFiManager)
{
  Serial.println("[INFO] Starting WiFi Configuration Portal.");
  __onEnterConfig__();
}

void saveConfigCallback()
{
  __shouldSaveConfig__ = true;
}

void AutoIOT::_setup(bool enableOTA)
{
  _otaEnabled = enableOTA;
  _ledEnabled = true;

  WiFiManager wifiManager;
}

AutoIOT::AutoIOT()
{
  _setup(false);
}

AutoIOT::AutoIOT(bool enableOTA)
{
  _setup(enableOTA);
}

AutoIOT::AutoIOT(char *defaultHostname, char *defaultPassword)
{
  strcpy(hostname, defaultHostname);
  strcpy(otaPassword, defaultPassword);

  _setup(true);
}

AutoIOT::AutoIOT(char *defaultHostname, char *defaultPassword, bool enableOTA)
{
  strcpy(hostname, defaultHostname);
  strcpy(otaPassword, defaultPassword);

  _setup(enableOTA);
}

void AutoIOT::_digitalWrite(int value)
{
  if (_ledEnabled)
  {
    digitalWrite(_LED, value);
  }
}

void AutoIOT::disableLED()
{
  _ledEnabled = false;
}

void AutoIOT::setOnConnect(void (*onConnect)())
{
  _onConnect = onConnect;
}

void AutoIOT::setOnDisconnect(void (*onDisconnect)())
{
  _onDisconnect = onDisconnect;
}

void AutoIOT::setOnEnterConfig(void (*onEnterConfig)())
{
  __onEnterConfig__ = onEnterConfig;
}

void AutoIOT::begin()
{
  if (_ledEnabled)
  {
    pinMode(_LED, OUTPUT);
  }

  readConfig(); // update hostname and password from stored JSON

  // I've been told this line is a good idea
  WiFi.mode(WIFI_STA);

  if (__onEnterConfig__ != NULL)
  {
    wifiManager.setAPCallback(__handleOnEnterConfig__);
  }

  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConnectTimeout(__CONNECT_TIMEOUT__);
  wifiManager.setTimeout(__AP_TIMEOUT__);
  wifiManager.setCountry("US");

  WiFiManagerParameter customHostname("hostname", "AP/Hostname", hostname, CONFIG_MAX_LENGTH);
  WiFiManagerParameter customPassword("password", "OTA Password", otaPassword, CONFIG_MAX_LENGTH);
  wifiManager.addParameter(&customHostname);
  wifiManager.addParameter(&customPassword);

  _lastWiFiStatus = false;
  if (!wifiManager.autoConnect(hostname, otaPassword))
  {
    // If we've hit the config portal timeout, then retstart
    Serial.println("[ERROR] Failed to connect and hit timeout, restarting after 5 seconds...");
    delay(5000);
    ESP.restart();
    delay(5000);
  }

  // Update parameters from the new values set in the portal
  if (__shouldSaveConfig__)
  {
    writeConfig(customHostname.getValue(), customPassword.getValue());
    // restart here when there is a new configuration
    // to fix WifiManager incompatibility w/ EspAsyncWebServer on ESP32
    Serial.println("Forcing restart to enable server...");
    wifiManager.reboot();
    delay(5000);
  }

  Serial.println("[SUCCESS] Connected to WiFi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

#ifdef ESP32
  WiFi.setHostname(hostname);
#elif defined(ESP8266)
  // esp8266mDNS does not work without ArduinoOTA
  // so this line is likely not doing what we want it to do
  WiFi.hostname(hostname);
#endif
  wifiManager.setHostname(hostname);

  if (!MDNS.begin(hostname))
  {
    Serial.println("[ERROR] Could not start mDNS");
  }
  else
  {
    Serial.print("[INFO] Started mDNS with name: ");
    Serial.println(hostname);
  }

  if (_otaEnabled)
  {
    ArduinoOTA.setHostname(hostname);
    ArduinoOTA.setPassword(otaPassword);
    // esp8266 mDNS does not work without ArduinoOTA
    ArduinoOTA.begin();
    Serial.println("[INFO] ArduinoOTA enabled!");
  }
  else
  {
    Serial.println("[WARN] ArduinoOTA disabled.");
  }

  _ledOff();
}

void AutoIOT::loop()
{
  bool isConnected = WiFi.status() == WL_CONNECTED;

  if (isConnected != _lastWiFiStatus)
  {
    Serial.println("[INFO] WiFi Connectivity change");

    _lastWiFiStatus = isConnected;
    if (isConnected)
    {
      Serial.println("[INFO] Device is now connected to WiFi");
      if (_onConnect != NULL)
      {
        _onConnect();
      }
    }
    else
    {
      Serial.println("[WARNING] Device has lost it's connection to WiFi");
      if (_onDisconnect != NULL)
      {
        _onDisconnect();
      }
    }
  }

  _ledOff();
  if (_otaEnabled)
  {
    ArduinoOTA.handle();
  }
}

void AutoIOT::_ledOn()
{
  _digitalWrite(_LED_ON);
}

void AutoIOT::_ledOff()
{
  _digitalWrite(_LED_OFF);
}

// reset wifi creds and maybe reboot
void AutoIOT::resetWiFiCredentials()
{
  Serial.println("[WARNING] Resetting WiFi credentials!");
  wifiManager.resetSettings();
  delay(500);
  ESP.restart();
}

// reset wifi creds and reset ap/pw (don't reboot)
void AutoIOT::resetAllSettings()
{
  resetConfig();
  resetWiFiCredentials();
}
