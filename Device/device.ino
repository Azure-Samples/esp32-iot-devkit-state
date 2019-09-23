/**
 * A simple Azure IoT example for sending telemetry to Iot Hub.
 */

#include <WiFi.h>
#include "Esp32MQTTClient.h"
#include <Preferences.h>

Preferences preferences;


#define INTERVAL 10000
#define DEVICE_ID "Esp32Device"
#define MESSAGE_MAX_LEN 512
// Please input the SSID and password of WiFi
const char* ssid     = "";
const char* password = "";

/*String containing Hostname, Device Id & Device Key in the format:                         */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"                */
/*  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessSignature=<device_sas_token>"    */
static const char* connectionString = "";
const char *messageData = "%d: %s (%d)\\n  ";
static bool hasIoTHub = false;
static bool hasWifi = false;
static bool reportSending = true;
static bool shouldRestarting = false;
static uint64_t send_interval_ms;
unsigned int if_restarted;

//used to set the variable after function returns
struct controlRestarting{
  ~controlRestarting() {
    shouldRestarting = true;
  }
};

static int  DeviceMethodCallback(const char *methodName, const unsigned char *payload, int size, unsigned char **response, int *response_size)
{
  controlRestarting temp;
  LogInfo("Try to invoke method %s", methodName);
  const char *responseMessage = "\"Successfully invoke device method\"";
  int result = 200;
  if (strcmp(methodName, "restart") == 0)
  { 
    LogInfo("Start restarting device");
  }
  else
  {
    LogInfo("No method %s found", methodName);
    responseMessage = "\"No method found\"";
    result = 404;
  }
  *response_size = strlen(responseMessage) + 1;
  *response = (unsigned char *)strdup(responseMessage);
  return result;
}


void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  Serial.println("Starting connecting WiFi.");
  delay(10);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    hasWifi = false;
  }
  hasWifi = true;
  
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  if (!Esp32MQTTClient_Init((const uint8_t*)connectionString, true))
  {
    hasIoTHub = false;
    Serial.println("Initializing IoT hub failed.");
    return;
  }
  hasIoTHub = true;
  preferences.begin("restart", false);
  if_restarted = preferences.getUInt("if_restarted", 0);
  if (if_restarted == 1){
    preferences.putUInt("if_restarted", 0);
    Esp32MQTTClient_ReportState("{\"restarted\": true }");
  }
  Esp32MQTTClient_SetDeviceMethodCallback(DeviceMethodCallback);
  send_interval_ms = millis();
}



void loop() {
  if (shouldRestarting){
    Esp32MQTTClient_ReportState("{\"restarted\": false }");
    preferences.putUInt("if_restarted", 1);
    ESP.restart();
  }
  if (hasWifi && hasIoTHub){
    if (reportSending && 
        (int)(millis() - send_interval_ms) >= INTERVAL)
    {
      Serial.begin(115200);

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
      delay(100);

      Serial.println("Setup done");
      Serial.println("Scan start");
      char Twinmessage[MESSAGE_MAX_LEN];
      char Wifi_str[MESSAGE_MAX_LEN];
        // WiFi.scanNetworks will return the number of networks found
      int n = WiFi.scanNetworks();
      int count = 0;
      Serial.println("Scan done");
      if (n == 0) {
        snprintf(Twinmessage, MESSAGE_MAX_LEN, "No wifi found");
      } else {
        if (n > 10){
          n = 10;
        }
        for (int i = 0; i < n; ++i) {
          count = count + snprintf(Twinmessage + count * sizeof(char), MESSAGE_MAX_LEN, messageData, i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i)); 
          delay(10);
          }
        }
        Serial.println(Twinmessage);

        // Send wifi information to iothub
      snprintf(Wifi_str, MESSAGE_MAX_LEN, "{\"Wifi\": \"%s\"}", Twinmessage);
      Esp32MQTTClient_ReportState(Wifi_str);
      
        // Wait a bit before scanning again
      send_interval_ms = millis();
    }
    else{
      Esp32MQTTClient_Check();
    }
  }
  delay(10);
}