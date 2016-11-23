//Communication Interface
#include "CommLgc.h"
#include "CommCmd.h"

char FW_VERSION[] = "0.0.1";

//cached values
IPAddress _reqHostIp;

IPAddress* _handyIp;
IPAddress* _handySubnet;
IPAddress* _handyGateway;

uint8_t tcpResult = 0;

int bufferSize = 0;

//WiFiServer, WiFiClient and WiFiUDP map
WiFiServer* mapWiFiServers[MAX_SOCK_NUM];
WiFiClient mapWiFiClients[MAX_SOCK_NUM];
WiFiUDP mapWiFiUDP[MAX_SOCK_NUM];

WiFiClient client;

#define Serial1 Serial
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
		//if(debug){
			// Serial1.println("==== RECEIVED ====");
			// DEBUG(reqPckt);
			// 	Serial1.println("==================");
		 	process(reqPckt, resPckt);
			// 	Serial1.println("=== TRANSMITTED ==");
			//  DEBUG(resPckt);
			// 	Serial1.println("==================");
		//}
		CommunicationInterface.write(resPckt);
		//Free momory
		freeMem(reqPckt);
		freeMem(resPckt);
		//DEBUG_MEM();
	}
}

void CommLgc::freeMem(tMsgPacket *_pckt){
	if((_pckt->tcmd >= 0x40 && _pckt->tcmd < 0x50) || ( _pckt->tcmd >= (0x40 | REPLY_FLAG) && _pckt->tcmd < (0x50 | REPLY_FLAG)) && _pckt->tcmd != (0x44 | REPLY_FLAG)){ //16 Bit
		for(int i=0; i<_pckt->nParam; i++)
			free(_pckt->paramsData[i].data);
	}
	else{ //8 Bit
		for(int i=0; i<_pckt->nParam; i++)
			free(_pckt->params[i].param);
	}
}

void CommLgc::DEBUG_MEM() {
	Serial1.print("-- Free Memory: ");
	Serial1.print(ESP.getFreeHeap());
	Serial1.println(" --");
}

void CommLgc::DEBUG(tMsgPacket *_pckt) {

	Serial1.println("--- Packet  ---");
	Serial1.println(_pckt->cmd, HEX);
	Serial1.println(_pckt->tcmd, HEX);
	Serial1.println(_pckt->nParam, HEX);
	for(int i=0; i<(int)_pckt->nParam; i++){
		if(_pckt->tcmd >= 0x40 && _pckt->tcmd < 0x50){ //16 Bit
			Serial1.println(_pckt->paramsData[i].dataLen, HEX );
			for(int j=0; j< (int)_pckt->paramsData[i].dataLen; j++)
				Serial1.println( _pckt->paramsData[i].data[j], HEX);
		}
		else{ //8 Bit
			Serial1.println(_pckt->params[i].paramLen, HEX );
			for(int j=0; j< (int)_pckt->params[i].paramLen; j++)
				Serial1.println( _pckt->params[i].param[j], HEX);
			}
	}
	Serial1.println(0xEE, HEX);
	Serial1.println("--- End Packet ---");
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
			case SET_DNS_CONFIG_CMD:		setDNS(_reqPckt, _resPckt);				break;
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
			case DATA_SENT_TCP_CMD:			checkDataSent(_reqPckt, _resPckt);break;
			case AVAIL_DATA_TCP_CMD:		availData(_reqPckt, _resPckt);		break;
			case GET_DATA_TCP_CMD:			getData(_reqPckt, _resPckt);			break;
			case START_CLIENT_TCP_CMD:	startClient(_reqPckt, _resPckt);			break;
			case STOP_CLIENT_TCP_CMD:		stopClient(_reqPckt, _resPckt);				break;
			case GET_CLIENT_STATE_TCP_CMD:	clientStatus(_reqPckt, _resPckt);	break;
			case DISCONNECT_CMD:				disconnect(_reqPckt, _resPckt);				break;
			case GET_IDX_RSSI_CMD:			getRSSI(_reqPckt, _resPckt, 0);				break;
			case GET_IDX_ENCT_CMD:			getEncryption(_reqPckt, _resPckt, 0);	break;
			case REQ_HOST_BY_NAME_CMD:	reqHostByName(_reqPckt, _resPckt);		break;
			case GET_HOST_BY_NAME_CMD:	getHostByName(_reqPckt, _resPckt);		break;
			case GET_FW_VERSION_CMD:		getFwVersion(_reqPckt, _resPckt);			break;
			case START_SCAN_NETWORKS:		startScanNetwork(_reqPckt, _resPckt);	break;
			case SEND_DATA_UDP_CMD:			sendUdpData(_reqPckt, _resPckt);	break;
			case GET_REMOTE_DATA_CMD:		remoteData(_reqPckt, _resPckt);		break;
			case SEND_DATA_TCP_CMD:			sendData(_reqPckt, _resPckt);			break;
			case GET_DATABUF_TCP_CMD:		getDataBuf(_reqPckt, _resPckt);		break;
			case INSERT_DATABUF_CMD:		insDataBuf(_reqPckt, _resPckt);		break;
			//case PARSE_UDP_PCK:					insDataBuf(_reqPckt, _resPckt);		break;
			default:										createErrorResponse(_resPckt); 		break;
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
	for (int i=0, j=paramLen-1; i<paramLen, j>=0; i++, j--){
		_resPckt->params[0].param[i] = mac[j];
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
			char ssid[_reqPckt->params[0].paramLen+1];
			strncpy(ssid, _reqPckt->params[0].param, _reqPckt->params[0].paramLen);
			ssid[_reqPckt->params[0].paramLen] = '\0';

			//set network and retrieve result
			result = WiFi.begin(ssid);
		}
	else{ // idx ==1 - SET_PASSPHRASE_CMD
			//retrieve parameters
			char ssid[_reqPckt->params[0].paramLen+1];
			char pass[_reqPckt->params[1].paramLen+1];
			strncpy(ssid, _reqPckt->params[0].param, _reqPckt->params[0].paramLen);
			strncpy(pass, _reqPckt->params[1].param, _reqPckt->params[1].paramLen);
			ssid[_reqPckt->params[0].paramLen] = '\0';
			pass[_reqPckt->params[1].paramLen] = '\0';

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
	for (int i=0, j=paramLen-1; i<paramLen, j>=0; i++, j--){
		_resPckt->params[0].param[i] = result[j];
	}
}

void CommLgc::config(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	/*
	WiFi.config call arduino side: WiFi.config(local_ip, gateway, subnet, dns1, dns2)
	*/

	//TODO: To be tested
	bool result;
	uint8_t validParams = 0;

	uint8_t stip0, stip1, stip2, stip3,
					gwip0, gwip1, gwip2, gwip3,
					snip0, snip1, snip2, snip3;

	validParams = _reqPckt->params[0].param[0];

	//retrieve the static IP address
	stip0 = _reqPckt->params[1].param[0];
	stip1 = _reqPckt->params[1].param[1];
	stip2 = _reqPckt->params[1].param[2];
	stip3 = _reqPckt->params[1].param[3];
	_handyIp = new IPAddress(stip0, stip1, stip2, stip3);


	//retrieve the gateway IP address
	gwip0 = _reqPckt->params[2].param[0];
	gwip1 = _reqPckt->params[2].param[1];
	gwip2 = _reqPckt->params[2].param[2];
	gwip3 = _reqPckt->params[2].param[3];
	_handyGateway = new IPAddress(gwip0, gwip1, gwip2, gwip3);

	//retrieve the subnet mask
	snip0 = _reqPckt->params[3].param[0];
	snip1 = _reqPckt->params[3].param[1];
	snip2 = _reqPckt->params[3].param[2];
	snip3 = _reqPckt->params[3].param[3];
	_handySubnet = new IPAddress(snip0, snip1, snip2, snip3);

	result = WiFi.config(*_handyIp, *_handyGateway, *_handySubnet);

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}

void CommLgc::setDNS(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested
	bool result;
	uint8_t validParams = 0;

	validParams = _reqPckt->params[0].param[0];

	uint8_t dns1ip0, dns1ip1, dns1ip2, dns1ip3,
					dns2ip0, dns2ip1, dns2ip2, dns2ip3;

	//retrieve the dns 1 address
	dns1ip0 = _reqPckt->params[1].param[0];
	dns1ip1 = _reqPckt->params[1].param[1];
	dns1ip2 = _reqPckt->params[1].param[2];
	dns1ip3 = _reqPckt->params[1].param[3];
	IPAddress dns1(dns1ip0, dns1ip1, dns1ip2, dns1ip3);

	//retrieve the dns 2 address
	dns2ip0 = _reqPckt->params[2].param[0];
	dns2ip1 = _reqPckt->params[2].param[1];
	dns2ip2 = _reqPckt->params[2].param[2];
	dns2ip3 = _reqPckt->params[2].param[3];
	IPAddress dns2(dns2ip0, dns2ip1, dns2ip2, dns2ip3);

	result = WiFi.config(*_handyIp, *_handyGateway, *_handySubnet, dns1, dns2);

	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;
}

void CommLgc::reqHostByName(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested

	char host[_reqPckt->params[0].paramLen];
	int result;

	//get the host name to look up
	strncpy(host, _reqPckt->params[0].param, _reqPckt->params[0].paramLen);
	host[_reqPckt->params[0].paramLen] = '\0';

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

	IPAddress localIp, subnetMask, gatewayIp;//, dnsIp1, dnsIp2;

	localIp = WiFi.localIP();
	subnetMask = WiFi.subnetMask();
	gatewayIp = WiFi.gatewayIP();
	// dnsIp1 = WiFi.dnsIP(0); //Ready to have even DNS 1 and DNS 2
	// dnsIp2 = WiFi.dnsIP(1);

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
	uint8_t result = 0;
	uint16_t _port = 0;
	int _sock = 0;
	uint8_t _prot = 0;

	//retrieve the port to start server
	uint8_t _p1 = (uint8_t)_reqPckt->params[0].param[0];
	uint8_t _p2 = (uint8_t)_reqPckt->params[0].param[1];
	_port = (_p1 << 8) + _p2;

	//retrieve sockets number
	_sock = (int)_reqPckt->params[1].param[0];

	//retrieve protocol mode (TCP/UDP)
	_prot = (uint8_t)_reqPckt->params[2].param[0];
	if(_sock < MAX_SOCK_NUM) {
		if(_prot == 0){ //TCP MODE
			if(mapWiFiServers[_sock] != NULL ){
				mapWiFiServers[_sock]->stop();
				mapWiFiServers[_sock]->close();
				free(mapWiFiServers[_sock]);
			}
			mapWiFiServers[_sock] = new WiFiServer(_port);
			mapWiFiServers[_sock]->begin();
			result = 1;
		} else {	//UDP MODE
			if(mapWiFiUDP[_sock] != NULL ){
				//Serial1.println("START_SERVER_TCP_CMD: WIFI UDP EXISTS");
				//TODO: stop, close?
			} else {
				//Serial1.println("START_SERVER_TCP_CMD: WIFI UDP NOT EXISTS");
				mapWiFiUDP[_sock].begin(_port);
				result = 1;
			}
		}
	}
	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}

void CommLgc::availData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested
	uint16_t result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt->params[0].param[0];

	if(_sock < MAX_SOCK_NUM) {
		if(mapWiFiClients[_sock] != NULL ){
			result = mapWiFiClients[_sock].available();
		}
		else if(mapWiFiUDP[_sock] != NULL){//non ho un riferimento sul protocollo, quindi uso l'indice di socket
			//Serial1.println("AVAIL DATA 5 UDP");
			result = mapWiFiUDP[_sock].parsePacket();
			//result = mapWiFiUDP[_sock].available();
			//Serial1.println("AVAIL DATA 6 UDP");Serial1.println("AVAIL DATA 7 UDP");Serial1.println(result);
		}
	}
	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = sizeof(result);
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);

	for(int i=0; i<_resPckt->params[0].paramLen; i++){
		_resPckt->params[0].param[i] = ((uint8_t*)&result)[i];
	}

	bufferSize = result;
}

void CommLgc::serverStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested
	uint8_t result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt->params[0].param[0];

	result = mapWiFiServers[_sock]->status();

	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}

void CommLgc::getData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested
	int result = 0;
	uint8_t _sock = 0;
	uint8_t _peek = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt->params[0].param[0];

	//retrieve peek
	_peek = (uint8_t)_reqPckt->params[1].param[0];

	if(mapWiFiClients[_sock] != NULL ){
		if(_peek > 0){
			result = mapWiFiClients[_sock].peek();
		}
		else{
			result = mapWiFiClients[_sock].read();
		}
	}
	else if(mapWiFiUDP[_sock] != NULL){
		if(_peek > 0){
			result = mapWiFiUDP[_sock].peek();
		}
		else{
			result = mapWiFiUDP[_sock].read();
		}
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
	uint8_t result = 0;
	uint8_t _sock = 0;

	_sock = (uint8_t)_reqPckt->params[0].param[0];

	if(_sock < MAX_SOCK_NUM){
		if(mapWiFiClients[_sock] != NULL ){
			mapWiFiClients[_sock].stop();
			result = 1;
		}
		else if(mapWiFiUDP[_sock] != NULL){
			mapWiFiUDP[_sock].stop();
			result = 1;
		}
	}

	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}

void CommLgc::clientStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested
	uint8_t result = 0;
	uint8_t _sock = 0; //socket index
	_sock = (uint8_t)_reqPckt->params[0].param[0];
	if(_sock < MAX_SOCK_NUM) {
		if(mapWiFiClients[_sock] == NULL){
			if(mapWiFiServers[_sock] != NULL){
				mapWiFiClients[_sock] = mapWiFiServers[_sock]->available(); //Create the client from the server [Arduino as a Server]
				result = mapWiFiClients[_sock].status();
			}
		}else {
			result = mapWiFiClients[_sock].status();
		}
	}

	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;
}

void CommLgc::sendData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested

	int result = 0;
	uint8_t _sock = 0; //socket index

	//retrieve socket index and data
	_sock = (uint8_t)_reqPckt->paramsData[0].data[0];

	if(mapWiFiClients[_sock] != NULL){
		//send data to client
		result = mapWiFiClients[_sock].write(_reqPckt->paramsData[1].data, _reqPckt->paramsData[1].dataLen);
		if(result == _reqPckt->paramsData[1].dataLen)
			tcpResult = 1;
		else
			tcpResult = 0;
	}
	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = tcpResult;//(result > 0) ? 1 : 0;

}

void CommLgc::checkDataSent(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested

	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = tcpResult; //(tcpResult > 0) ? 1 : 0;
}

void CommLgc::startClient(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested
	int result = 0;
	int _sock;
	uint16_t _port;
	uint8_t _prot;

	//retrieve the IP address to connect to
	uint8_t stip1 = _reqPckt->params[0].param[0];
	uint8_t stip2 = _reqPckt->params[0].param[1];
	uint8_t stip3 = _reqPckt->params[0].param[2];
	uint8_t stip4 = _reqPckt->params[0].param[3];
	IPAddress _ip(stip1, stip2, stip3, stip4);

	//retrieve the port to connect to
	uint8_t _p1 = (uint8_t)_reqPckt->params[1].param[0];
	uint8_t _p2 = (uint8_t)_reqPckt->params[1].param[1];
	_port = (_p1 << 8) + _p2;

	//retrieve sockets number
	_sock = (int)_reqPckt->params[2].param[0];

	//retrieve protocol mode (TCP/UDP)
	_prot = (uint8_t)_reqPckt->params[3].param[0];

	if(_sock < MAX_SOCK_NUM) {
		if(_prot == 0){ //TCP MODE
			if(mapWiFiClients[_sock] == NULL){
				WiFiClient wc;
				mapWiFiClients[_sock] = wc;
			}
			result = mapWiFiClients[_sock].connect(_ip, _port);
		} else { //UDP MODE
			if(mapWiFiUDP[_sock] == NULL){
				WiFiUDP wu;
				mapWiFiUDP[_sock] = wu;
			}//Serial1.println("BEGIN PACKET 1");Serial1.println(_ip);Serial1.println(_port);
			result = mapWiFiUDP[_sock].beginPacket(_ip, _port);
			//Serial1.println("BEGIN PACKET 2");
		}
	}
	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;
}

/* WiFi UDP Client */
void CommLgc::remoteData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO to be tested
	int _sock;

	//retrieve sockets number
	_sock = (int)_reqPckt->params[0].param[0];

	if(_sock < MAX_SOCK_NUM && mapWiFiUDP[_sock] != NULL) {
		_resPckt->nParam = 2;
		_resPckt->params[0].paramLen = 4;
		_resPckt->params[1].paramLen = 2;

		IPAddress remoteIp = mapWiFiUDP[_sock].remoteIP();
		_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
		_resPckt->params[0].param[0] = remoteIp.operator[](0);
		_resPckt->params[0].param[1] = remoteIp.operator[](1);
		_resPckt->params[0].param[2] = remoteIp.operator[](2);
		_resPckt->params[0].param[3] = remoteIp.operator[](3);

		uint16_t remotePort = mapWiFiUDP[_sock].remotePort();
		_resPckt->params[1].param = (char*)malloc(_resPckt->params[1].paramLen);
		_resPckt->params[1].param[0] = (uint8_t)((remotePort & 0xff00)>>8);
		_resPckt->params[1].param[1] = (uint8_t)(remotePort & 0xff);

	} else {
		createErrorResponse(_resPckt);
	}
}

void CommLgc::getDataBuf(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested
	//Serial1.println("DATA BUF 1");
	int result = 0;
	uint8_t _sock = 0;
	//char buffer[bufferSize]; //bufferSize is filled before by availData
	//Serial1.println("DATA BUF 2 ");//Serial1.println(bufferSize);
	//retrieve socket index
	_sock = (uint8_t)_reqPckt->paramsData[0].data[0];
	//Serial1.println("DATA BUF 3 ");Serial1.println(_sock);
	if(_sock < MAX_SOCK_NUM){
	  if(mapWiFiUDP[_sock] != NULL){
  		//Serial1.println("DATA BUF 4");
      char buffer[bufferSize]; //bufferSize is filled before by availData
  		result = mapWiFiUDP[_sock].read(buffer, bufferSize);
      _resPckt->nParam = 1;
      _resPckt->paramsData[0].dataLen = bufferSize;
      _resPckt->paramsData[0].data = (char*)malloc(_resPckt->params[0].paramLen);
      strncpy(_resPckt->paramsData[0].data, buffer, bufferSize);
	  }
    else if(mapWiFiClients[_sock] != NULL){
      //Serial1.println("DATA BUF 3");
      //bufferSize =5;
      uint8_t buffer_tcp[bufferSize];
      result = mapWiFiClients[_sock].read(buffer_tcp, bufferSize);  
      //Serial1.println("DATA BUF 4");
      //Serial1.println(buffer_tcp);
      _resPckt->nParam = 1;
      _resPckt->paramsData[0].dataLen = bufferSize;
      _resPckt->paramsData[0].data = (char*)malloc(_resPckt->paramsData[0].dataLen);
      strncpy(_resPckt->paramsData[0].data, (char*)buffer_tcp, bufferSize);
      //_resPckt->paramsData[0].data = "ciao\0";
      //host[_reqPckt->params[0].paramLen] = '\0';
      //Serial1.println("DATA BUF 5");
    }

	}
	//Serial1.println("DATA BUF 6");
}

void CommLgc::insDataBuf(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested

	//NOTE myabe can use sendData, it's similar to this except the UDP

	uint8_t result = 0;
	uint8_t _sock = 0;
	//Serial1.println("INS BUF 1 ");
	//retrieve socket index
	_sock = (uint8_t)_reqPckt->paramsData[0].data[0];
	//Serial1.println("INS BUF 2 ");Serial1.println(_sock);
	if(_sock < MAX_SOCK_NUM && mapWiFiUDP[_sock] != NULL){
		//send data to client
		//Serial1.println("INS BUF 3 ");
		//Serial1.println(_reqPckt->paramsData[1].data);
		//Serial1.println("INS BUF 4 ");
		//Serial1.println(_reqPckt->paramsData[1].dataLen);
		//Serial1.println("INS BUF 5 ");
		//result =
		mapWiFiUDP[_sock].write(_reqPckt->paramsData[1].data, _reqPckt->paramsData[1].dataLen);
		//Serial1.println(result);
		//Serial1.println("INS BUF 6 ");
		result = 1;
	}
	//Serial1.println("INS BUF 7 ");
	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;
	DEBUG(_resPckt);
}

void CommLgc::sendUdpData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
	//TODO: To be tested
	int result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt->paramsData[0].data[0];

	if(_sock < MAX_SOCK_NUM && mapWiFiUDP[_sock] != NULL){
		//send data to client
		result = mapWiFiUDP[_sock].endPacket();
	}
	//set the response struct
	_resPckt->nParam = 1;
	_resPckt->params[0].paramLen = 1;
	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	_resPckt->params[0].param[0] = result;

}

//ServerDrv::insertDataBuf(_sock, buffer, size);

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
