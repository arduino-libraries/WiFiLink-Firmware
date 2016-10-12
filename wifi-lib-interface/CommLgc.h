#include "CommItf.h"
#include "Arduino.h"
#include "utility/wifi_utils.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiScan.h>

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

private:

	/* Logic Functions */
	void freeMem(tMsgPacket *_pckt);
	void DEBUG(tMsgPacket *_pckt);
	void createErrorResponse(tMsgPacket *_pckt);
	void process(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getParam(tParam *param, uint8_t * data);
	void DEBUG_MEM();
	/* Commands Functions */

	/* WiFi Base */
	void getCurrentSSID(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getRSSI(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current);
	void getEncryption(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current);
	void getMacAddress(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void disconnect(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void begin(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current);
	void startScanNetwork(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void scanNetwork(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getBSSID(tMsgPacket *_reqPckt, tMsgPacket *_resPckt, uint8_t current);
	void config(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void reqHostByName(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getHostByName(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getFwVersion(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	/* WiFI IPAddress */
	void getNetworkData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);

	/* WiFi Server */
	void startServer(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void availData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void serverStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void getData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void sendData(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void checkDataSent(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);

	/* WiFi Client */
	void startClient(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void stopClient(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
	void clientStatus(tMsgPacket *_reqPckt, tMsgPacket *_resPckt);
};

extern CommLgc CommunicationLogic;

#endif
