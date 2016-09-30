#include "CommItf.h"
#include "Arduino.h"
#include "utility/wifi_utils.h"

#ifndef H_COMM_LGC_H
#define H_COMM_LGC_H

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
	/* Commands Functions */
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



};

extern CommLgc CommunicationLogic;

#endif
