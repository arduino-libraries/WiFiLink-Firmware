//Communication Interface
#include "CommItf.h"
#include "utility/wifi_utils.h"
/*
	TODO: select the correct driver, depends on the target MCU. supported drivers are:
	SPI: SPI Driver
	SER: Serial Driver
*/


//TODO the following const must be setted elsewhere
//#define END_CMD char(0xEE)
#define END_CMD '\n'

CHANNEL CommChannel;
MCU McuType;
int BaudRate;
int SlaveSelectPin;

CommItf::CommItf()
{
	//TODO: MCU selection by compiler flags

	//TEST
	 McuType = AVR328P;

	 switch(int(McuType)){
	 	case AVR328P:
	 		CommChannel = CH_SERIAL;
	 		BaudRate = 9600;
	 	break;
	 	case NRF52:
	 		CommChannel = CH_SPI;
	 		SlaveSelectPin = 31;	//Internal GPIO of the NRF52
	 	break;
	 	case SAMD21:	//TODO test these identical cases
	 	case STM32:
	 		CommChannel = CH_SERIAL;
	 		BaudRate = 115200;
	 	break;
	 }
}

bool CommItf::begin()
{
	 if(CommChannel == CH_SERIAL){
	 	Serial.begin(BaudRate);
    while(!Serial);
    return true;
	 }
	 else if(CommChannel == CH_SPI){
	 	//TODO:
	 	//SPISlave.begin();
    //return true;
	 }
   return false;
}

void CommItf::read(tMsgPacket *_pckt)
{
	 if(CommChannel == CH_SERIAL){
		return createPacketFromSerial(_pckt);
	 }
	 else if(CommChannel == CH_SPI){
		 return createPacketFromSPI(_pckt);
	 }
}


void CommItf::createPacketFromSerial(tMsgPacket *_pckt){
	//TODO read message packet from serial
	//String raw_pckt = Serial.readStringUntil(END_CMD);

	//TEST
	//_pckt->test = "test-data";


	//TODO parse the message and create the packet
}

void CommItf::createPacketFromSPI(tMsgPacket *_pckt){
	//TEST
	//_pckt->test = "test-data";

	//TODO parse the message and create the packet
}


void CommItf::write(tMsgPacket *_packet)
{
   if(CommChannel == CH_SERIAL){
	 	//Serial.print(packet + String(END_CMD));
	 }
	 else if(CommChannel == CH_SPI){
	 	//TODO
	 }
}

void CommItf::end()
{
	 if(CommChannel == CH_SERIAL){
	 	Serial.end();
	 }
	 else if(CommChannel == CH_SPI){
	 	//TODO
	 	//SPISlave.end()
	 }
}

CommItf CommunicationInterface;
