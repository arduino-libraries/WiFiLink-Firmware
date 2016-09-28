//Communication Interface
#include "CommLgc.h"
#include "CommCmd.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiScan.h>
#include "utility/wifi_utils.h"

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

	if( CommunicationInterface.read(reqPckt) == 0){

		DEBUG(reqPckt);

		process(reqPckt, resPckt);

		DEBUG(resPckt);
		CommunicationInterface.write(resPckt);
	}
}

void CommLgc::DEBUG(tMsgPacket *_pckt) {

	Serial1.println("--- Packet  ---");
	Serial1.println(_pckt->cmd, HEX);
	Serial1.println(_pckt->tcmd, HEX);
	Serial1.println(_pckt->nParam, HEX);
	for(int i=0; i<(int)_pckt->nParam; i++){
		Serial1.println(_pckt->params[i].paramLen, HEX );
		for(int j=0; j< (int)_pckt->params[i].paramLen; j++)
			Serial1.println( _pckt->params[i].param[j], HEX);
	}
	Serial1.println(0xEE, HEX);
	Serial1.println("--- End Packet ---");
}

void CommLgc::process(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){

	if (	(_reqPckt->cmd == START_CMD) &&
				((_reqPckt->tcmd & REPLY_FLAG) == 0) ){

		_resPckt->cmd = START_CMD;
		_resPckt->tcmd = _reqPckt->tcmd | REPLY_FLAG;

		switch(_reqPckt->tcmd){

			case SET_PASSPHRASE_CMD:				break;
			case SET_IP_CONFIG_CMD:					break;
			case SET_DNS_CONFIG_CMD:				break;
			case GET_CONN_STATUS_CMD:				break;
			case GET_IPADDR_CMD:						break;
			case GET_MACADDR_CMD:		getMacAddress(_reqPckt, _resPckt);	break;
			case GET_CURR_SSID_CMD: getCurrentSSID(_reqPckt, _resPckt);	break;
			case GET_CURR_BSSID_CMD:				break;
			case GET_CURR_RSSI_CMD: getRSSI(_reqPckt, _resPckt, 1);	break;
			case GET_CURR_ENCT_CMD:	getEncryption(_reqPckt, _resPckt, 1);				break;
			case SCAN_NETWORKS:							break;
			case START_SERVER_TCP_CMD:			break;
			case GET_STATE_TCP_CMD:					break;
			case DATA_SENT_TCP_CMD:					break;
			case AVAIL_DATA_TCP_CMD:				break;
			case GET_DATA_TCP_CMD:					break;
			case START_CLIENT_TCP_CMD:			break;
			case STOP_CLIENT_TCP_CMD:				break;
			case GET_CLIENT_STATE_TCP_CMD:	break;
			case DISCONNECT_CMD:				break;
			case GET_IDX_RSSI_CMD: getRSSI(_reqPckt, _resPckt, 0);	break;
			case GET_IDX_ENCT_CMD: getEncryption(_reqPckt, _resPckt, 0);	break;
			case GET_HOST_BY_NAME_CMD:	break;
			case START_SCAN_NETWORKS:		break;
			case SEND_DATA_UDP_CMD:			break;
			case GET_REMOTE_DATA_CMD:		break;
			case SEND_DATA_TCP_CMD:			break;
			case GET_DATABUF_TCP_CMD:		break;
			case INSERT_DATABUF_CMD:		break;
			default:										break;
		}
	}
}

void CommLgc::getRSSI(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current){
	//TODO: To be tested
	int32_t result;// = -68;

	//retrieve RSSI

	if(current == 1){
		result = WiFi.RSSI();
	}
	else{
		uint8_t idx = String((_reqPckt->params[0].param[0])).toInt();

		// NOTE: only for test this function
		// User must call scan network before
		//int num = WiFi.scanNetworks();
		result = WiFi.RSSI(idx);
	}

	//Response contains 1 param with length 4
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 4;
	_resPckt->params[0].param = (char)result;
	_resPckt->params[0].param += (char)0xFF;
	_resPckt->params[0].param += (char)0xFF;
	_resPckt->params[0].param += (char)0xFF;

}

void CommLgc::getCurrentSSID(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested

	//retrieve SSID of the current network
	String result = WiFi.SSID();

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = result.length();
	_resPckt->params[0].param = result;

}

void CommLgc::getEncryption(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current){
	//TODO: To be tested

	uint8_t result = 0;
	uint8_t idx = 0;
	uint8_t numNets = WiFi.scanNetworks();	//get networks numbers

	if(current == 1){
		String currSSID = WiFi.SSID();					//get current SSID
		for(int i=0; i<numNets; i++){
			if(currSSID == WiFi.SSID(i)){
				idx = i;	//get the index of the current network
				break;
			}
		}
	}
	else{
		idx = String((_reqPckt->params[0].param[0])).toInt();
	}
	result = WiFi.encryptionType(idx);
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char)result;
}

void CommLgc::getMacAddress(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested

	int paramLen = 6;
	uint8_t mac[paramLen];

	//Retrive mac address
	WiFi.macAddress(mac);

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = paramLen;
	for(int i=0; i<paramLen; i++){
		_resPckt->params[0].param += (char)mac[i];
	}
}

CommLgc CommunicationLogic;
