#include "CommItf.h"

#include "utility/wifi_utils.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiScan.h>
#include <WiFiUdp.h>
#include "Arduino.h"

#ifndef H_COMM_LGC_H
#define H_COMM_LGC_H

#define MAX_MODE_NUM 2
#define MAP_TCP_MODE 1

#define debug false

class CommLgc {

public:
	CommLgc();
  void begin();
	void handle();



	/* Logic Functions */
	void freeMem(tMsgPacket *_pckt);
	void DEBUG(tMsgPacket *_pckt);
  //void DEBUG(tMsgPacketStatic *_pckt);
	//void createErrorResponse(tMsgPacket *_pckt);
	void createErrorResponse(char* _pckt);
	//void process(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	//void process(tMsgPacket *_reqPckt, char* _resPckt);
	void process(tMsgPacket *_reqPckt, char* resp);
	void getParam(tParam *param, uint8_t * data);
	void DEBUG_MEM();
	/* Commands Functions */
private:
	/* WiFi Base */
	//void getCurrentSSID(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getCurrentSSID(tMsgPacket *_reqPckt, char* _resPckt);
	//void getRSSI(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current);
	void getRSSI(tMsgPacket *_reqPckt, char* _resPckt, uint8_t current);
	//void getEncryption(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current);
	void getEncryption(tMsgPacket *_reqPckt, char* _resPckt, uint8_t current);
	//void getMacAddress(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getMacAddress(tMsgPacket *_reqPckt, char* _resPckt);
	//void disconnect(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void disconnect(tMsgPacket *_reqPckt, char* _resPckt);
	//void getStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getStatus(tMsgPacket *_reqPckt, char* _resPckt);
	//void begin(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current);
	void begin(tMsgPacket *_reqPckt, char* _resPckt, uint8_t current);
	//void startScanNetwork(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void startScanNetwork(tMsgPacket *_reqPckt, char* _resPckt);
	//void scanNetwork(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void scanNetwork(tMsgPacket *_reqPckt, char* _resPckt);
	//void getBSSID(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current);
	void getBSSID(tMsgPacket *_reqPckt, char* _resPckt, uint8_t current);
	//void config(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void config(tMsgPacket *_reqPckt, char* _resPckt);
	//void setDNS(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void setDNS(tMsgPacket *_reqPckt, char* _resPckt);
	//void reqHostByName(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void reqHostByName(tMsgPacket *_reqPckt, char* _resPckt);
	//void getHostByName(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getHostByName(tMsgPacket *_reqPckt, char* _resPckt);
	//void getFwVersion(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getFwVersion(tMsgPacket *_reqPckt, char* _resPckt);
	/* WiFI IPAddress */
	//void getNetworkData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getNetworkData(tMsgPacket *_reqPckt, char* _resPckt);

	/* WiFi Server */
	//void startServer(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
  void startServer(tMsgPacket *_reqPckt, char* _resPckt);
	//void availData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void availData(tMsgPacket *_reqPckt, char* _resPckt);
	//void serverStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void serverStatus(tMsgPacket *_reqPckt, char* _resPckt);
	//void getData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getData(tMsgPacket *_reqPckt, char* _resPckt);
	//void sendData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
  void sendData(tMsgPacket *_reqPckt, char* _resPckt);
	//void checkDataSent(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
  void checkDataSent(tMsgPacket *_reqPckt, char* _resPckt);

	/* WiFi Client */
	//void startClient(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void startClient(tMsgPacket *_reqPckt, char* _resPckt);
	//void stopClient(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void stopClient(tMsgPacket *_reqPckt, char* _resPckt);
	//void clientStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void clientStatus(tMsgPacket *_reqPckt, char* _resPckt);

	/* WiFI UDP Client */
	//void remoteData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void remoteData(tMsgPacket *_reqPckt, char* _resPckt);
	//void getDataBuf(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getDataBuf(tMsgPacket *_reqPckt, char* _resPckt);
	//void insDataBuf(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void insDataBuf(tMsgPacket *_reqPckt, char* _resPckt);
	//void sendUdpData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void sendUdpData(tMsgPacket *_reqPckt, char* _resPckt);

};

extern CommLgc CommunicationLogic;

#endif
