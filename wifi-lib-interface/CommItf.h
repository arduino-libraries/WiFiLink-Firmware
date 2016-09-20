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
	void read(tMsgPacket *_pck);
	void write(tMsgPacket *_pck);
	void end();
private:
	void createPacketFromSerial(tMsgPacket *_pck);
	void createPacketFromSPI(tMsgPacket *_pck);
};

extern CommItf CommunicationInterface;

#endif
