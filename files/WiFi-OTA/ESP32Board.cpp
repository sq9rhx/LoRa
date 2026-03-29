#ifdef ESP_PLATFORM

#include "ESP32Board.h"

#if defined(ADMIN_PASSWORD) && !defined(DISABLE_WIFI_OTA)   // Repeater or Room Server only
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#include <SPIFFS.h>

bool ESP32Board::startOTAUpdate(const char* id, char reply[]) {
  inhibit_sleep = true;   // prevent sleep during OTA

  bool usingAP = true;

  // Optional STA mode if set via build flags in variants (preferred)
#if defined(OTA_WIFI_SSID)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  WiFi.begin(OTA_WIFI_SSID, OTA_WIFI_PASS);
  unsigned long started = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - started < 10000) {
    delay(250);
  }

  if (WiFi.status() == WL_CONNECTED) {
    usingAP = false;
    sprintf(reply, "Started OTA on STA %s IP %s", OTA_WIFI_SSID, WiFi.localIP().toString().c_str());
    MESH_DEBUG_PRINTLN("startOTAUpdate STA: %s", reply);
  } else {
    MESH_DEBUG_PRINTLN("startOTAUpdate STA failed, fallback to AP");
  }
#endif

  if (usingAP) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MeshCore-OTA", NULL);

    sprintf(reply, "Started: http://%s/update", WiFi.softAPIP().toString().c_str());
    MESH_DEBUG_PRINTLN("startOTAUpdate AP: %s", reply);
  }

  static char id_buf[60];
  sprintf(id_buf, "%s (%s)", id, getManufacturerName());
  static char home_buf[90];
  sprintf(home_buf, "<H2>Hi! I am a MeshCore Repeater. ID: %s</H2>", id);

  AsyncWebServer* server = new AsyncWebServer(80);

  server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", home_buf);
  });
  server->on("/log", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/packet_log", "text/plain");
  });

  AsyncElegantOTA.setID(id_buf);
  AsyncElegantOTA.begin(server);    // Start ElegantOTA
  server->begin();

  return true;
}

#else
bool ESP32Board::startOTAUpdate(const char* id, char reply[]) {
  return false; // not supported
}
#endif

#endif
