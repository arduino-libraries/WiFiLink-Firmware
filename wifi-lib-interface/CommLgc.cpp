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

	tMsgPacket pckt1;
	tMsgPacket *reqPckt = &pckt1;

	tMsgPacket pckt2;
	tMsgPacket *resPckt = &pckt2;

	CommunicationInterface.read(reqPckt);

	process(reqPckt, resPckt);

	DEBUG(resPckt);
	// Serial.println(resPckt->params[0].param);
	// Serial.println(resPckt->params[0].paramLen);

	// if(reqPckt.cmd != NULL ){
	// 	tMsgPacket resPckt = process(reqPckt);
	// 	CommunicationInterface.write(resPckt);
	// }
}

void CommLgc::DEBUG(tMsgPacket *_pckt) {

	Serial.println("---- Message Packet ----");
	Serial.print("Start Command: ");
	Serial.println(_pckt->cmd, HEX);
	Serial.print("Command: ");
	Serial.println(_pckt->tcmd, HEX);
	Serial.print("Num Param: ");
	Serial.println(_pckt->nParam);

	for(int i=0; i<_pckt->nParam; i++){
		Serial.print("Param Len: " );
		Serial.println(_pckt->params[i].paramLen);
		Serial.print("Param Val: " );
		Serial.println(_pckt->params[i].param);
	}
}

void CommLgc::getCurrentSSID(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){

	//retrieve SSID of the current network
	String result = WiFi.SSID();

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = result.length();
	//_resPckt->params[0].param = buf1;
	_resPckt->params[0].param = result;

}

void CommLgc::process(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){



	if ((_reqPckt->cmd == START_CMD) && ((_reqPckt->tcmd & REPLY_FLAG) == 0)){

		_resPckt->cmd = START_CMD;
		_resPckt->tcmd = _reqPckt->tcmd | REPLY_FLAG;

		switch(_reqPckt->tcmd){

			case SET_PASSPHRASE_CMD:
			break;

			case SET_IP_CONFIG_CMD:
			break;

			case SET_DNS_CONFIG_CMD:
			break;

			case GET_CONN_STATUS_CMD:
			break;

			case GET_IPADDR_CMD:
			break;

			case GET_MACADDR_CMD:
			break;

			case GET_CURR_SSID_CMD:
				getCurrentSSID(_reqPckt, _resPckt);
			break;

			case GET_CURR_BSSID_CMD:
			break;

			case GET_CURR_RSSI_CMD:
			break;

			case GET_CURR_ENCT_CMD:
			break;

			case SCAN_NETWORKS:
			break;

			case START_SERVER_TCP_CMD:
			break;

			case GET_STATE_TCP_CMD:
			break;

			case DATA_SENT_TCP_CMD:
			break;

			case AVAIL_DATA_TCP_CMD:
			break;

			case GET_DATA_TCP_CMD:
			break;

			case START_CLIENT_TCP_CMD:
			break;

			case STOP_CLIENT_TCP_CMD:
			break;

			case GET_CLIENT_STATE_TCP_CMD:
			break;

			case DISCONNECT_CMD:
			break;

			case GET_IDX_RSSI_CMD:
			break;

			case GET_IDX_ENCT_CMD:
			break;

			case GET_HOST_BY_NAME_CMD:
			break;

			case START_SCAN_NETWORKS:
			break;

			case SEND_DATA_UDP_CMD:
			break;

			case GET_REMOTE_DATA_CMD:
			break;

			case SEND_DATA_TCP_CMD:
			break;

			case GET_DATABUF_TCP_CMD:
			break;

			case INSERT_DATABUF_CMD:
			break;

			default:
				//Serial.println("DEFAULT");
				//_reqPckt->test = "undefined";
			break;
		}
	}
}

CommLgc CommunicationLogic;
