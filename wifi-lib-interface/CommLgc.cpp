//Communication Interface
#include "CommLgc.h"
#include "CommCmd.h"
#include <ESP8266WiFi.h>

String recCmd = "";

CommLgc::CommLgc()
{
	//while(!CommunicationInterface.begin());
}

void CommLgc::begin()
{
   while(!CommunicationInterface.begin());
}

void CommLgc::handle()
{
	recCmd = CommunicationInterface.read();
	if(recCmd != "" ){
		String resp = process(recCmd);
		CommunicationInterface.write(resp);
	}
}

String CommLgc::process(String cmd){
	//TODO
  if(cmd==String(SCAN_NETWORKS,HEX))
	  return String(WiFi.scanNetworks());
  else if(cmd == String(GET_CURR_SSID_CMD,HEX))
   return String(WiFi.SSID());
	else
	  return "received: " + cmd;
}

String getCurrentSSID(String cmd){
  //elabora il comando

  //esegue il comando
  WiFi.SSID();

  //crea la risposta

  //ritorna la risposta
  
}

CommLgc CommunicationLogic;
