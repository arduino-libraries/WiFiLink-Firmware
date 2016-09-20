#include "CommLgc.h"
#include "utility/wifi_utils.h"
//#include "CommItf.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

void setup() {

  WiFi.mode(WIFI_STA);
  WiFi.begin("DHLabs", "dhlabsrfid01");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA.begin();
  //delay(300);
  //CommunicationInterface.begin();
  CommunicationLogic.begin();
  //while(!CommunicationInterface.begin());
}

void loop() {
  ArduinoOTA.handle();
//  Serial.print("SSID");
//  Serial.println(WiFi.SSID());
  //Serial.println("Prova");
  CommunicationLogic.handle();
	delay(1000);
  //String a = CommunicationInterface.read();
  //if(a!="")
   // CommunicationInterface.write("prova");
  //Serial.println("Prova");

}
