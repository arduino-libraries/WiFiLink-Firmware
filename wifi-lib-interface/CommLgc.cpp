//Communication Interface
#include "CommLgc.h"
#include "CommCmd.h"
#include <ESP8266WiFi.h>
#include "utility/wifi_utils.h"

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

	tMsgPacket reqPckt;
	tMsgPacket *refPckt = &reqPckt;
	CommunicationInterface.read(refPckt);
	Serial.println(reqPckt.cmd);

	// if(reqPckt.cmd != NULL ){
	// 	tMsgPacket resPckt = process(reqPckt);
	// 	CommunicationInterface.write(resPckt);
	// }
}

void CommLgc::process(tMsgPacket *pckt){
	//TODO
  // if(pckt.cmd ==String(SCAN_NETWORKS,HEX))
	//   return String(WiFi.scanNetworks());
  // else if(pckt.cmd == String(GET_CURR_SSID_CMD,HEX))
  //  return String(WiFi.SSID());
	// else
	//   return "received: " + pckt.cmd;

}

String getCurrentSSID(String cmd){
  //elabora il comando

  //esegue il comando
  WiFi.SSID();

  //crea la risposta

  //ritorna la risposta

}

CommLgc CommunicationLogic;
