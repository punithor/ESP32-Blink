#include <WiFi.h>
#include "Network.h"
#include "config.h"
#include "rpc.h"


WiFiClient espClient;
PubSubClient mqtt(espClient);

// ---------------------------------------------------------------------
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " connected." : " FAILED (will retry).");
}

void connectMQTT() {
  if (WiFi.status() != WL_CONNECTED) return;
  Serial.print("Connecting to ThingsBoard MQTT...");
  if (mqtt.connect(BAY_ID, TB_TOKEN, NULL)) {
    Serial.println(" connected.");
    mqtt.subscribe("v1/devices/me/rpc/request/+");
  } else {
    Serial.print(" failed, rc=");
    Serial.println(mqtt.state());
    delay(1000);
  }
}

void maintainMQTT() {
  static unsigned long lastAttempt = 0;
  if (mqtt.connected() || WiFi.status() != WL_CONNECTED) return;
  if (millis() - lastAttempt < 5000) return;

  lastAttempt = millis();
  connectMQTT();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  const char* requestPrefix = "v1/devices/me/rpc/request/";
  String topicName(topic);
  if (!topicName.startsWith(requestPrefix)) return;

  String requestId = topicName.substring(strlen(requestPrefix));
  String requestPayload;
  requestPayload.reserve(length + 1);
  for (unsigned int index = 0; index < length; index++) {
    requestPayload += static_cast<char>(payload[index]);
  }

  handleRpc(requestId, requestPayload.c_str());
}

// ---------------------------------------------------------------------
// Single MQTT callback, dispatched by topic: attribute push/response vs.
// RPC request. This is where Phase 6's two device-side features live.
// ---------------------------------------------------------------------
