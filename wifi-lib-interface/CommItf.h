//#include "SPISlave.h"
#include "Arduino.h"
#include "utility/wifi_utils.h"

#ifndef H_COMM_ITF_H
#define H_COMM_ITF_H

enum CHANNEL {
  CH_SERIAL,
  CH_SPI
};

enum MCU {
  AVR328P,
  NRF52,
  STM32,
  SAMD21
};

class CommItf {

public:
	CommItf();
	bool begin();
	int read(tMsgPacket *_pck);
	void write(tMsgPacket *_pck);
	void end();

private:
	int createPacketFromSerial(tMsgPacket *_pck);
	int createPacketFromSPI(tMsgPacket *_pck);
	String readStringUntil(char);
	int timedRead();
};

extern CommItf CommunicationInterface;

#endif
