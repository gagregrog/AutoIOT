#ifndef AutoIOTConfig_h
#define AutoIOTConfig_h

#include <FS.h>
#include <ArduinoJson.h>

#ifdef ESP32
#include <SPIFFS.h>
#define FileSystem SPIFFS
#elif defined(ESP8266)
#include <LittleFS.h>
#define FileSystem LittleFS
#endif

#define CONFIG_MAX_LENGTH 40
#define CONFIG_PATH "/config.json"

void resetConfig();
bool readConfig();
void writeConfig(const char *accessPoint, const char *password);
bool initFileSystem();

#endif
