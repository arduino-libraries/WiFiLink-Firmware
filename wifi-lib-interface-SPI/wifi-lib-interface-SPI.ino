#include "CommLgc.h"
//#include "utility/wifi_utils.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <FS.h>
#include <Hash.h>
#include <ESP8266WebServer.h>

#define WIFI_LED 2
#define HOSTNAME "Arduino-PrimoSPI"

ESP8266WebServer server(80);    //server UI

void setup() {

  ArduinoOTA.begin();
  CommunicationLogic.begin();
  WiFi.hostname(HOSTNAME);      //set hostname
  MDNS.begin(HOSTNAME);         //set mdns
  initWBServer();               //UI begin

}

void loop() {
  
  ArduinoOTA.handle();
  CommunicationLogic.handle();
  handleWBServer();

}
