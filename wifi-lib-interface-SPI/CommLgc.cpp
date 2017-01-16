//Communication Interface
#include "CommLgc.h"
#include "CommCmd.h"
#include "SPISlave.h"

char FW_VERSION[] = "0.0.1";

//cached values
IPAddress _reqHostIp;

IPAddress* _handyIp;
IPAddress* _handySubnet;
IPAddress* _handyGateway;

uint8_t tcpResult = 0;											//used by availData function
int bufferSize = 0;													//used by getDataBuf function
uint8_t numNets;														//number of networks scanned

//WiFiServer, WiFiClient and WiFiUDP map
WiFiServer* mapWiFiServers[MAX_SOCK_NUMBER];
WiFiClient mapWiFiClients[MAX_SOCK_NUMBER];
WiFiUDP mapWiFiUDP[MAX_SOCK_NUMBER];

//TODO the following const must be setted elsewhere
#define END_CMD 0xEE
#define START_CMD 0xE0
#define RESPONSE_LENGHT 256

tMsgPacket _reqPckt;                          //initialize struct to receive a command from MCU
String raw_pckt_spi ="";  										//packet received from spi master
char _resPckt[RESPONSE_LENGHT];														//response array
int _resPckt_len = 0;													//size of array response (length/32)
CommLgc* This;

WiFiClient client;

#define Serial1 Serial
CommLgc::CommLgc(){
}

/** Logic Functions **/

void CommLgc::begin(){

	Serial.begin(115200);
	Serial.println("init SPI");
	initSPISlave();

}

void CommLgc::handle(){

	if(processing){
		process();    																	//process commands received
		SPISlave.setData(_resPckt);                     //send response to MCU
		digitalWrite(SlaveReadyPin,HIGH);
		if(_resPckt_len > 0){														//response length greater than 32 bytes
			for(int i=1;i<_resPckt_len;i++){
				while(!req_send){
					delayMicroseconds(100);										//wait master
				};
				SPISlave.setData(_resPckt+(i*32));					//split response
				digitalWrite(SlaveReadyPin,HIGH);
				req_send = false;
			}
			_resPckt_len=0;
		}
		processing = false;
	}

}

void CommLgc::DEBUG_MEM() {
	Serial1.print("-- Free Memory: ");
	Serial1.print(ESP.getFreeHeap());
	Serial1.println(" --");
}

int CommLgc::createPacketFromSPI(){
	//TODO parse the message and create the packet
      int idx = 0;
      unsigned char tmp;

      //Start Command
      if(raw_pckt_spi[idx] != START_CMD){
        //Error
        return -1;
      }
        _reqPckt.cmd = raw_pckt_spi[idx];
        //The command
        _reqPckt.tcmd = raw_pckt_spi[++idx];
        //The number of parameters for the command
        tmp = raw_pckt_spi[++idx];
        _reqPckt.nParam = tmp;
        //Get each parameter
        for(int a=0; a<(int)_reqPckt.nParam; a++){
          //Length of the parameter
          if( _reqPckt.tcmd >= 0x40 && _reqPckt.tcmd < 0x50 ){ //16bit tParam
            tmp = (uint16_t)((raw_pckt_spi[++idx] << 8) + (uint8_t)raw_pckt_spi[++idx]);
            _reqPckt.paramsData[a].dataLen = tmp;
            for(int b=0; b<(int)_reqPckt.paramsData[a].dataLen; b++){
              tmp = raw_pckt_spi[++idx];
              _reqPckt.paramsData[a].data[b] = (char)tmp;
            }
          }else{ //8bit tParamData
            tmp = raw_pckt_spi[++idx];
            _reqPckt.params[a].paramLen = tmp;
            for(int b=0; b<(int)_reqPckt.params[a].paramLen; b++){
              tmp = raw_pckt_spi[++idx];
              _reqPckt.params[a].param[b] = (char)tmp;

            }
          }
        }
      //OK
      raw_pckt_spi ="";
			return 0;
}

void CommLgc::initSPISlave(){

    pinMode(SlaveReadyPin,OUTPUT);
    digitalWrite(SlaveReadyPin,LOW);
		This = this;
    SPISlave.onData([](uint8_t * data, size_t len) {
       digitalWrite(SlaveReadyPin,LOW);
       for(int i=0;i<len;i++){
           raw_pckt_spi += (char)data[i];
        }
    });

    SPISlave.onStatus([](uint32_t data) {
				if(data ==1){
					//digitalWrite(SlaveReadyPin,LOW);									//Slave ready command
	        This->createPacketFromSPI();                 			//parse the command received
	        memset(_resPckt,0,sizeof(_resPckt));    					//reset response array
					This->processing = true;
				}
				else if(data==2){
					digitalWrite(SlaveReadyPin,LOW);
					This->req_send = true;
				}
        else
          Serial.println("error");
    });

    // Setup SPI Slave registers and pins
    SPISlave.begin();

}

void CommLgc::createErrorResponse(){

	_resPckt[0] = ERR_CMD;
	_resPckt[1] = 0;
	_resPckt[2] = END_CMD;

}

void CommLgc::process(){

	if (	(_reqPckt.cmd == START_CMD) &&
				((_reqPckt.tcmd & REPLY_FLAG) == 0) ){

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
			//case GET_CLIENT_TCP_CMD:		clientConnected(); break;
			//case PARSE_UDP_PCK:					insDataBuf(_reqPckt, _resPckt);		break;
			default:										createErrorResponse(); 		break;
		}
	}
}

/* Commands Functions */
/* WiFi Base */
void CommLgc::getRSSI(uint8_t current){
	//TODO: To be tested
	int32_t result;

	//retrieve RSSI
	if(current == 1){
		result = WiFi.RSSI();
	}
	else{
		uint8_t idx = _reqPckt.params[0].param[0];

		// NOTE: only for test this function
		// user must call scan network before
		//int num = WiFi.scanNetworks();
		result = WiFi.RSSI(idx);
	}

	//Response contains 1 param with length 4
	_resPckt[2] = 1;
	_resPckt[3] = 4;
	_resPckt[4] = result;
	_resPckt[5] = 0xFF;
	_resPckt[6] = 0xFF;
	_resPckt[7] = 0xFF;
	_resPckt[8] = END_CMD;


}

void CommLgc::getCurrentSSID(){
	//TODO: To be tested

	//retrieve SSID of the current network
	int idx = 2;
	String result = WiFi.SSID();

	//set the response struct
	_resPckt[idx++] = 1;
	_resPckt[idx++] = result.length();
	for(int i=0; i< result.length(); i++){ //char *
		_resPckt[idx++] = result[i];
	}
	_resPckt[idx] = END_CMD;

}

void CommLgc::getEncryption(uint8_t current){
	//TODO: To be tested

	uint8_t result = 0;
	uint8_t idx = 0;

	if(current == 1){
		//uint8_t numNets = WiFi.scanNetworks();	//get networks numbers
		String currSSID = WiFi.SSID();					//get current SSID
		for(int i=0; i<numNets; i++){
			if(currSSID == WiFi.SSID(i)){
				idx = i;	//get the index of the current network
				break;
			}
		}
	}
	else{
		idx = _reqPckt.params[0].param[0];
	}

	result = WiFi.encryptionType(idx);
	//if result
	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;
}

void CommLgc::getMacAddress(){
	//TODO: To be tested

	int paramLen = 6;
	int idx = 2;
	uint8_t mac[paramLen];

	//Retrive mac address
	WiFi.macAddress(mac);

	//set the response struct
	_resPckt[idx++] = 1;
	_resPckt[idx++] = paramLen;
	for (int i=0, j=paramLen-1; i<paramLen, j>=0; i++, j--){
		_resPckt[idx++]=mac[j];
	}
	_resPckt[idx] = END_CMD;

}

void CommLgc::disconnect(){
	//TODO: To be tested
	bool result;

	//Disconnet from the network
	result = WiFi.disconnect();

	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}

void CommLgc::getStatus(){
	//TODO: To be tested
	uint8_t result;

	//Disconnet from the network
	result = WiFi.status();

	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;
}

void CommLgc::begin(uint8_t idx){
	//TODO: To be tested
	uint8_t result;

	if(idx == 0){ // idx==0 - SET_NET_CMD
			//retrieve parameters
			char ssid[_reqPckt.params[0].paramLen+1];
			strncpy(ssid, _reqPckt.params[0].param, _reqPckt.params[0].paramLen);
			ssid[_reqPckt.params[0].paramLen] = '\0';

			//set network and retrieve result
			result = WiFi.begin(ssid);
		}
	else{ // idx ==1 - SET_PASSPHRASE_CMD
			//retrieve parameters
			char ssid[_reqPckt.params[0].paramLen+1];
			char pass[_reqPckt.params[1].paramLen+1];
			strncpy(ssid, _reqPckt.params[0].param, _reqPckt.params[0].paramLen);
			strncpy(pass, _reqPckt.params[1].param, _reqPckt.params[1].paramLen);
			ssid[_reqPckt.params[0].paramLen] = '\0';
			pass[_reqPckt.params[1].paramLen] = '\0';

			//set network and retrieve result
			result = WiFi.begin(ssid, pass);
	}

	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}

void CommLgc::startScanNetwork(){
	//TODO: To be tested

	// Fake response

	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = 1;
	_resPckt[5] = END_CMD;


}

void CommLgc::scanNetwork(){
	//TODO: To be tested
	uint8_t idx = 2;
	numNets = WiFi.scanNetworks();
	//

	_resPckt[idx++] = (numNets <= MAX_PARAMS) ? numNets : MAX_PARAMS;
	for (int i=0; i<(int)_resPckt[2]; i++)
	{
		String ssidNet = WiFi.SSID(i).c_str();
		_resPckt[idx++] = ssidNet.length() /* + 1*/;
		//memcpy(_resPckt+5,buffer_tcp,bufferSize);
		for(int j=0; j<ssidNet.length(); j++){
			_resPckt[idx++] = ssidNet[j];
		}
	}
	if(idx > 31)
		_resPckt_len = ceil((float)idx/32);
	else
		_resPckt_len = 0;

	_resPckt[idx] = END_CMD;
}

void CommLgc::getBSSID(uint8_t current){
	//TODO: To be tested
	int paramLen = 6;
	uint8_t idx = 2;
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
	_resPckt[idx++] = 1;
	_resPckt[idx++] = paramLen;
	for (int j=paramLen-1; j>=0; j--){
		_resPckt[idx++] = result[j];
	}
	_resPckt[idx] = END_CMD;

}

void CommLgc::config(){
	/*
	WiFi.config call arduino side: WiFi.config(local_ip, gateway, subnet, dns1, dns2)
	*/

	//TODO: To be tested
	bool result;
	uint8_t validParams = 0;

	uint8_t stip0, stip1, stip2, stip3,
					gwip0, gwip1, gwip2, gwip3,
					snip0, snip1, snip2, snip3;

	validParams = _reqPckt.params[0].param[0];

	//retrieve the static IP address
	stip0 = _reqPckt.params[1].param[0];
	stip1 = _reqPckt.params[1].param[1];
	stip2 = _reqPckt.params[1].param[2];
	stip3 = _reqPckt.params[1].param[3];
	_handyIp = new IPAddress(stip0, stip1, stip2, stip3);


	//retrieve the gateway IP address
	gwip0 = _reqPckt.params[2].param[0];
	gwip1 = _reqPckt.params[2].param[1];
	gwip2 = _reqPckt.params[2].param[2];
	gwip3 = _reqPckt.params[2].param[3];
	_handyGateway = new IPAddress(gwip0, gwip1, gwip2, gwip3);

	//retrieve the subnet mask
	snip0 = _reqPckt.params[3].param[0];
	snip1 = _reqPckt.params[3].param[1];
	snip2 = _reqPckt.params[3].param[2];
	snip3 = _reqPckt.params[3].param[3];
	_handySubnet = new IPAddress(snip0, snip1, snip2, snip3);

	result = WiFi.config(*_handyIp, *_handyGateway, *_handySubnet);

	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}

void CommLgc::setDNS(){
	//TODO: To be tested
	bool result;
	uint8_t validParams = 0;

	validParams = _reqPckt.params[0].param[0];

	uint8_t dns1ip0, dns1ip1, dns1ip2, dns1ip3,
					dns2ip0, dns2ip1, dns2ip2, dns2ip3;

	//retrieve the dns 1 address
	dns1ip0 = _reqPckt.params[1].param[0];
	dns1ip1 = _reqPckt.params[1].param[1];
	dns1ip2 = _reqPckt.params[1].param[2];
	dns1ip3 = _reqPckt.params[1].param[3];
	IPAddress dns1(dns1ip0, dns1ip1, dns1ip2, dns1ip3);

	//retrieve the dns 2 address
	dns2ip0 = _reqPckt.params[2].param[0];
	dns2ip1 = _reqPckt.params[2].param[1];
	dns2ip2 = _reqPckt.params[2].param[2];
	dns2ip3 = _reqPckt.params[2].param[3];
	IPAddress dns2(dns2ip0, dns2ip1, dns2ip2, dns2ip3);

	result = WiFi.config(*_handyIp, *_handyGateway, *_handySubnet, dns1, dns2);

	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;
}

void CommLgc::reqHostByName(){
	//TODO to be tested

	char host[_reqPckt.params[0].paramLen];
	int result;

	//get the host name to look up
	strncpy(host, _reqPckt.params[0].param, _reqPckt.params[0].paramLen);
	host[_reqPckt.params[0].paramLen] = '\0';

	result = WiFi.hostByName(host, _reqHostIp); //retrieve the ip address of the host

	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}

void CommLgc::getHostByName(){
	//TODO to be tested

	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 4;
	//gets _reqHostIp (obtained before using reqHostByName) and send back to arduino
	_resPckt[4] = _reqHostIp.operator[](0);
	_resPckt[5] = _reqHostIp.operator[](1);
	_resPckt[6] = _reqHostIp.operator[](2);
	_resPckt[7] = _reqHostIp.operator[](3);
	_resPckt[8] = END_CMD;

}

void CommLgc::getFwVersion(){
	//TODO to be tested

	//send back to arduino the firmware version number
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = 3;  //TODO
	_resPckt[5] = END_CMD;

}

/* WiFi IPAddress*/
void CommLgc::getNetworkData(){
	//TODO to be tested

	IPAddress localIp, subnetMask, gatewayIp;//, dnsIp1, dnsIp2;

	localIp = WiFi.localIP();
	subnetMask = WiFi.subnetMask();
	gatewayIp = WiFi.gatewayIP();

	//set the response struct
	_resPckt[2] = 3;
	_resPckt[3] = 4;

	_resPckt[4] = localIp.operator[](0);
	_resPckt[5] = localIp.operator[](1);
	_resPckt[6] = localIp.operator[](2);
	_resPckt[7] = localIp.operator[](3);

	_resPckt[8] = 4;
	_resPckt[9] = subnetMask.operator[](0);
	_resPckt[10] = subnetMask.operator[](1);
	_resPckt[11] = subnetMask.operator[](2);
	_resPckt[12] = subnetMask.operator[](3);

	_resPckt[13] = 4;
	_resPckt[14] = gatewayIp.operator[](0);
	_resPckt[15] = gatewayIp.operator[](1);
	_resPckt[16] = gatewayIp.operator[](2);
	_resPckt[17] = gatewayIp.operator[](3);
	_resPckt[18] = END_CMD;

}

/* WiFI Server */
void CommLgc::startServer(){
	//TODO: To be tested
	uint8_t result = 0;
	uint16_t _port = 0;
	int _sock = 0;
	uint8_t _prot = 0;

	//retrieve the port to start server
	uint8_t _p1 = (uint8_t)_reqPckt.params[0].param[0];
	uint8_t _p2 = (uint8_t)_reqPckt.params[0].param[1];
	_port = (_p1 << 8) + _p2;

	//retrieve sockets number
	_sock = (int)_reqPckt.params[1].param[0];

	//retrieve protocol mode (TCP/UDP)
	_prot = (uint8_t)_reqPckt.params[2].param[0];
	if(_sock < MAX_SOCK_NUMBER) {
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
				//TODO: stop, close?
			} else {

				mapWiFiUDP[_sock].begin(_port);
				result = 1;
			}
		}
	}
	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}

void CommLgc::availData(){
	//TODO to be tested
	uint16_t result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.params[0].param[0];

	if(_sock < MAX_SOCK_NUMBER) {
		if(mapWiFiClients[_sock] != NULL ){
			result = mapWiFiClients[_sock].available();
		}
		else if(mapWiFiUDP[_sock] != NULL){//non ho un riferimento sul protocollo, quindi uso l'indice di socket
			result = mapWiFiUDP[_sock].parsePacket();
		}
	}

	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 2;
	_resPckt[4] = ((uint8_t*)&result)[0];
	_resPckt[5] = ((uint8_t*)&result)[1];
	_resPckt[6] = END_CMD;

	bufferSize = result;
}

void CommLgc::serverStatus(){
	//TODO: To be tested
	uint8_t result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.params[0].param[0];
	//Serial.print("1");
  if(mapWiFiServers[_sock] != NULL)
	  result = mapWiFiServers[_sock]->status();
  else
    result =0;
	//Serial.print("2");
	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;
	//Serial.print("3");

}

void CommLgc::getData(){
	//TODO: To be tested
	int result = 0;
	uint8_t _sock = 0;
	uint8_t _peek = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.params[0].param[0];

	//retrieve peek
	_peek = (uint8_t)_reqPckt.params[1].param[0];

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
  //Serial.println(result);
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}

/* WiFi Client */
void CommLgc::stopClient(){
	//TODO to be tested
	uint8_t result = 0;
	uint8_t _sock = 0;
	_sock = (uint8_t)_reqPckt.params[0].param[0];
	if(_sock < MAX_SOCK_NUMBER){
		if(mapWiFiClients[_sock] != NULL ){
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
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}

/*void CommLgc::clientConnected(){
  //TODO to be tested
  uint8_t result = 0;
  uint8_t _sock = 0; //socket index
  _sock = (uint8_t)_reqPckt.params[0].param[0];
  if(_sock < MAX_SOCK_NUMBER) {
		if(mapWiFiClients[_sock] != NULL){
		// 	if(mapWiFiServers[_sock] != NULL){
    //     mapWiFiClients[_sock] = mapWiFiServers[_sock]->available(); //Create the client from the server [Arduino as a Server]
    //     result = mapWiFiClients[_sock].status();
		// 		Serial.println("1");
    //   }
    // }else {
      result = mapWiFiClients[_sock].connected();
			Serial.println("2");
    }
		else
			result = 0;
  }

  //set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}*/

void CommLgc::clientStatus(){
  //TODO to be tested
  uint8_t result = 0;
  uint8_t _sock = 0; //socket index
  _sock = (uint8_t)_reqPckt.params[0].param[0];
	//Serial.println(_sock);
  if(_sock < MAX_SOCK_NUMBER) {
		if(mapWiFiClients[_sock] == NULL){
			if(mapWiFiServers[_sock] != NULL){
        mapWiFiClients[_sock] = mapWiFiServers[_sock]->available(); //Create the client from the server [Arduino as a Server]
        result = mapWiFiClients[_sock].status();
				//Serial.println("1");
      }
    }else {
      result = mapWiFiClients[_sock].status();
			//Serial.println(_sock);
			//Serial.println(result);
    }
  }

  //set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}

void CommLgc::sendData(){
	//TODO to be tested

	int result = 0;
	uint8_t _sock = 0; //socket index

	_sock = (uint8_t)_reqPckt.paramsData[0].data[0];
	if(mapWiFiClients[_sock] != NULL){
		result = mapWiFiClients[_sock].write(_reqPckt.paramsData[1].data, _reqPckt.paramsData[1].dataLen);
		if(result == _reqPckt.paramsData[1].dataLen)
			tcpResult = 1;
		else
			tcpResult = 0;
	}
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = tcpResult;
	_resPckt[5] = END_CMD;

}

void CommLgc::checkDataSent(){
	//TODO to be tested

	//set the response struct
     _resPckt[2] = 1;
     _resPckt[3] = 1;
     _resPckt[4] = tcpResult;
     _resPckt[5] = END_CMD;
}

void CommLgc::startClient(){
	//TODO to be tested
	int result = 0;
	int _sock;
	uint16_t _port;
	uint8_t _prot;

	//retrieve the IP address to connect to
	uint8_t stip1 = _reqPckt.params[0].param[0];
	uint8_t stip2 = _reqPckt.params[0].param[1];
	uint8_t stip3 = _reqPckt.params[0].param[2];
	uint8_t stip4 = _reqPckt.params[0].param[3];
	IPAddress _ip(stip1, stip2, stip3, stip4);

	//retrieve the port to connect to
	uint8_t _p1 = (uint8_t)_reqPckt.params[1].param[0];
	uint8_t _p2 = (uint8_t)_reqPckt.params[1].param[1];
	_port = (_p1 << 8) + _p2;

	//retrieve sockets number
	_sock = (int)_reqPckt.params[2].param[0];

	//retrieve protocol mode (TCP/UDP)
	_prot = (uint8_t)_reqPckt.params[3].param[0];

	if(_sock < MAX_SOCK_NUMBER) {
		if(_prot == 0){
			//TCP MODE
			if(mapWiFiClients[_sock] == NULL){
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
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;
}

/* WiFi UDP Client */
void CommLgc::remoteData(){
	//TODO to be tested
	int _sock;

	//retrieve sockets number
	_sock = (int)_reqPckt.params[0].param[0];

	if(_sock < MAX_SOCK_NUMBER && mapWiFiUDP[_sock] != NULL) {
		//set the response struct
		_resPckt[2] = 2;
		_resPckt[3] = 4;

		IPAddress remoteIp = mapWiFiUDP[_sock].remoteIP();
		_resPckt[4] = remoteIp.operator[](0);
		_resPckt[5] = remoteIp.operator[](1);
		_resPckt[6] = remoteIp.operator[](2);
		_resPckt[7] = remoteIp.operator[](3);

		uint16_t remotePort = mapWiFiUDP[_sock].remotePort();
		_resPckt[8] = 2;
		_resPckt[9] = (uint8_t)((remotePort & 0xff00)>>8);
		_resPckt[10] = (uint8_t)(remotePort & 0xff);
		_resPckt[11] = END_CMD;

	} else {
		createErrorResponse();
	}
}

void CommLgc::getDataBuf(){
	//TODO: To be tested

	int result = 0;
	uint8_t _sock = 0;
	_sock = (uint8_t)_reqPckt.paramsData[0].data[0];

	if(bufferSize>RESPONSE_LENGHT-6)
		bufferSize= RESPONSE_LENGHT-6;																	//fix max length for UDP packet
	if(bufferSize > 26)
		_resPckt_len = ceil((float)bufferSize/32);
	else
		_resPckt_len = 0;

	if(_sock < MAX_SOCK_NUMBER){
	  if(mapWiFiUDP[_sock] != NULL){
      char buffer[bufferSize+1]; 										//bufferSize is filled before by availData
  		result = mapWiFiUDP[_sock].read(buffer, bufferSize);
			buffer[bufferSize] = END_CMD;

			_resPckt[2] = 1;
			_resPckt[4] = ((uint8_t*)&bufferSize)[0];
			_resPckt[5] = ((uint8_t*)&bufferSize)[1];
			memcpy(_resPckt+5,buffer,bufferSize);
			//int resp_size = ceil((float)bufferSize/32);			//split the response (256) in array of 32 element

	  }
    else if(mapWiFiClients[_sock] != NULL){

      uint8_t buffer_tcp[bufferSize+1];
      result = mapWiFiClients[_sock].read(buffer_tcp, bufferSize);
			buffer_tcp[bufferSize] = END_CMD;
			//TODO need to add a buffer
      _resPckt[2] = 1;
      _resPckt[3] = ((uint8_t*)&bufferSize)[0];
			_resPckt[4] = ((uint8_t*)&bufferSize)[1];
			memcpy(_resPckt+5,buffer_tcp,bufferSize);
			//int resp_size = ceil((float)bufferSize/32);

    }
	}
}

void CommLgc::insDataBuf(){
	//TODO: To be tested

	//NOTE maybe can use sendData, it's similar to this except the UDP

	uint8_t result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.paramsData[0].data[0];
	if(_sock < MAX_SOCK_NUMBER && mapWiFiUDP[_sock] != NULL){
		mapWiFiUDP[_sock].write(_reqPckt.paramsData[1].data, _reqPckt.paramsData[1].dataLen);
		result = 1;
	}

	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}

void CommLgc::sendUdpData(){
	//TODO: To be tested
	int result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.paramsData[0].data[0];

	if(_sock < MAX_SOCK_NUMBER && mapWiFiUDP[_sock] != NULL){
		//send data to client
		result = mapWiFiUDP[_sock].endPacket();
	}
	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = END_CMD;

}

CommLgc CommunicationLogic;
