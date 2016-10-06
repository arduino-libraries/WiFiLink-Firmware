//Communication Interface
#include "CommLgc.h"
#include "CommCmd.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiScan.h>
#include "utility/wifi_utils.h"

char FW_VERSION[] = "0.0.1";

WiFiServer* _wifi_server;

//cached values
IPAddress _reqHostIp;

CommLgc::CommLgc(){
	//while(!CommunicationInterface.begin());
}

/** Logic Functions **/

void CommLgc::begin(){
	while(!CommunicationInterface.begin());
}

void CommLgc::handle(){

	tMsgPacket pckt1;
	tMsgPacket *reqPckt = &pckt1;

	tMsgPacket pckt2;
	tMsgPacket *resPckt = &pckt2;

	if( CommunicationInterface.read(reqPckt) == 0){
		Serial1.println("==== RECEIVED ====");
		DEBUG(reqPckt);
		Serial1.println("==================");
		process(reqPckt, resPckt);
		Serial1.println("=== TRANSMITTED ==");
		DEBUG(resPckt);
		Serial1.println("==================");
		CommunicationInterface.write(resPckt);
		freeMem(reqPckt);
		freeMem(resPckt);
		//TODO: free memory
	}
}

void CommLgc::freeMem(tMsgPacket *_pckt){

	for(int i=0; i<_pckt->nParam; i++)
		free(_pckt->params[i].param);
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

	Serial1.println("-- Free Memory ---");
	Serial1.println(ESP.getFreeHeap());
	Serial1.println("-----------------");
}

void CommLgc::createErrorResponse(tMsgPacket *_pckt){

	_pckt->cmd = ERR_CMD;
	_pckt->nParam = 0;

}

void CommLgc::process(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){

	if (	(_reqPckt->cmd == START_CMD) &&
				((_reqPckt->tcmd & REPLY_FLAG) == 0) ){

		_resPckt->cmd = START_CMD;
		_resPckt->tcmd = _reqPckt->tcmd | REPLY_FLAG;

		switch(_reqPckt->tcmd){

			case SET_NET_CMD:						begin(_reqPckt, _resPckt, 0);			break;
			case SET_PASSPHRASE_CMD:		begin(_reqPckt, _resPckt, 1);			break;
			case SET_IP_CONFIG_CMD:			config(_reqPckt, _resPckt);				break;
			//case SET_DNS_CONFIG_CMD:				break;
			case GET_CONN_STATUS_CMD:		getStatus(_reqPckt, _resPckt);		break;
			case GET_IPADDR_CMD:		getNetworkData(_reqPckt, _resPckt);		break;
			case GET_MACADDR_CMD:		getMacAddress(_reqPckt, _resPckt);		break;
			case GET_CURR_SSID_CMD: getCurrentSSID(_reqPckt, _resPckt);		break;
			case GET_CURR_BSSID_CMD:getBSSID(_reqPckt, _resPckt, 1);			break;
			case GET_CURR_RSSI_CMD:	getRSSI(_reqPckt, _resPckt, 1);				break;
			case GET_CURR_ENCT_CMD:	getEncryption(_reqPckt, _resPckt, 1);	break;
			case SCAN_NETWORKS:			scanNetwork(_reqPckt, _resPckt);			break;
			case START_SERVER_TCP_CMD:	startServer(_reqPckt, _resPckt);	break;
			case GET_STATE_TCP_CMD:			serverStatus(_reqPckt, _resPckt);	break;
			case DATA_SENT_TCP_CMD:					break;
			case AVAIL_DATA_TCP_CMD:				break;
			case GET_DATA_TCP_CMD:					break;
			case START_CLIENT_TCP_CMD:			break;
			case STOP_CLIENT_TCP_CMD:				break;
			case GET_CLIENT_STATE_TCP_CMD:	break;
			case DISCONNECT_CMD:				disconnect(_reqPckt, _resPckt);				break;
			case GET_IDX_RSSI_CMD:			getRSSI(_reqPckt, _resPckt, 0);				break;
			case GET_IDX_ENCT_CMD:			getEncryption(_reqPckt, _resPckt, 0);	break;
			case REQ_HOST_BY_NAME_CMD:	reqHostByName(_reqPckt, _resPckt);		break;
			case GET_HOST_BY_NAME_CMD:	getHostByName(_reqPckt, _resPckt);		break;
			case GET_FW_VERSION_CMD:		getFwVersion(_reqPckt, _resPckt);			break;
			case START_SCAN_NETWORKS:		startScanNetwork(_reqPckt, _resPckt);	break;
			case SEND_DATA_UDP_CMD:			break;
			case GET_REMOTE_DATA_CMD:		break;
			case SEND_DATA_TCP_CMD:			break;
			case GET_DATABUF_TCP_CMD:		break;
			case INSERT_DATABUF_CMD:		break;
			default:	createErrorResponse(_resPckt); break;
		}
	}
}

/* Commands Functions */
/* WiFi Base */
void CommLgc::getRSSI(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current){
	//TODO: To be tested
	int32_t result;

	//retrieve RSSI
	if(current == 1){
		result = WiFi.RSSI();
	}
	else{
		uint8_t idx = _reqPckt->params[0].param[0];

		// NOTE: only for test this function
		// user must call scan network before
		//int num = WiFi.scanNetworks();
		result = WiFi.RSSI(idx);
	}

	//Response contains 1 param with length 4
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 4;

	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;
	_resPckt->params[0].param[1] = 0xFF;
	_resPckt->params[0].param[2] = 0xFF;
	_resPckt->params[0].param[3] = 0xFF;

}

void CommLgc::getCurrentSSID(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested

	//retrieve SSID of the current network
	String result = WiFi.SSID();

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = result.length();

	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	for(int i=0; i< result.length(); i++){ //char *
		_resPckt->params[0].param[i] = result[i];
	}


}

void CommLgc::getEncryption(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current){
	//TODO: To be tested

	uint8_t result = 0;
	uint8_t idx = 0;

	if(current == 1){
		uint8_t numNets = WiFi.scanNetworks();	//get networks numbers
		String currSSID = WiFi.SSID();					//get current SSID
		for(int i=0; i<numNets; i++){
			if(currSSID == WiFi.SSID(i)){
				idx = i;	//get the index of the current network
				break;
			}
		}
	}
	else{
		idx = _reqPckt->params[0].param[0];
	}

	result = WiFi.encryptionType(idx);
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;

	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;
}

void CommLgc::getMacAddress(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested

	int paramLen = 6;
	uint8_t mac[paramLen];

	//Retrive mac address
	WiFi.macAddress(mac);

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = paramLen;

	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	for(int i=0; i<paramLen; i++){
		_resPckt->params[0].param[i] = mac[i];
	}
}

void CommLgc::disconnect(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested
	bool result;

	//Disconnet from the network
	result = WiFi.disconnect();

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;

	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}

void CommLgc::getStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested
	uint8_t result;

	//Disconnet from the network
	result = WiFi.status();

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;

	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;
}

void CommLgc::begin(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t idx){
	//TODO: To be tested
	uint8_t result;

	if(idx == 0){ // idx==0 - SET_NET_CMD
			//retrieve parameters
			char* ssid = _reqPckt->params[0].param;

			//set network and retrieve result
			result = WiFi.begin(ssid);
		}
	else{ // idx ==1 - SET_PASSPHRASE_CMD
			//retrieve parameters
			char* ssid = _reqPckt->params[0].param;
			char* pass = _reqPckt->params[1].param;

			//set network and retrieve result
			result = WiFi.begin(ssid, pass);
	}

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;

	_resPckt->params[0].param = (char*) malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}

void CommLgc::startScanNetwork(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested

	// Fake response
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;

	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = 1;

}

void CommLgc::scanNetwork(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested
	uint8_t numNets = WiFi.scanNetworks();

	_resPckt->nParam = (numNets <= MAX_PARAMS) ? numNets : MAX_PARAMS;

	for (int i=0; i<(int)_resPckt->nParam; i++)
	{
		String ssidNet = WiFi.SSID(i).c_str();
		_resPckt->params[i].paramLen = ssidNet.length() /* + 1*/;
		_resPckt->params[i].param = (char*)malloc( ssidNet.length() /* + 1 */);

		for(int j=0; j<ssidNet.length(); j++){
			_resPckt->params[i].param[j] = ssidNet[j];
		}
	}
}

void CommLgc::getBSSID(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current){
	//TODO: To be tested
	int paramLen = 6;
	uint8_t idx = 0;
	uint8_t* result;
	uint8_t numNets = WiFi.scanNetworks();	//get networks numbers

	if(current == 1){
		String currSSID = WiFi.SSID();					//get current SSID
		for(int i=0; i<numNets; i++){
			if(currSSID == WiFi.SSID(i)){
				idx = i;	//get the index of the current network
				break;
			}
		}
	}else {
		//TODO: not present in arduino wifi shield library
	}

	//Retrive the BSSID
	result = WiFi.BSSID(idx);

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = paramLen;

	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	for(int i=0; i<paramLen; i++){
		_resPckt->params[0].param[i] = result[i];
	}
}

void CommLgc::config(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	/*
	WiFi.config call arduino side: WiFi.config(local_ip, gateway, subnet, dns1, dns2)
	*/

	//TODO: To be tested
	bool result;

	//retrieve the static IP address
	uint8_t stip1 = _reqPckt->params[0].param[0];
	uint8_t stip2 = _reqPckt->params[0].param[1];
	uint8_t stip3 = _reqPckt->params[0].param[2];
	uint8_t stip4 = _reqPckt->params[0].param[3];
	IPAddress staticIP(stip1, stip2, stip3, stip4);

	//retrieve the gateway IP address
	uint8_t gwip1 = _reqPckt->params[1].param[0];
	uint8_t gwip2 = _reqPckt->params[1].param[1];
	uint8_t gwip3 = _reqPckt->params[1].param[2];
	uint8_t gwip4 = _reqPckt->params[1].param[3];
	IPAddress gateway(gwip1, gwip2, gwip3, gwip4);

	//retrieve the subnet mask
	uint8_t snip1 = _reqPckt->params[2].param[0];
	uint8_t snip2 = _reqPckt->params[2].param[1];
	uint8_t snip3 = _reqPckt->params[2].param[2];
	uint8_t snip4 = _reqPckt->params[2].param[3];
	IPAddress subnet(snip1, snip2, snip3, snip4);

	if(_reqPckt->nParam == 3){
		result = WiFi.config(staticIP, gateway, subnet);
	}
	else if(_reqPckt->nParam == 4){
		//retrieve the dns 1 address
		uint8_t dns1ip1 = _reqPckt->params[3].param[0];
		uint8_t dns1ip2 = _reqPckt->params[3].param[1];
		uint8_t dns1ip3 = _reqPckt->params[3].param[2];
		uint8_t dns1ip4 = _reqPckt->params[3].param[3];
		IPAddress dns1(dns1ip1, dns1ip2, dns1ip3, dns1ip4);

		result = WiFi.config(staticIP, gateway, subnet, dns1);
	}
	else if(_reqPckt->nParam == 5){
		//retrieve the dns 1 address
		uint8_t dns1ip1 = _reqPckt->params[3].param[0];
		uint8_t dns1ip2 = _reqPckt->params[3].param[1];
		uint8_t dns1ip3 = _reqPckt->params[3].param[2];
		uint8_t dns1ip4 = _reqPckt->params[3].param[3];
		IPAddress dns1(dns1ip1, dns1ip2, dns1ip3, dns1ip4);

		//retrieve the dns 2 address
		uint8_t dns2ip1 = _reqPckt->params[4].param[0];
		uint8_t dns2ip2 = _reqPckt->params[4].param[1];
		uint8_t dns2ip3 = _reqPckt->params[4].param[2];
		uint8_t dns2ip4 = _reqPckt->params[4].param[3];
		IPAddress dns2(dns2ip1, dns2ip2, dns2ip3, dns2ip4);

		result = WiFi.config(staticIP, gateway, subnet, dns1, dns2);
	}

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}


void CommLgc::reqHostByName(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested
	char* host;
	int result;

	host = _reqPckt->params[0].param; //get the host name to look up
	result = WiFi.hostByName(host, _reqHostIp); //retrieve the ip address of the host

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}

void CommLgc::getHostByName(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested

	//gets _reqHostIp (obtained before using reqHostByName) and send back to arduino

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 4;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = _reqHostIp.operator[](0);
	_resPckt->params[0].param[1] = _reqHostIp.operator[](1);
	_resPckt->params[0].param[2] = _reqHostIp.operator[](2);
	_resPckt->params[0].param[3] = _reqHostIp.operator[](3);

}

void CommLgc::getFwVersion(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested

	//send back to arduino the firmware version number

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = sizeof(FW_VERSION)-1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	strncpy(_resPckt->params[0].param, FW_VERSION, _resPckt->params[0].paramLen) ;

}

/* WiFi IPAddress*/
void CommLgc::getNetworkData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested

	IPAddress localIp, subnetMask, gatewayIp, dnsIp;

	localIp = WiFi.localIP();
	subnetMask = WiFi.subnetMask();
	gatewayIp = WiFi.gatewayIP();
	//dnsIp = WiFi.dnsIP();

	_resPckt->nParam = 3;
	_resPckt->params[0].paramLen = 4;
	_resPckt->params[1].paramLen = 4;
	_resPckt->params[2].paramLen = 4;

	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = localIp.operator[](0);
	_resPckt->params[0].param[1] = localIp.operator[](1);
	_resPckt->params[0].param[2] = localIp.operator[](2);
	_resPckt->params[0].param[3] = localIp.operator[](3);

	_resPckt->params[1].param = (char*)malloc(_resPckt->params[1].paramLen);
	_resPckt->params[1].param[0] = subnetMask.operator[](0);
	_resPckt->params[1].param[1] = subnetMask.operator[](1);
	_resPckt->params[1].param[2] = subnetMask.operator[](2);
	_resPckt->params[1].param[3] = subnetMask.operator[](3);

	_resPckt->params[2].param = (char*)malloc(_resPckt->params[2].paramLen);
	_resPckt->params[2].param[0] = gatewayIp.operator[](0);
	_resPckt->params[2].param[1] = gatewayIp.operator[](1);
	_resPckt->params[2].param[2] = gatewayIp.operator[](2);
	_resPckt->params[2].param[3] = gatewayIp.operator[](3);

}


/* WiFI Server */
void CommLgc::startServer(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested
	int _port = 0;
	int _socks = 0;
	uint8_t _prot_mode = 0;

	String _ports;
	//retrieve the port to start server
	for(int i=0;  i<(int)_reqPckt->params[0].paramLen; i++){
		_ports += _reqPckt->params[0].param[i];
	}
	_port = _ports.toInt();

	//retrieve sockets number
	_socks = (int)_reqPckt->params[1].param[0];

	//retrieve protocol mode (TCP/UDP)
	_prot_mode = (uint8_t)_reqPckt->params[2].param[0];

	delete _wifi_server;
	_wifi_server = new WiFiServer(_port);
	_wifi_server->begin();

	//TODO sockets and protocol

	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = 1;

}

void CommLgc::available(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested
	//TODO
}

void CommLgc::serverStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested
	uint8_t result = 0;
	uint8_t _socket = 0;

	//retrieve socket index
	_socket = (uint8_t)_reqPckt->params[0].param[0];

	//NOTE =0 is the case of a direct call to WiFiServer.status();
	if(_socket == 0){
		if(_wifi_server != NULL){
			result = _wifi_server->status();
		} else { //wifi server is not started
			result = 0; //TODO take a look and understand the correct value for this case
		}

	}
	//NOTE >0 are the cases of a non-direct calls, like WiFiServer.available();
	else {
		//TODO ???
	}

	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}


/* WiFi Client */
void CommLgc::stopClient(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested
	//TODO
}

void CommLgc::clientStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested
	//TODO
	uint8_t result = 0;
	uint8_t _socket = 0; //socket index

	// //retrieve socket index
	// _socket = (uint8_t)_reqPckt->params[0].param[0];
	//
	// //NOTE =0 is the case of a direct call to WiFiClient.status();
	// if(_socket == 0){
	// 	if(/*_wifi_server*/ != NULL){
	// 		result = /*_wifi_server->status()*/;
	// 	} else { //wifi server is not started
	// 		result = 0; //TODO take a look and understand the correct value for this case
	// 	}
	//
	// }
	// //NOTE >0 are the cases of a non-direct calls, like WiFiServer.available();
	// else {
	// 	//TODO ???
	// }

	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}

void CommLgc::startClient(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested
	//TODO
}


// void CommLgc::getParam(tParam *param, uint8_t *data){
// 	for(int i=0; i< param->paramLen; i++){
// 		data[i] = param->param[i];
// 	}
// }
//TODO code a function to get/set params from/to the request/response struct

// setParam(tParam *param, char* data){
//
// 	param->paramLen = sizeof(data);
// 	params->param = (char*)malloc(params.paramLen);
// 	for(int i=0; i < param->paramLen; i++){
// 		params->param[i] = data[i];
// 	}
//
// }

CommLgc CommunicationLogic;
