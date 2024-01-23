/*
  AutoIOT - Library for initiating a WiFi connection and managing a server.
  Created by Gary Gregory Rogers, December 18, 2023.
  Released into the public domain.
*/

#include <AutoIOT.h>

// Instantiate the app

// default AP/Hostname: "autoIOT"
// default PW: "newcouch"
AutoIOT app;

// explicitly enable/disable ArduinoOTA (true by default)
// mDNS does not work on ESP8266 without ArduinoOTA
// AutoIOT app(false);

// Specify access point/hostname and password
// max of 40 chars for each
// AutoIOT app("my_ap", "my_pw"); // OTA enabled by default if pw provided

// AutoIOT app("my_ap", "my_pw", false); // do it all (set ap/pw and disable OTA)

// You can access the WifiManager directly via app.wifiManager

void handleConnect()
{
  Serial.println("The board has connected to WiFi!");
}

void handleDisconnect()
{
  Serial.println("The board has DISCONNECTED from WiFi!");
}

void handleConfig()
{
  Serial.println("The configuration portal has been started!");
}

void setup()
{
  // WiFi information is printed, so it's a good idea to start the Serial monitor
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  delay(1000);

  // if you need to fetch the current hostname
  // String hostname = app.wifiManager.getHostname();

  // to change the WiFi and erase the config (with custom hostname/pw)
  // this will also reboot the board
  // app.resetAllSettings();

  // disable LED indicator if desired (call before app.begin();)
  // app.disableLED();

  // set callbacks to handle various lifecycle events

  // called once each time the board (re-)establishes a WiFi connection
  app.setOnConnect(handleConnect);

  // called once each time the board loses WiFi connection
  app.setOnDisconnect(handleDisconnect);

  // called once each time the board starts the Config Portal
  app.setOnEnterConfig(handleConfig);

  app.begin();

  // do any additional server / ws setup here

  // you can use the AutoIOTConfig functions to simplify reading/writing json documents
  DynamicJsonDocument appConfig(256);
  const char appConfigPath[] = "/app.json";

  if (!readConfig(appConfigPath, appConfig))
  {
    // config was not loaded, so init with defaults
    appConfig["enabled"] = true;
    writeConfig(appConfigPath, appConfig);
  }
  else
  {
    bool enabled = appConfig["enabled"].as<bool>();
    Serial.print(F("App is "));
    Serial.println(enabled ? "enabled!" : "disabled");
  }
}

void loop()
{
  // You must call app.loop(); if you are using ArduinoOTA
  // if you do not use OTA updates then you can skip this
  app.loop();
}
