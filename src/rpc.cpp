#include <ArduinoJson.h>
#include "rpc.h"
#include "Network.h"
#include "State.h"
#include "config.h"

// ---------------------------------------------------------------------
// SRS 8.5: RPC command handling.
// ---------------------------------------------------------------------
void handleRpc(String requestId, const char* payload) {
  JsonDocument doc;
  if (deserializeJson(doc, payload)) return;

  String method = doc["method"] | "";
  JsonObject params = doc["params"];

  JsonDocument response;

  if (method == "setRelayState") {
    bool state = params["state"] | false;
    
    if(state)
    {
      digitalWrite(RELAY_PIN, HIGH);
    }
    else
    {
      digitalWrite(RELAY_PIN, LOW);
    }
    throttleLevel = state ? 100 : 0;
    localDecision = state ? "ALLOW" : "MANUAL_OFF";
    response["success"] = true;
    response["throttleLevel"] = throttleLevel;
    Serial.print(">> RPC setRelayState("); Serial.print(state); Serial.println(") - manual override engaged.");

  } else if (method == "setThrottle") {
    int level = params["level"] | 100;
    level = constrain(level, 0, 100);
    throttleLevel = level;
    localDecision = "MANUAL_THROTTLE";
    response["success"] = true;
    response["throttleLevel"] = throttleLevel;
    Serial.print(">> RPC setThrottle("); Serial.print(level); Serial.println(") - manual override engaged.");

  }else {
    response["success"] = false;
    response["error"] = "unknown method";
  }

  char buffer[220];
  serializeJson(response, buffer);
  mqtt.publish(("v1/devices/me/rpc/response/" + requestId).c_str(), buffer);
}
