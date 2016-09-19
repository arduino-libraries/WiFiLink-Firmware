//#include "SPISlave.h"
#include "Arduino.h"

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
	String read();
	void write(String); //or int
	void end();

};

extern CommItf CommunicationInterface;

#endif
