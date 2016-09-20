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
	void process(tMsgPacket *_pckt);
  String getCurrentSSID(String);
};

extern CommLgc CommunicationLogic;

#endif
