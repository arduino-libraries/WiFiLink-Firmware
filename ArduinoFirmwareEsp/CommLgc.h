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
#include "CommItf.h"
#include "utility/wifi_utils.h"
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiScan.h>
#include <WiFiUdp.h>
#include "Arduino.h"

#ifndef H_COMM_LGC_H
#define H_COMM_LGC_H

// #define MAX_MODE_NUM 2
// #define MAP_TCP_MODE 1

class CommLgc {

public:
	CommLgc();
	bool UI_alert = false;

	/* Logic Functions */
	void begin();
	void handle();

	/* Commands Functions */
private:

	/* WiFi Communication */
	void createErrorResponse();
	void process();

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
