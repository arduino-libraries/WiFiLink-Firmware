#include "CommItf.h"
#include "Arduino.h"

#ifndef H_COMM_LGC_H
#define H_COMM_LGC_H

class CommLgc {

public:
	CommLgc();
  void begin();
	void handle();
private:
	String process(String);
  String getCurrentSSID(String);
};

extern CommLgc CommunicationLogic;

#endif
