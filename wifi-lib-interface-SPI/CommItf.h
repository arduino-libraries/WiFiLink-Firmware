//#include "SPISlave.h"
#include "Arduino.h"
#include "utility/wifi_utils.h"
#include "config.h"

#ifndef H_COMM_ITF_H
#define H_COMM_ITF_H

// enum CHANNEL {
//   CH_SERIAL = 0,
//   CH_SPI
// };
//
// enum MCU {
//   AVR328P = 0,
//   NRF52,
//   STM32,
//   SAMD21
// };

class CommItf {

public:

	CommItf();
	bool begin();
	int read(tMsgPacket *_pck);
	void write(uint8_t *_pck, int transfer_size);
  void end();
  bool available();

private:

  int createPacket(tMsgPacket *_pck);

  /*SPI*/
  #if defined ESP_CH_SPI
  void SPISlaveInit();
  void SPISlaveWrite(uint8_t* _resPckt,int transfer_size);
  #endif

  /*Serial*/
  #if defined ESP_CH_UART
  String readStringUntil(char);
  int timedRead();
  #endif

};

extern CommItf CommunicationInterface;

#endif
