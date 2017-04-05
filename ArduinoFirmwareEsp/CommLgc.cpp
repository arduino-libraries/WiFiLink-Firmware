/*
Copyright <2017> <COPYRIGHT HOLDER>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE

*/

//Communication Interface
#include "CommLgc.h"
#include "CommCmd.h"

char FW_VERSION[] = "1.0.0";

//cached values
IPAddress _reqHostIp;

IPAddress* _handyIp;
IPAddress* _handySubnet;
IPAddress* _handyGateway;

uint8_t tcpResult = 0;											//used by availData function
uint16_t bufferSize = 0;													//used by getDataBuf function
uint8_t numNets;														//number of networks scanned

//WiFiServer, WiFiClient and WiFiUDP map
WiFiServer* mapWiFiServers[MAX_SOCK_NUMBER];
WiFiClient mapWiFiClients[MAX_SOCK_NUMBER];
WiFiUDP mapWiFiUDP[MAX_SOCK_NUMBER];

tMsgPacket _reqPckt;                          //initialize struct to receive a command from MCU
uint8_t _resPckt[RESPONSE_LENGHT];								//response array
int transfer_size = 0;												//size of array response (length/32)

CommLgc::CommLgc(){
}

/** Logic Functions **/

void CommLgc::begin(){
	while(!CommunicationInterface.begin());
}

void CommLgc::handle(){
	if(CommunicationInterface.available()){
		if(CommunicationInterface.read(&_reqPckt)==0){
			process();
			CommunicationInterface.write(_resPckt,transfer_size);
			transfer_size = 0;		//reset transfer size
		}
		else{
			//TODO
		}
	}
}

void CommLgc::createErrorResponse(){
	_resPckt[0] = ERR_CMD;
	_resPckt[1] = 0;
	_resPckt[2] = END_CMD;
}

void CommLgc::process(){

	if ((_reqPckt.cmd == START_CMD) &&
				((_reqPckt.tcmd & REPLY_FLAG) == 0)){

		_resPckt[0] = 0xE0;
		_resPckt[1] = _reqPckt.tcmd | REPLY_FLAG;

		switch(_reqPckt.tcmd){
			case SET_NET_CMD:						begin(0);					break;
			case SET_PASSPHRASE_CMD:		begin(1);					break;
			case SET_IP_CONFIG_CMD:			config();					break;
			case SET_DNS_CONFIG_CMD:		setDNS();					break;
			case GET_CONN_STATUS_CMD:		getStatus();			break;
			case GET_IPADDR_CMD:				getNetworkData();	break;
			case GET_MACADDR_CMD:				getMacAddress();	break;
			case GET_CURR_SSID_CMD: 		getCurrentSSID();	break;
			case GET_CURR_BSSID_CMD:		getBSSID(1);			break;
			case GET_CURR_RSSI_CMD:			getRSSI(1);				break;
			case GET_CURR_ENCT_CMD:			getEncryption(1);	break;
			case SCAN_NETWORKS:					scanNetwork();		break;
			case START_SERVER_TCP_CMD:	startServer();		break;
			case GET_STATE_TCP_CMD:			serverStatus();		break;
			case DATA_SENT_TCP_CMD:			checkDataSent();	break;
			case AVAIL_DATA_TCP_CMD:		availData();			break;
			case GET_DATA_TCP_CMD:			getData();				break;
			case START_CLIENT_TCP_CMD:	startClient();		break;
			case STOP_CLIENT_TCP_CMD:		stopClient();			break;
			case GET_CLIENT_STATE_TCP_CMD:	clientStatus();	break;
			case DISCONNECT_CMD:				disconnect();				break;
			case GET_IDX_RSSI_CMD:			getRSSI(0);				break;
			case GET_IDX_ENCT_CMD:			getEncryption(0);	break;
			case REQ_HOST_BY_NAME_CMD:	reqHostByName();	break;
			case GET_HOST_BY_NAME_CMD:	getHostByName();	break;
			case GET_FW_VERSION_CMD:		getFwVersion();		break;
			case START_SCAN_NETWORKS:		startScanNetwork();	break;
			case SEND_DATA_UDP_CMD:			sendUdpData();		break;
			case GET_REMOTE_DATA_CMD:		remoteData();			break;
			case SEND_DATA_TCP_CMD:			sendData();				break;
			case GET_DATABUF_TCP_CMD:		getDataBuf();			break;
			case INSERT_DATABUF_CMD:		insDataBuf();			break;
			default:										createErrorResponse(); 		break;
		}
	}
}

/* Commands Functions */
/* WiFi Base */
void CommLgc::getRSSI(uint8_t current){

	int resp_idx=2;
	int32_t result;
	//retrieve RSSI
	if(current == 1){
		result = WiFi.RSSI();
	}
	else{
		uint8_t net_idx = _reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];

		// NOTE: only for test this function
		// user must call scan network before
		// int num = WiFi.scanNetworks();
		result = WiFi.RSSI(net_idx);
	}

	//Response contains 1 param with length 4
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_4;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = 0xFF;
	_resPckt[resp_idx++] = 0xFF;
	_resPckt[resp_idx++] = 0xFF;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::getCurrentSSID(){

	int resp_idx = 2;

	//retrieve SSID of the current network
	String result = WiFi.SSID();

	//set the response struct
	_resPckt[resp_idx++] = 1;
	_resPckt[resp_idx++] = result.length();
	memcpy(_resPckt+resp_idx,result.c_str(),result.length());
	resp_idx = resp_idx+result.length();
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::getEncryption(uint8_t current){

	uint8_t result = 0;
	uint8_t net_idx =	0; //network index
	int resp_idx = 2;

	if(current == 1){
		//uint8_t numNets = WiFi.scanNetworks();	//get networks numbers
		String currSSID = WiFi.SSID();					//get current SSID
		for(int i=0; i<numNets; i++){
			if(currSSID == WiFi.SSID(i)){
				net_idx = i;	//get the index of the current network
				break;
			}
		}
	}
	else{
		net_idx = _reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];
	}

	result = WiFi.encryptionType(net_idx);
	//if result
	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;
}

void CommLgc::getMacAddress(){

	int resp_idx = 2;
	uint8_t mac[PARAM_SIZE_6];

	//Retrive mac address
	WiFi.macAddress(mac);

	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_6;
	for (int i=0, j=PARAM_SIZE_6-1; i<PARAM_SIZE_6, j>=0; i++, j--){
		_resPckt[resp_idx++]=mac[j];
	}
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::disconnect(){

	int resp_idx = 2;
	bool result;

	//Disconnet from the network
	result = WiFi.disconnect();

	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::getStatus(){

	int resp_idx = 2;
	uint8_t result;

	//Disconnet from the network
	result = WiFi.status();

	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::begin(uint8_t idx){

	int resp_idx = 2;
	uint8_t result;

	if(idx == 0){ // if idx==0 connection without password
			//retrieve parameters
			char ssid[_reqPckt.params[PARAM_NUMS_0].paramLen+1];
			//strncpy(ssid, _reqPckt.params[0].param, _reqPckt.params[0].paramLen);
			memcpy(ssid, _reqPckt.params[PARAM_NUMS_0].param, _reqPckt.params[PARAM_NUMS_0].paramLen);
			ssid[_reqPckt.params[PARAM_NUMS_0].paramLen] = '\0';
			//set network and retrieve result
			result = WiFi.begin(ssid);
	}
	else{
			//retrieve parameters
			char ssid[_reqPckt.params[PARAM_NUMS_0].paramLen+1];
			char pass[_reqPckt.params[PARAM_NUMS_1].paramLen+1];
			//strncpy(ssid, _reqPckt.params[0].param, _reqPckt.params[0].paramLen);
			//strncpy(pass, _reqPckt.params[1].param, _reqPckt.params[1].paramLen);
			memcpy(ssid, _reqPckt.params[PARAM_NUMS_0].param, _reqPckt.params[PARAM_NUMS_0].paramLen);
			memcpy(pass, _reqPckt.params[PARAM_NUMS_1].param, _reqPckt.params[PARAM_NUMS_1].paramLen);
			ssid[_reqPckt.params[PARAM_NUMS_0].paramLen] = '\0';
			pass[_reqPckt.params[PARAM_NUMS_1].paramLen] = '\0';
			//set network and retrieve result
			result = WiFi.begin(ssid, pass);
	}

	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::startScanNetwork(){

	int resp_idx = 2;
	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = 1;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::scanNetwork(){

	int resp_idx = 2;
	//scanNetworks command
	numNets = WiFi.scanNetworks();
	//fix the maximum network number to 10
	uint8_t numNetsMax = 10

	numNets = (numNets <= numNetsMax) ? numNets : numNetsMax;
	_resPckt[resp_idx++]=numNets;
	for (int i=0; i<numNets; i++)
	{
		String ssidNet = WiFi.SSID(i);
		_resPckt[resp_idx++] = ssidNet.length();
		//set response array
		//strncpy(_resPckt+idx, ssidNet.c_str(), ssidNet.length());
		memcpy(_resPckt+resp_idx, ssidNet.c_str(), ssidNet.length());
		resp_idx = resp_idx+ssidNet.length();

	}
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::getBSSID(uint8_t current){
	//TODO: To be tested
	int paramLen = 6;
	uint8_t idx = 0;		//network index
	int resp_idx = 2;
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
	result = WiFi.BSSID();

	//set the response struct
	_resPckt[resp_idx++] = 1;
	_resPckt[resp_idx++] = paramLen;
	for (int j=paramLen-1; j>=0; j--){
		_resPckt[resp_idx++] = result[j];
	}
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::config(){
	/*
	WiFi.config call arduino side: WiFi.config(local_ip, gateway, subnet, dns1, dns2)
	*/

	//TODO: To be tested
	bool result;
	int resp_idx = 2;
	uint8_t validParams = 0;

	uint8_t stip0, stip1, stip2, stip3,
					gwip0, gwip1, gwip2, gwip3,
					snip0, snip1, snip2, snip3;

	validParams = _reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];

	//retrieve the static IP address
	stip0 = _reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_0];
	stip1 = _reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_1];
	stip2 = _reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_2];
	stip3 = _reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_3];
	_handyIp = new IPAddress(stip0, stip1, stip2, stip3);


	//retrieve the gateway IP address
	gwip0 = _reqPckt.params[PARAM_NUMS_2].param[PARAM_NUMS_0];
	gwip1 = _reqPckt.params[PARAM_NUMS_2].param[PARAM_NUMS_1];
	gwip2 = _reqPckt.params[PARAM_NUMS_2].param[PARAM_NUMS_2];
	gwip3 = _reqPckt.params[PARAM_NUMS_2].param[PARAM_NUMS_3];
	_handyGateway = new IPAddress(gwip0, gwip1, gwip2, gwip3);

	//retrieve the subnet mask
	snip0 = _reqPckt.params[PARAM_NUMS_3].param[PARAM_NUMS_0];
	snip1 = _reqPckt.params[PARAM_NUMS_3].param[PARAM_NUMS_1];
	snip2 = _reqPckt.params[PARAM_NUMS_3].param[PARAM_NUMS_2];
	snip3 = _reqPckt.params[PARAM_NUMS_3].param[PARAM_NUMS_3];
	_handySubnet = new IPAddress(snip0, snip1, snip2, snip3);

	result = WiFi.config(*_handyIp, *_handyGateway, *_handySubnet);

	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::setDNS(){
	//TODO: To be tested
	int resp_idx = 2;
	bool result;
	uint8_t validParams = 0;

	validParams = _reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];

	uint8_t dns1ip0, dns1ip1, dns1ip2, dns1ip3,
					dns2ip0, dns2ip1, dns2ip2, dns2ip3;

	//retrieve the dns 1 address
	dns1ip0 = _reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_0];
	dns1ip1 = _reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_1];
	dns1ip2 = _reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_2];
	dns1ip3 = _reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_3];
	IPAddress dns1(dns1ip0, dns1ip1, dns1ip2, dns1ip3);

	//retrieve the dns 2 address
	dns2ip0 = _reqPckt.params[PARAM_NUMS_2].param[PARAM_NUMS_0];
	dns2ip1 = _reqPckt.params[PARAM_NUMS_2].param[PARAM_NUMS_1];
	dns2ip2 = _reqPckt.params[PARAM_NUMS_2].param[PARAM_NUMS_2];
	dns2ip3 = _reqPckt.params[PARAM_NUMS_2].param[PARAM_NUMS_3];
	IPAddress dns2(dns2ip0, dns2ip1, dns2ip2, dns2ip3);

	result = WiFi.config(*_handyIp, *_handyGateway, *_handySubnet, dns1, dns2);

	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::reqHostByName(){
	//TODO to be tested
	int8_t resp_idx = 2;
	int result;
	char host[_reqPckt.params[PARAM_NUMS_0].paramLen];
	//get the host name to look up
	//strncpy(host, _reqPckt.params[0].param, _reqPckt.params[0].paramLen);
	memcpy(host, _reqPckt.params[PARAM_NUMS_0].param, _reqPckt.params[PARAM_NUMS_0].paramLen);
	host[_reqPckt.params[PARAM_NUMS_0].paramLen] = '\0';

	result = WiFi.hostByName(host, _reqHostIp); //retrieve the ip address of the host

	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::getHostByName(){
	//TODO to be tested
	int resp_idx = 2;
	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_4;
	//gets _reqHostIp (obtained before using reqHostByName) and send back to arduino
	_resPckt[resp_idx++] = _reqHostIp.operator[](0);
	_resPckt[resp_idx++] = _reqHostIp.operator[](1);
	_resPckt[resp_idx++] = _reqHostIp.operator[](2);
	_resPckt[resp_idx++] = _reqHostIp.operator[](3);
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::getFwVersion(){

	int resp_idx = 2;
	//send back to arduino the firmware version number
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_5;
	//set fw version
	memcpy(_resPckt+(resp_idx), FW_VERSION, PARAM_SIZE_5);
	resp_idx = resp_idx + PARAM_SIZE_5;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

/* WiFi IPAddress*/
void CommLgc::getNetworkData(){
	//TODO to be tested
	int resp_idx = 2;
	IPAddress localIp, subnetMask, gatewayIp;//, dnsIp1, dnsIp2;

	localIp = WiFi.localIP();
	subnetMask = WiFi.subnetMask();
	gatewayIp = WiFi.gatewayIP();

	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_3;
	_resPckt[resp_idx++] = PARAM_SIZE_4;

	_resPckt[resp_idx++] = localIp.operator[](0);
	_resPckt[resp_idx++] = localIp.operator[](1);
	_resPckt[resp_idx++] = localIp.operator[](2);
	_resPckt[resp_idx++] = localIp.operator[](3);

	_resPckt[resp_idx++] = PARAM_SIZE_4;
	_resPckt[resp_idx++] = subnetMask.operator[](0);
	_resPckt[resp_idx++] = subnetMask.operator[](1);
	_resPckt[resp_idx++] = subnetMask.operator[](2);
	_resPckt[resp_idx++] = subnetMask.operator[](3);

	_resPckt[resp_idx++] = PARAM_SIZE_4;
	_resPckt[resp_idx++] = gatewayIp.operator[](0);
	_resPckt[resp_idx++] = gatewayIp.operator[](1);
	_resPckt[resp_idx++] = gatewayIp.operator[](2);
	_resPckt[resp_idx++] = gatewayIp.operator[](3);
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

/* WiFI Server */
void CommLgc::startServer(){
	//TODO: To be tested
	int resp_idx = 2;
	uint8_t result = 0;
	uint16_t _port = 0;
	int _sock = 0;
	uint8_t _prot = 0;

	//retrieve the port to start server
	uint8_t _p1 = (uint8_t)_reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];
	uint8_t _p2 = (uint8_t)_reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_1];
	_port = (_p1 << 8) + _p2;
	//retrieve sockets number
	_sock = (int)_reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_0];

	//retrieve protocol mode (TCP/UDP)
	_prot = (uint8_t)_reqPckt.params[PARAM_NUMS_2].param[PARAM_NUMS_0];
	if(_sock < MAX_SOCK_NUMBER) {
		if(_prot == 0){ //TCP MODE
			if(mapWiFiServers[_sock] != NULL ){
				//mapWiFiServers[_sock]->stop();
				mapWiFiServers[_sock]->close();
				free(mapWiFiServers[_sock]);
			}
			if(_port == 80){
				//UIserver.stop();			//stop UI SERVER
				UI_alert = true;
			}
			mapWiFiServers[_sock] = new WiFiServer(_port);
			mapWiFiServers[_sock]->begin();
			result = 1;
		} else {	//UDP MODE
			if(mapWiFiUDP[_sock] != NULL ){
				//TODO: stop, close?
			} else {

				mapWiFiUDP[_sock].begin(_port);
				result = 1;
			}
		}
	}
	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::availData(){
	//TODO to be tested
	int resp_idx = 2;
	uint16_t result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.params[0].param[0];

	if(_sock < MAX_SOCK_NUMBER) {
		if(mapWiFiClients[_sock]){ //mapWiFiClients[_sock] != NULL
			result = mapWiFiClients[_sock].available();
		}
		else if(mapWiFiUDP[_sock] != NULL){//non ho un riferimento sul protocollo, quindi uso l'indice di socket
			result = mapWiFiUDP[_sock].parsePacket();
		}
	}

	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_2;
	_resPckt[resp_idx++] = ((uint8_t*)&result)[0];
	// _resPckt[4] = (uint8_t)((result & 0xff00)>>8);//((uint8_t*)&bufferSize)[1];
	// _resPckt[5] = (uint8_t)(result & 0xff);
	_resPckt[resp_idx++] = ((uint8_t*)&result)[1];
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;
	bufferSize = result;
}

void CommLgc::serverStatus(){
	//TODO: To be tested
	int resp_idx = 2;
	uint8_t result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];
  if(mapWiFiServers[_sock] != NULL)
	  result = mapWiFiServers[_sock]->status();
  else
    result =0;
	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;
	//Serial.print("3");

}

void CommLgc::getData(){
	//TODO: To be tested
	int resp_idx = 2;
	uint8_t result = 0;
	uint8_t _sock = 0;
	uint8_t _peek = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];

	//retrieve peek
	_peek = (uint8_t)_reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_0];

	if(mapWiFiClients[_sock]){
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
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

/* WiFi Client */
void CommLgc::stopClient(){
	//TODO to be tested
	int resp_idx = 2;
	uint8_t result = 0;
	uint8_t _sock = 0;
	_sock = (uint8_t)_reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];
	if(_sock < MAX_SOCK_NUMBER){
		if(mapWiFiClients[_sock]){  //!= NULL
			mapWiFiClients[_sock].stop();
			//mapWiFiClients[_sock] = NULL;
			result = 1;
		}
		else if(mapWiFiUDP[_sock] != NULL){
			mapWiFiUDP[_sock].stop();
			result = 1;
		}
	}

	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::clientStatus(){
  //TODO to be tested
	int resp_idx = 2;
  uint8_t result = 0;
  uint8_t _sock = 0; //socket index
  _sock = (uint8_t)_reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];
  if(_sock < MAX_SOCK_NUMBER) {
		if(!mapWiFiClients[_sock]){ //mapWiFiClients[_sock] == NULL
			if(mapWiFiServers[_sock]!= NULL){
        mapWiFiClients[_sock] = mapWiFiServers[_sock]->available(); //Create the client from the server [Arduino as a Server]
        result = mapWiFiClients[_sock].status();
      }
    }else {
      result = mapWiFiClients[_sock].status();
    }
  }

  //set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::sendData(){
	//TODO to be tested
	int resp_idx = 2;
	int result = 0;
	uint8_t _sock = 0; //socket index

	_sock = (uint8_t)_reqPckt.paramsData[PARAM_NUMS_0].data[PARAM_NUMS_0];
	if(mapWiFiClients[_sock]){
		if(mapWiFiClients[_sock].status()== 4) //TODO
			result = mapWiFiClients[_sock].write(_reqPckt.paramsData[PARAM_NUMS_1].data,_reqPckt.paramsData[PARAM_NUMS_1].dataLen);
		if(result == _reqPckt.paramsData[PARAM_NUMS_1].dataLen)
			tcpResult = 1;
		else
			tcpResult = 0;
	}
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = tcpResult; //tcpResult
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::checkDataSent(){

	int resp_idx = 2;
	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = tcpResult;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::startClient(){
	//TODO to be tested
	int resp_idx = 2;
	int result = 0;
	int _sock;
	uint16_t _port;
	uint8_t _prot;

	//retrieve the IP address to connect to
	uint8_t stip1 = _reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];
	uint8_t stip2 = _reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_1];
	uint8_t stip3 = _reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_2];
	uint8_t stip4 = _reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_3];
	IPAddress _ip(stip1, stip2, stip3, stip4);

	//retrieve the port to connect to
	uint8_t _p1 = (uint8_t)_reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_0];
	uint8_t _p2 = (uint8_t)_reqPckt.params[PARAM_NUMS_1].param[PARAM_NUMS_1];
	_port = (_p1 << 8) + _p2;

	//retrieve sockets number
	_sock = (int)_reqPckt.params[PARAM_NUMS_2].param[PARAM_NUMS_0];

	//retrieve protocol mode (TCP/UDP)
	_prot = (uint8_t)_reqPckt.params[PARAM_NUMS_3].param[PARAM_NUMS_0];

	if(_sock < MAX_SOCK_NUMBER) {
		if(_prot == 0){
			//TCP MODE
			if(mapWiFiClients[_sock]){
				WiFiClient wc;
				mapWiFiClients[_sock] = wc;
			}
			result = mapWiFiClients[_sock].connect(_ip, _port);
		} else {
			//UDP MODE
			if(mapWiFiUDP[_sock] == NULL){
				WiFiUDP wu;
				mapWiFiUDP[_sock] = wu;
			}
			result = mapWiFiUDP[_sock].beginPacket(_ip, _port);
		}
	}
	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;
}

/* WiFi UDP Client */
void CommLgc::remoteData(){
	//TODO to be tested
	int resp_idx = 2;
	int _sock;

	//retrieve sockets number
	_sock = (int)_reqPckt.params[PARAM_NUMS_0].param[PARAM_NUMS_0];

	if(_sock < MAX_SOCK_NUMBER && mapWiFiUDP[_sock] != NULL) {
		//set the response struct
		_resPckt[resp_idx++] = PARAM_NUMS_2;
		_resPckt[resp_idx++] = PARAM_SIZE_4;

		IPAddress remoteIp = mapWiFiUDP[_sock].remoteIP();
		_resPckt[resp_idx++] = remoteIp.operator[](0);
		_resPckt[resp_idx++] = remoteIp.operator[](1);
		_resPckt[resp_idx++] = remoteIp.operator[](2);
		_resPckt[resp_idx++] = remoteIp.operator[](3);

		uint16_t remotePort = mapWiFiUDP[_sock].remotePort();
		_resPckt[resp_idx++] = PARAM_SIZE_2;
		_resPckt[resp_idx++] = (uint8_t)((remotePort & 0xff00)>>8);
		_resPckt[resp_idx++] = (uint8_t)(remotePort & 0xff);
		_resPckt[resp_idx++] = END_CMD;
		transfer_size = resp_idx;

	} else {
		createErrorResponse();
	}
}

void CommLgc::getDataBuf(){
	//TODO: To be tested
	int resp_idx = 2;
	int result = 0;
	uint8_t _sock = 0;
	_sock = (uint8_t)_reqPckt.paramsData[PARAM_NUMS_0].data[PARAM_NUMS_0];

	// if(bufferSize>RESPONSE_LENGHT-6)
	// 	bufferSize= RESPONSE_LENGHT-6;																	//fix max length for UDP packet
	// if(bufferSize > 26)
	// 	_resPckt_len = ceil((float)bufferSize/32);
	// else
	// 	_resPckt_len = 0;
	if(_sock < MAX_SOCK_NUMBER){
	  if(mapWiFiUDP[_sock] != NULL){
      char buffer[bufferSize+1]; 										//bufferSize is filled before by availData
  		result = mapWiFiUDP[_sock].read(buffer, bufferSize);
			_resPckt[resp_idx++] = PARAM_NUMS_1;
			_resPckt[resp_idx++] = (uint8_t)((bufferSize & 0xff00)>>8);//((uint8_t*)&bufferSize)[1];
			_resPckt[resp_idx++] = (uint8_t)(bufferSize & 0xff);
			memcpy(_resPckt+resp_idx,buffer,bufferSize);
			resp_idx = resp_idx + bufferSize;
			_resPckt[resp_idx++] = END_CMD;
			transfer_size = resp_idx;

	  }
    else if(mapWiFiClients[_sock]){

      uint8_t buffer_tcp[bufferSize+1];
      result = mapWiFiClients[_sock].read(buffer_tcp, bufferSize);
			//TODO need to add a buffer
      _resPckt[resp_idx++] = PARAM_NUMS_1;
			_resPckt[resp_idx++] = (uint8_t)((bufferSize & 0xff00)>>8);//((uint8_t*)&bufferSize)[1];
			_resPckt[resp_idx++] = (uint8_t)(bufferSize & 0xff);
			memcpy(_resPckt+resp_idx,buffer_tcp,bufferSize);
			resp_idx = resp_idx + bufferSize;
			_resPckt[resp_idx++] = END_CMD;
			transfer_size = resp_idx;

    }
	}
}

void CommLgc::insDataBuf(){
	//TODO: To be tested

	//NOTE maybe can use sendData, it's similar to this except the UDP
	int resp_idx = 2;
	uint8_t result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.paramsData[PARAM_NUMS_0].data[PARAM_NUMS_0];
	if(_sock < MAX_SOCK_NUMBER && mapWiFiUDP[_sock] != NULL){
		mapWiFiUDP[_sock].write(_reqPckt.paramsData[PARAM_NUMS_1].data, _reqPckt.paramsData[PARAM_NUMS_1].dataLen);
		result = 1;
	}

	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

void CommLgc::sendUdpData(){
	//TODO: To be tested
	int resp_idx = 2;
	int result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.paramsData[PARAM_NUMS_0].data[PARAM_NUMS_0];

	if(_sock < MAX_SOCK_NUMBER && mapWiFiUDP[_sock] != NULL){
		//send data to client
		result = mapWiFiUDP[_sock].endPacket();
	}
	//set the response struct
	_resPckt[resp_idx++] = PARAM_NUMS_1;
	_resPckt[resp_idx++] = PARAM_SIZE_1;
	_resPckt[resp_idx++] = result;
	_resPckt[resp_idx++] = END_CMD;
	transfer_size = resp_idx;

}

CommLgc CommunicationLogic;
