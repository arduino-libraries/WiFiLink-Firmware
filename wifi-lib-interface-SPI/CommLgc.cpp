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

uint8_t tcpResult = 0;
int bufferSize = 0;

//WiFiServer, WiFiClient and WiFiUDP map
WiFiServer* mapWiFiServers[MAX_SOCK_NUM];
WiFiClient mapWiFiClients[MAX_SOCK_NUM];
WiFiUDP mapWiFiUDP[MAX_SOCK_NUM];

//TODO the following const must be setted elsewhere
#define END_CMD 0xEE
#define START_CMD 0xE0

tMsgPacket _reqPckt;                             //initialize struct to receive a command from MCU
int BaudRate;
int SlaveSelectPin;
String raw_pckt_spi ="";  //packet received from spi master
char _resPckt[32];
CommLgc* This;
// int status_ready = 0;   //to check the spi trasmission status
bool en = 0;

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
		process();    							//process commands received
		SPISlave.setData(_resPckt);                       //send response to MCU
		digitalWrite(SlaveReadyPin,HIGH);
		processing = false;
	}

}

void CommLgc::DEBUG_MEM() {
	Serial1.print("-- Free Memory: ");
	Serial1.print(ESP.getFreeHeap());
	Serial1.println(" --");
}

//void CommLgc::DEBUG(tMsgPacketStatic *_pckt) {
//
//  Serial1.println("--- Packet  ---");
//  int ix =0;
//  Serial1.println(_pckt->response[ix++], HEX);
//  Serial1.println(_pckt->response[ix++], HEX);
//  Serial1.println(_pckt->response[ix++], HEX);
//  for(int i=0; i<(int)_pckt->response[2]; i++){
//    if(_pckt->tcmd >= 0x40 && _pckt->tcmd < 0x50){ //16 Bit
//      Serial1.println(_pckt->paramsData[i].dataLen, HEX );
//      for(int j=0; j< (int)_pckt->paramsData[i].dataLen; j++)
//        Serial1.println( _pckt->paramsData[i].data[j], HEX);
//    }
//    else{ //8 Bit
//      Serial1.println(_pckt->params[i].paramLen, HEX );
//      for(int j=0; j< (int)_pckt->params[i].paramLen; j++)
//        Serial1.println( _pckt->params[i].param[j], HEX);
//      }
//  }
//  Serial1.println(0xEE, HEX);
//  Serial1.println("--- End Packet ---");
//}

// void CommLgc::DEBUG(tMsgPacket *_pckt) {
//
// 	Serial1.println("--- Packet  ---");
// 	Serial1.println(_pckt->cmd, HEX);
// 	Serial1.println(_pckt->tcmd, HEX);
// 	Serial1.println(_pckt->nParam, HEX);
// 	for(int i=0; i<(int)_pckt->nParam; i++){
// 		if(_pckt->tcmd >= 0x40 && _pckt->tcmd < 0x50){ //16 Bit
// 			Serial1.println(_pckt->paramsData[i].dataLen, HEX );
// 			for(int j=0; j< (int)_pckt->paramsData[i].dataLen; j++)
// 				Serial1.println( _pckt->paramsData[i].data[j], HEX);
// 		}
// 		else{ //8 Bit
// 			Serial1.println(_pckt->params[i].paramLen, HEX );
// 			for(int j=0; j< (int)_pckt->params[i].paramLen; j++)
// 				Serial1.println( _pckt->params[i].param[j], HEX);
// 			}
// 	}
// 	Serial1.println(0xEE, HEX);
// 	Serial1.println("--- End Packet ---");
// }

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

            //_pckt->paramsData[a].data = (char*)malloc(_pckt->paramsData[a].dataLen);
            //Value of the parameter
            for(int b=0; b<(int)_reqPckt.paramsData[a].dataLen; b++){
              tmp = raw_pckt_spi[++idx];
              _reqPckt.paramsData[a].data[b] = (char)tmp;
            }
          }else{ //8bit tParamData
            tmp = raw_pckt_spi[++idx];
            _reqPckt.params[a].paramLen = tmp;
            //_pckt->params[a].param = (char*)malloc(_pckt->params[a].paramLen);
            //Value of the parameter
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
		This = this;
    SPISlave.onData([](uint8_t * data, size_t len) {
       for(int i=0;i<len;i++){
           raw_pckt_spi += (char)data[i];
        }
    });

    SPISlave.onStatus([](uint32_t data) {
				digitalWrite(SlaveReadyPin,LOW);									//Slave ready commmand
        This->createPacketFromSPI();                 //parse the command received
        memset(_resPckt,0,sizeof(_resPckt));    //reset response array
				This->processing = true;
    });

    // Setup SPI Slave registers and pins
    SPISlave.begin();

}

void CommLgc::createErrorResponse(){

	// _pckt->cmd = ERR_CMD;
	// _pckt->nParam = 0;
	_resPckt[0] = ERR_CMD;
	_resPckt[1] = 0;
	_resPckt[2] = 0xEE;
}

//void CommLgc::process(tMsgPacket *_reqPckt, char* _resPckt){

// void CommLgc::process(String _reqPckt, char* _resPckt){
//
// 	if (	(_reqPckt[0] == START_CMD) &&
// 				((_reqPckt[1] & REPLY_FLAG) == 0) ){
//
// 		//_resPckt->cmd = START_CMD;
// 		//_resPckt->tcmd = _reqPckt.tcmd | REPLY_FLAG;
// 		// _resPckt[0] = 0xE0;
// 		// _resPckt[1] = _reqPckt.tcmd | REPLY_FLAG;
// 		_resPckt[0] = 0xE0;
// 		_resPckt[1] = _reqPckt[1] | REPLY_FLAG;
// 		//Serial.println(_reqPckt.tcmd, HEX);
//
// 		switch(_reqPckt[1]){
// 			case SEND_DATA_TCP_CMD:			sendData(_reqPckt, _resPckt);			break;
// 			//case PARSE_UDP_PCK:					insDataBuf(_reqPckt, _resPckt);		break;
// 			default:										createErrorResponse(_resPckt); 		break;
// 		}
// 		_reqPckt = "";
// 	}
// }


void CommLgc::process(){

	if (	(_reqPckt.cmd == START_CMD) &&
				((_reqPckt.tcmd & REPLY_FLAG) == 0) ){

		//_resPckt->cmd = START_CMD;
		//_resPckt->tcmd = _reqPckt.tcmd | REPLY_FLAG;
		// _resPckt[0] = 0xE0;
		// _resPckt[1] = _reqPckt.tcmd | REPLY_FLAG;
		_resPckt[0] = 0xE0;
		_resPckt[1] = _reqPckt.tcmd | REPLY_FLAG;
		//Serial.println(_reqPckt.tcmd, HEX);

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
			//case PARSE_UDP_PCK:					insDataBuf(_reqPckt, _resPckt);		break;
			default:										createErrorResponse(); 		break;
		}
	}
}


/*void CommLgc::process(tMsgPacket *_reqPckt, char* _resPckt){

  if (  (_reqPckt.cmd == START_CMD) &&
        ((_reqPckt.tcmd & REPLY_FLAG) == 0) ){

   _resPckt[0] = 0xE0;
   _resPckt[1] = _reqPckt.tcmd | REPLY_FLAG;
		//Serial.println(_reqPckt.tcmd, HEX);
    switch(_reqPckt.tcmd){
//      case SET_NET_CMD:           begin(_reqPckt, _resPckt, 0);     break;
//      case SET_PASSPHRASE_CMD:    begin(_reqPckt, _resPckt, 1);     break;
//      case SET_IP_CONFIG_CMD:     config(_reqPckt, _resPckt);       break;
//      case SET_DNS_CONFIG_CMD:    setDNS(_reqPckt, _resPckt);       break;
//      case GET_CONN_STATUS_CMD:   getStatus(_reqPckt, _resPckt);    break;
//      case GET_IPADDR_CMD:    getNetworkData(_reqPckt, _resPckt);   break;
//      case GET_MACADDR_CMD:   getMacAddress(_reqPckt, _resPckt);    break;
//      case GET_CURR_SSID_CMD: getCurrentSSID(_reqPckt, _resPckt);   break;
//      case GET_CURR_BSSID_CMD:getBSSID(_reqPckt, _resPckt, 1);      break;
//      case GET_CURR_RSSI_CMD: getRSSI(_reqPckt, _resPckt, 1);       break;
//      case GET_CURR_ENCT_CMD: getEncryption(_reqPckt, _resPckt, 1); break;
//      case SCAN_NETWORKS:     scanNetwork(_reqPckt, _resPckt);      break;
//      case START_SERVER_TCP_CMD:  startServer(_reqPckt, _resPckt);  break;
//      case GET_STATE_TCP_CMD:     serverStatus(_reqPckt, _resPckt); break;
//      case DATA_SENT_TCP_CMD:     checkDataSent(_reqPckt, _resPckt);break;
//    	case AVAIL_DATA_TCP_CMD:    availData(_reqPckt, _resPckt);    break;
//      case GET_DATA_TCP_CMD:      getData(_reqPckt, _resPckt);      break;
//      case START_CLIENT_TCP_CMD:  startClient(_reqPckt, _resPckt);      break;
//      case STOP_CLIENT_TCP_CMD:   stopClient(_reqPckt, _resPckt);       break;
//      case GET_CLIENT_STATE_TCP_CMD:  clientStatus(_reqPckt, _resPckt); break;
//      case DISCONNECT_CMD:        disconnect(_reqPckt, _resPckt);       break;
//      case GET_IDX_RSSI_CMD:      getRSSI(_reqPckt, _resPckt, 0);       break;
//      case GET_IDX_ENCT_CMD:      getEncryption(_reqPckt, _resPckt, 0); break;
//      case REQ_HOST_BY_NAME_CMD:  reqHostByName(_reqPckt, _resPckt);    break;
//      case GET_HOST_BY_NAME_CMD:  getHostByName(_reqPckt, _resPckt);    break;
//      case GET_FW_VERSION_CMD:    getFwVersion(_reqPckt, _resPckt);     break;
//      case START_SCAN_NETWORKS:   startScanNetwork(_reqPckt, _resPckt); break;
//      case SEND_DATA_UDP_CMD:     sendUdpData(_reqPckt, _resPckt);  break;
//      case GET_REMOTE_DATA_CMD:   remoteData(_reqPckt, _resPckt);   break;
//       case SEND_DATA_TCP_CMD:     sendData(_reqPckt, _resPckt);     break;
//      case GET_DATABUF_TCP_CMD:   getDataBuf(_reqPckt, _resPckt);   break;
//      case INSERT_DATABUF_CMD:    insDataBuf(_reqPckt, _resPckt);   break;
      //case PARSE_UDP_PCK:         insDataBuf(_reqPckt, _resPckt);   break;
      //default:                    createErrorResponse(response);    break;
    }
  }
}*/



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
	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 4;

	//_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = result;
	// _resPckt->params[0].param[1] = 0xFF;
	// _resPckt->params[0].param[2] = 0xFF;
	// _resPckt->params[0].param[3] = 0xFF;
	_resPckt[2] = 1;
	_resPckt[3] = 4;
	_resPckt[4] = result;
	_resPckt[5] = 0xFF;
	_resPckt[6] = 0xFF;
	_resPckt[7] = 0xFF;
	_resPckt[8] = 0xEE;


}

void CommLgc::getCurrentSSID(){
	//TODO: To be tested

	//retrieve SSID of the current network
	int idx = 2;
	String result = WiFi.SSID();

	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = result.length();
	_resPckt[idx++] = 1;
	_resPckt[idx++] = result.length();

	//_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	for(int i=0; i< result.length(); i++){ //char *
		//_resPckt->params[0].param[i] = result[i];
		//idx++;
		_resPckt[idx++] = result[i];
		// Serial.print(idx);
		// Serial.print(_resPckt[idx]);
	}
	_resPckt[idx] = 0xEE;
	//Serial.print(idx);


}

void CommLgc::getEncryption(uint8_t current){
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
		idx = _reqPckt.params[0].param[0];
	}

	result = WiFi.encryptionType(idx);

	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 1;
	//
	// //_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = result;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;
}

void CommLgc::getMacAddress(){
	//TODO: To be tested

	int paramLen = 6;
	int idx = 2;
	uint8_t mac[paramLen];

	//Retrive mac address
	WiFi.macAddress(mac);

	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = paramLen;
	_resPckt[idx++] = 1;
	_resPckt[idx++] = paramLen;

	//_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	for (int i=0, j=paramLen-1; i<paramLen, j>=0; i++, j--){
		//_resPckt->params[0].param[i] = mac[j];
		//idx++;
		_resPckt[idx++]=mac[j];

	}
	_resPckt[idx] = 0xEE;
}

void CommLgc::disconnect(){
	//TODO: To be tested
	bool result;

	//Disconnet from the network
	result = WiFi.disconnect();
  //ESP.restart();			//TODO check
	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 1;
	//
	// //_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = result;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;

}

void CommLgc::getStatus(){
	//TODO: To be tested
	uint8_t result;

	//Disconnet from the network
	result = WiFi.status();

	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 1;
	//
	// //_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = result;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;
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

	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 1;
	//
	// //_resPckt->params[0].param = (char*) malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = result;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;

}

void CommLgc::startScanNetwork(){
	//TODO: To be tested

	// Fake response
	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 1;
	//
	// //_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = 1;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = 1;
	_resPckt[5] = 0xEE;


}

void CommLgc::scanNetwork(){
	//TODO: To be tested
	// uint8_t numNets = WiFi.scanNetworks();
	//
	// _resPckt->nParam = (numNets <= MAX_PARAMS) ? numNets : MAX_PARAMS;
	//
	// for (int i=0; i<(int)_resPckt->nParam; i++)
	// {
	// 	String ssidNet = WiFi.SSID(i).c_str();
	// 	_resPckt->params[i].paramLen = ssidNet.length() /* + 1*/;
	// 	//_resPckt->params[i].param = (char*)malloc( ssidNet.length() /* + 1 */);
	//
	// 	for(int j=0; j<ssidNet.length(); j++){
	// 		_resPckt->params[i].param[j] = ssidNet[j];
	// 	}
	// }
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
	//String result = WiFi.BSSIDstr();
	//Serial.print(result);
	//_resPckt->nParam = 1;
	//_resPckt->params[0].paramLen = paramLen;
	//Serial.println("1");
	_resPckt[idx++] = 1;
	//Serial.println("2");
	_resPckt[idx++] = paramLen;
	//Serial.println("3");
	//_resPckt[4] = result;

	//_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	//for (int i=0, j=paramLen-1; i<paramLen, j>=0; i++, j--){
	for (int j=paramLen-1; j>=0; j--){
		_resPckt[idx++] = result[j];
		//Serial.print(result[j],HEX);
	}
	//Serial.println("4");
	_resPckt[idx] = 0xEE;
	//Serial.println("5");
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

	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 1;
	// //_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = result;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;

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

	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 1;
	// //_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = result;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;
}

void CommLgc::reqHostByName(){
	//TODO to be tested

	char host[_reqPckt.params[0].paramLen];
	int result;

	//get the host name to look up
	strncpy(host, _reqPckt.params[0].param, _reqPckt.params[0].paramLen);
	host[_reqPckt.params[0].paramLen] = '\0';
	//Serial.println("1");
	result = WiFi.hostByName(host, _reqHostIp); //retrieve the ip address of the host
	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 1;
	// //_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = result;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;

}

void CommLgc::getHostByName(){
	//TODO to be tested

	//gets _reqHostIp (obtained before using reqHostByName) and send back to arduino

	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 4;
	// //_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = _reqHostIp.operator[](0);
	// _resPckt->params[0].param[1] = _reqHostIp.operator[](1);
	// _resPckt->params[0].param[2] = _reqHostIp.operator[](2);
	// _resPckt->params[0].param[3] = _reqHostIp.operator[](3);
	_resPckt[2] = 1;
	_resPckt[3] = 4;
	_resPckt[4] = _reqHostIp.operator[](0);
	_resPckt[5] = _reqHostIp.operator[](1);
	_resPckt[6] = _reqHostIp.operator[](2);
	_resPckt[7] = _reqHostIp.operator[](3);
	_resPckt[8] = 0xEE;

}

void CommLgc::getFwVersion(){
	//TODO to be tested

	//send back to arduino the firmware version number

	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = sizeof(FW_VERSION)-1;
	// //_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// strncpy(_resPckt->params[0].param, FW_VERSION, _resPckt->params[0].paramLen) ;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = 3;  //TODO
	_resPckt[5] = 0xEE;

}

/* WiFi IPAddress*/
void CommLgc::getNetworkData(){
	//TODO to be tested

	IPAddress localIp, subnetMask, gatewayIp;//, dnsIp1, dnsIp2;

	localIp = WiFi.localIP();
	subnetMask = WiFi.subnetMask();
	gatewayIp = WiFi.gatewayIP();
	// dnsIp1 = WiFi.dnsIP(0); //Ready to have even DNS 1 and DNS 2
	// dnsIp2 = WiFi.dnsIP(1);

	//_resPckt->nParam = 3;
	//_resPckt->params[0].paramLen = 4;
	//_resPckt->params[1].paramLen = 4;
	//_resPckt->params[2].paramLen = 4;
	_resPckt[2] = 3;
	_resPckt[3] = 4;
	//_resPckt[4] = 3;  //TODO
	//_resPckt[5] = 0xEE;

	//_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = localIp.operator[](0);
	// _resPckt->params[0].param[1] = localIp.operator[](1);
	// _resPckt->params[0].param[2] = localIp.operator[](2);
	// _resPckt->params[0].param[3] = localIp.operator[](3);
	_resPckt[4] = localIp.operator[](0);
	_resPckt[5] = localIp.operator[](1);
	_resPckt[6] = localIp.operator[](2);
	_resPckt[7] = localIp.operator[](3);

	//_resPckt->params[1].param = (char*)malloc(_resPckt->params[1].paramLen);
	_resPckt[8] = 4;
	// _resPckt->params[1].param[0] = subnetMask.operator[](0);
	// _resPckt->params[1].param[1] = subnetMask.operator[](1);
	// _resPckt->params[1].param[2] = subnetMask.operator[](2);
	// _resPckt->params[1].param[3] = subnetMask.operator[](3);
	_resPckt[9] = subnetMask.operator[](0);
	_resPckt[10] = subnetMask.operator[](1);
	_resPckt[11] = subnetMask.operator[](2);
	_resPckt[12] = subnetMask.operator[](3);


	//_resPckt->params[2].param = (char*)malloc(_resPckt->params[2].paramLen);
	// _resPckt->params[2].param[0] = gatewayIp.operator[](0);
	// _resPckt->params[2].param[1] = gatewayIp.operator[](1);
	// _resPckt->params[2].param[2] = gatewayIp.operator[](2);
	// _resPckt->params[2].param[3] = gatewayIp.operator[](3);
	_resPckt[13] = 4;
	_resPckt[14] = gatewayIp.operator[](0);
	_resPckt[15] = gatewayIp.operator[](1);
	_resPckt[16] = gatewayIp.operator[](2);
	_resPckt[17] = gatewayIp.operator[](3);
	_resPckt[18] = 0xEE;

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
  // _resPckt[2] = 1;
  // _resPckt[3] = 2;
  // _resPckt[4] = ((uint8_t*)&result)[0];
  // _resPckt[5] = ((uint8_t*)&result)[1];
  // _resPckt[6] = 0xEE;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;

}

void CommLgc::availData(){
	//TODO to be tested
	uint16_t result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.params[0].param[0];

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
 //Serial.println(result);
	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 2;
	_resPckt[4] = ((uint8_t*)&result)[0];
	_resPckt[5] = ((uint8_t*)&result)[1];
	_resPckt[6] = 0xEE;

	bufferSize = result;
}

void CommLgc::serverStatus(){
	//TODO: To be tested
	uint8_t result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.params[0].param[0];
  if(mapWiFiServers[_sock] != NULL)
	  result = mapWiFiServers[_sock]->status();
  else
    result =0;

//ANDREA EDIT
//  if(_sock < MAX_SOCK_NUM) {
//    if(mapWiFiClients[_sock] == NULL){
//      if(mapWiFiServers[_sock] != NULL){
//        mapWiFiClients[_sock] = mapWiFiServers[_sock]->available(); //Create the client from the server [Arduino as a Server]
//        result = mapWiFiClients[_sock].status();
//      }
//      else
//        result =0;
//    }
//  }
	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;

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
	_resPckt[5] = 0xEE;

}

/* WiFi Client */
void CommLgc::stopClient(){
	//TODO to be tested
	uint8_t result = 0;
	uint8_t _sock = 0;
	//Serial.println("dentroww");
	_sock = (uint8_t)_reqPckt.params[0].param[0];
	if(_sock < MAX_SOCK_NUM){
		if(mapWiFiClients[_sock] != NULL ){
			mapWiFiClients[_sock].stop();
			//Serial.println("dentro stop");
			result = 1;
		}
		else if(mapWiFiUDP[_sock] != NULL){
			mapWiFiUDP[_sock].stop();
			//Serial.println("dentro UDP stop");
			result = 1;
		}
	}

	//set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;
	//Serial.println("dopo resp stop");

}

void CommLgc::clientStatus(){
  //TODO to be tested
  uint8_t result = 0;
  uint8_t _sock = 0; //socket index
  _sock = (uint8_t)_reqPckt.params[0].param[0];
  if(_sock < MAX_SOCK_NUM) {
    if(mapWiFiClients[_sock] == NULL){
      if(mapWiFiServers[_sock] != NULL){
        mapWiFiClients[_sock] = mapWiFiServers[_sock]->available(); //Create the client from the server [Arduino as a Server]
        //Serial.println( mapWiFiClients[_sock]);
        result = mapWiFiClients[_sock].status();
      }
    }else {
      result = mapWiFiClients[_sock].status();
    }
  }
//    if(_sock < MAX_SOCK_NUM) {
//      if(mapWiFiClients[_sock] != NULL){
//          //mapWiFiClients[_sock] = mapWiFiServers[_sock]->available(); //Create the client from the server [Arduino as a Server]
//          result = mapWiFiClients[_sock].status();
//        }
//      else {
//        //Serial.print("else");
//        result = 0;
//      }
//    }
    //Serial.println(result);
  //set the response struct
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;
  // _resPckt->nParam = 1;
  // _resPckt->params[0].paramLen = 1;
  // _resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
  // _resPckt->params[0].param[0] = result;
  // SPISlave.setData("ciao");
}

//void CommLgc::clientStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt){
//	//TODO to be tested
//	uint8_t result = 0;
//	uint8_t _sock = 0; //socket index
//	_sock = (uint8_t)_reqPckt.params[0].param[0];
//	if(_sock < MAX_SOCK_NUM) {
//		if(mapWiFiClients[_sock] == NULL){
//			if(mapWiFiServers[_sock] != NULL){
//				mapWiFiClients[_sock] = mapWiFiServers[_sock]->available(); //Create the client from the server [Arduino as a Server]
//				result = mapWiFiClients[_sock].status();
//			}
//		}else {
//			result = mapWiFiClients[_sock].status();
//		}
//	}
//
//	//set the response struct
//	_resPckt->nParam = 1;
//	_resPckt->params[0].paramLen = 1;
//	_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
//	_resPckt->params[0].param[0] = result;
//}

// int CommLgc::httpWrite(){
//
// 	tcpResult= mapWiFiClients[sock].write(sendData_array, sendData_len);
// 	return tcpResult;
//
// }

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
	_resPckt[5] = 0xEE;

}

void CommLgc::checkDataSent(){
	//TODO to be tested

	//set the response struct
     _resPckt[2] = 1;
     _resPckt[3] = 1;
     _resPckt[4] = tcpResult;
     _resPckt[5] = 0xEE;
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
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;
}

/* WiFi UDP Client */
void CommLgc::remoteData(){
	//TODO to be tested
	int _sock;

	//retrieve sockets number
	_sock = (int)_reqPckt.params[0].param[0];

	if(_sock < MAX_SOCK_NUM && mapWiFiUDP[_sock] != NULL) {
		//_resPckt->nParam = 2;
		_resPckt[2] = 2;
		_resPckt[3] = 4;
		//_resPckt->params[0].paramLen = 4;
		//_resPckt->params[1].paramLen = 2;

		IPAddress remoteIp = mapWiFiUDP[_sock].remoteIP();
		//_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
		_resPckt[4] = remoteIp.operator[](0);
		_resPckt[5] = remoteIp.operator[](1);
		_resPckt[6] = remoteIp.operator[](2);
		_resPckt[7] = remoteIp.operator[](3);
		// _resPckt->params[0].param[0] = remoteIp.operator[](0);
		// _resPckt->params[0].param[1] = remoteIp.operator[](1);
		// _resPckt->params[0].param[2] = remoteIp.operator[](2);
		// _resPckt->params[0].param[3] = remoteIp.operator[](3);

		uint16_t remotePort = mapWiFiUDP[_sock].remotePort();
		//_resPckt->params[1].param = (char*)malloc(_resPckt->params[1].paramLen);
		_resPckt[8] = 2;
		_resPckt[9] = (uint8_t)((remotePort & 0xff00)>>8);
		_resPckt[10] = (uint8_t)(remotePort & 0xff);
		_resPckt[11] = 0xEE;
		//_resPckt->params[1].param[0] = (uint8_t)((remotePort & 0xff00)>>8);
		//_resPckt->params[1].param[1] = (uint8_t)(remotePort & 0xff);

	} else {
		createErrorResponse();
	}
}

void CommLgc::getDataBuf(){
	//TODO: To be tested
	//Serial1.println("DATA BUF 1");
	int result = 0;
	uint8_t _sock = 0;
	//char buffer[bufferSize]; //bufferSize is filled before by availData
	//Serial1.println("DATA BUF 2 ");//Serial1.println(bufferSize);
	//retrieve socket index
	_sock = (uint8_t)_reqPckt.paramsData[0].data[0];
	//Serial1.println("DATA BUF 3 ");Serial1.println(_sock);
	if(_sock < MAX_SOCK_NUM){
	  if(mapWiFiUDP[_sock] != NULL){
  		//Serial1.println("DATA BUF 4");
      char buffer[bufferSize]; //bufferSize is filled before by availData
  		result = mapWiFiUDP[_sock].read(buffer, bufferSize);
			//TODO
      // _resPckt->nParam = 1;
      // _resPckt->paramsData[0].dataLen = bufferSize;
      // //_resPckt->paramsData[0].data = (char*)malloc(_resPckt->params[0].paramLen);
      // strncpy(_resPckt->paramsData[0].data, buffer, bufferSize);
	  }
    else if(mapWiFiClients[_sock] != NULL){
      //Serial1.println("DATA BUF 3");
      //bufferSize =5;
      uint8_t buffer_tcp[bufferSize];
      result = mapWiFiClients[_sock].read(buffer_tcp, bufferSize);
      //Serial1.println("DATA BUF 4");
      //Serial1.println(buffer_tcp);
			//TODO need to add a buffer
      //_resPckt->nParam = 1;
      //_resPckt->paramsData[0].dataLen = bufferSize;
      //_resPckt->paramsData[0].data = (char*)malloc(_resPckt->paramsData[0].dataLen);
      //strncpy(_resPckt->paramsData[0].data, (char*)buffer_tcp, bufferSize);
      //_resPckt->paramsData[0].data = "ciao\0";
      //host[_reqPckt.params[0].paramLen] = '\0';
      //Serial1.println("DATA BUF 5");
    }

	}
	//Serial1.println("DATA BUF 6");
}

void CommLgc::insDataBuf(){
	//TODO: To be tested

	//NOTE myabe can use sendData, it's similar to this except the UDP

	uint8_t result = 0;
	uint8_t _sock = 0;
	//Serial1.println("INS BUF 1 ");
	//retrieve socket index
	_sock = (uint8_t)_reqPckt.paramsData[0].data[0];
	//Serial1.println("INS BUF 2 ");Serial1.println(_sock);
	if(_sock < MAX_SOCK_NUM && mapWiFiUDP[_sock] != NULL){
		//send data to client
		//Serial1.println("INS BUF 3 ");
		//Serial1.println(_reqPckt.paramsData[1].data);
		//Serial1.println("INS BUF 4 ");
		//Serial1.println(_reqPckt.paramsData[1].dataLen);
		//Serial1.println("INS BUF 5 ");
		//result =
		mapWiFiUDP[_sock].write(_reqPckt.paramsData[1].data, _reqPckt.paramsData[1].dataLen);
		//Serial1.println(result);
		//Serial1.println("INS BUF 6 ");
		result = 1;
	}
	//Serial1.println("INS BUF 7 ");
	//set the response struct
	//_resPckt->nParam = 1;
	//_resPckt->params[0].paramLen = 1;
	//_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	//_resPckt->params[0].param[0] = result;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;
	//DEBUG(_resPckt);
}

void CommLgc::sendUdpData(){
	//TODO: To be tested
	int result = 0;
	uint8_t _sock = 0;

	//retrieve socket index
	_sock = (uint8_t)_reqPckt.paramsData[0].data[0];

	if(_sock < MAX_SOCK_NUM && mapWiFiUDP[_sock] != NULL){
		//send data to client
		result = mapWiFiUDP[_sock].endPacket();
	}
	//set the response struct
	// _resPckt->nParam = 1;
	// _resPckt->params[0].paramLen = 1;
	// //_resPckt->params[0].param = (char*)malloc(_resPckt->params[0].paramLen);
	// _resPckt->params[0].param[0] = result;
	_resPckt[2] = 1;
	_resPckt[3] = 1;
	_resPckt[4] = result;
	_resPckt[5] = 0xEE;

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
