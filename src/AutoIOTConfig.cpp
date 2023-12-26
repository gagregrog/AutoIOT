#include "AutoIOTConfig.h"

char hostname[CONFIG_MAX_LENGTH] = "autoIOT";
char otaPassword[CONFIG_MAX_LENGTH] = "newcouch";

bool initFileSystem()
{
  Serial.println(F("[INFO] Mounting File System..."));
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

bool readConfig()
{
  if (!initFileSystem())
  {
    return false;
  }
  Serial.println(F("[SUCCESS] File System Mounted"));

  if (!FileSystem.exists(CONFIG_PATH))
  {
    Serial.println(F("[WARNING] Config not found"));
    return false;
  }

  Serial.println(F("[INFO] Reading existing config..."));
  File configFile = FileSystem.open(CONFIG_PATH, "r");

  if (!configFile)
  {
    Serial.println(F("[ERROR] Failed to open config file for reading"));
    return false;
  }

  Serial.println(F("[SUCCESS] Existing config loaded"));

  // maybe change this bit
  size_t size = configFile.size();
  // Allocate a buffer to store the file contents
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
  StaticJsonDocument<256> json;
  DeserializationError jsonError = deserializeJson(json, buf.get());

  if (jsonError)
  {
    Serial.print(F("[ERROR] Failed to parse JSON config: "));
    Serial.println(jsonError.c_str());
    return false;
  }

  serializeJsonPretty(json, Serial);
  Serial.println();

  if (json.containsKey("hostname"))
  {
    strcpy(hostname, json["hostname"]);
    Serial.print("[INFO] Setting hostname/access point to: ");
    Serial.println(hostname);
  }
  else
  {
    Serial.print("[WARNING] hostname not in config");
  }

  if (json.containsKey("password"))
  {
    strcpy(otaPassword, json["password"]);
    Serial.print("[INFO] Setting password to: ");
    Serial.println(otaPassword);
  }
  else
  {
    Serial.print("[WARNING] password not in config");
  }

  Serial.println();

  return true;
}

void writeConfig(const char *updatedHostname, const char *updatedPassword)
{
  if ((strcmp(updatedHostname, hostname) + strcmp(updatedPassword, otaPassword)) == 0)
  {
    Serial.println(F("[INFO] Config is unchanged -- no need to write"));
    return;
  }

  StaticJsonDocument<256> json;

  json["hostname"] = updatedHostname;
  strcpy(hostname, updatedHostname);

  json["password"] = updatedPassword;
  strcpy(otaPassword, updatedPassword);

  Serial.println(F("[INFO] Saving config..."));

  File configFile = FileSystem.open(CONFIG_PATH, "w");
  if (!configFile)
  {
    Serial.println(F("[ERROR] Failed to open config file for writing"));
    return;
  }

  serializeJsonPretty(json, Serial);
  Serial.println();

  if (serializeJson(json, configFile) == 0)
  {
    Serial.println(F("[ERROR] Failed to write file"));
  }

  configFile.close();
}

void resetConfig()
{
  DynamicJsonDocument emptyDoc(0);
  emptyDoc.to<JsonObject>();

  if (!initFileSystem())
  {
    return;
  }

  Serial.println("[WARNING] Resetting config...");
  File configFile = FileSystem.open(CONFIG_PATH, "w");

  if (!configFile)
  {
    Serial.println(F("[ERROR] Failed to open config file for writing"));
    return;
  }

  serializeJson(emptyDoc, configFile);
  configFile.close();
}
