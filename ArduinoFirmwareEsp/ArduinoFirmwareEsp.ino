#include "CommLgc.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <FS.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <ESP8266WebServer.h>


ESP8266WebServer server(80);    //server UI
bool SERVER_STOP = false;       //check stop server

void setup() {

  ArduinoOTA.begin();
  CommunicationLogic.begin();
  initWBServer();               //UI begin
  initMDNS();                   //set MDNS

}

void loop() {

  ArduinoOTA.handle();
  CommunicationLogic.handle();
  if(CommunicationLogic.UI_alert){			//stop UI SERVER
    if(!SERVER_STOP){
      server.stop();
      SERVER_STOP = true;
    }
  }
  else
    handleWBServer();

}
