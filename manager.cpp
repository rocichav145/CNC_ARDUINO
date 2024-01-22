#include <Arduino.h>
#include <WiFiManager.h>

void setup() {
  // put your setup code here, to run once:
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  WiFiManager wiFiManager;

  wiFiManager.resetSettings();

  bool res;

  res = wiFiManager.autoConnect("AutoconnectESP32S","1111");

  if(!res){
    Serial.println("Connected :");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("Connected :");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
