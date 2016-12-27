#include "CommLgc.h"
#include "utility/wifi_utils.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// char ssid[] = "****";     //  your network SSID (name)
// char pass[] = "****";

void setup() {

  // WiFi.mode(WIFI_STA);
  // WiFi.begin(ssid, pass);
  // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //   delay(5000);
  //   ESP.restart();
  // }
	// delay(8000);
  ArduinoOTA.begin();
  CommunicationLogic.begin();

}

void loop() {
  ArduinoOTA.handle();
  CommunicationLogic.handle();

}
