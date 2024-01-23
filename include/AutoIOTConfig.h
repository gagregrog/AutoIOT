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

void resetConfig(const char *path);
bool readConfig(const char *path, DynamicJsonDocument &json);
void writeConfig(const char *path, DynamicJsonDocument &json);

#endif
