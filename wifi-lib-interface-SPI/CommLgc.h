//#include "CommItf.h"

#include "utility/wifi_utils.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiScan.h>
#include <WiFiUdp.h>
#include "Arduino.h"

#ifndef H_COMM_LGC_H
#define H_COMM_LGC_H

// #define MAX_MODE_NUM 2
// #define MAP_TCP_MODE 1

class CommLgc {

public:
	CommLgc();

	/* Logic Functions */
	void begin();
	void handle();
	void createErrorResponse();
	void process();

	/* DEBUG */
	void DEBUG_MEM();

	/* Commands Functions */
private:

	/* WiFi Communication */
	bool processing = false;
	bool req_send = false;
	int createPacketFromSPI();
	void initSPISlave();

	/* WiFi Base */
	void getCurrentSSID();
	void getRSSI(uint8_t current);
	void getEncryption(uint8_t current);
	void getMacAddress();
	void disconnect();
	void getStatus();
	void begin(uint8_t current);
	void startScanNetwork();
	void scanNetwork();
	void getBSSID(uint8_t current);
	void config();
	void setDNS();
	void reqHostByName();
	void getHostByName();
	void getFwVersion();
	void getNetworkData();

	/* WiFi Server */
  void startServer();
	void availData();
	void serverStatus();
	void getData();
  void sendData();
  void checkDataSent();

	/* WiFi Client */
	void startClient();
	void stopClient();
	void clientStatus();

	/* WiFI UDP Client */
	void remoteData();
	void getDataBuf();
	void insDataBuf();
	void sendUdpData();

};

extern CommLgc CommunicationLogic;

#endif
