#include "AutoIOTConfig.h"

bool initFileSystem()
{
#ifdef ESP32
  if (!(SPIFFS.begin(false) || SPIFFS.begin(true)))
#elif defined(ESP8266)
  if (!LittleFS.begin())
#endif
  {
    Serial.println(F("[ERROR] Failed to mount File System"));
    return false;
  }

  return true;
}

bool readConfig(const char *path, DynamicJsonDocument &json)
{
  if (!initFileSystem())
  {
    return false;
  }
  Serial.println(F("[SUCCESS] File System Mounted"));

  if (!FileSystem.exists(path))
  {
    Serial.print(F("[WARNING] Config not found at "));
    Serial.println(path);
    return false;
  }

  Serial.print(F("[INFO] Reading existing config at "));
  Serial.print(path);
  Serial.println(F("..."));
  File configFile = FileSystem.open(path, "r");

  if (!configFile)
  {
    Serial.println(F("[ERROR] Failed to open config file for reading"));
    return false;
  }

  Serial.println(F("[SUCCESS] Existing config loaded"));

  DeserializationError jsonError = deserializeJson(json, configFile);

  if (jsonError)
  {
    Serial.print(F("[ERROR] Failed to parse JSON config: "));
    Serial.println(jsonError.c_str());
    return false;
  }

  serializeJsonPretty(json, Serial);
  Serial.println();

  return true;
}

bool _writeJson(const char *path, DynamicJsonDocument &json)
{
  if (!initFileSystem())
  {
    return false;
  }

  File configFile = FileSystem.open(path, "w");
  if (!configFile)
  {
    Serial.println(F("[ERROR] Failed to open config file for writing"));
    return false;
  }

  bool isSuccess = true;
  if (serializeJson(json, configFile) == 0)
  {
    Serial.println(F("[ERROR] Failed to write file"));
    isSuccess = false;
  }

  configFile.close();

  return isSuccess;
}

void writeConfig(const char *path, DynamicJsonDocument &json)
{
  Serial.print(F("[INFO] Saving config to "));
  Serial.print(path);
  Serial.println(F("..."));

  if (_writeJson(path, json))
  {
    serializeJsonPretty(json, Serial);
    Serial.println();
  }
}

void resetConfig(const char *path)
{
  DynamicJsonDocument emptyDoc(0);
  emptyDoc.to<JsonObject>();

  Serial.print(F("[WARNING] Resetting config at "));
  Serial.println(path);

  _writeJson(path, emptyDoc);
}
