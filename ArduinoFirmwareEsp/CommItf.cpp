//Communication Interface
#include "CommItf.h"
#include "utility/wifi_utils.h"
#include "SPISlave.h"

uint8_t raw_pckt[SPI_BUFFER_SIZE];													//SPI buffer (limited to 128 bytes)
//String raw_pckt_serial;
uint8_t data_received_size = 0;													//check received data (maximum 128 byte (4 * 32))
bool req_send = false;																	//check data sent for lenght greater than 32 byte (SPI)
bool processing= false;																	//check data ready to process (SPI)
CommItf* This;																					//need for SPI event

unsigned long _startMillis;															//need to Serial
unsigned long _timeout = 1000; 													//1 Second Serial Timeout for Serial

CommItf::CommItf(){
}

bool CommItf::begin(){

		#if defined ESP_CH_UART
	 	Serial.begin(BAUDRATE_COMMUNICATION);
		while(!Serial);
		#ifdef DEBUG
		Serial1.begin(BAUDRATE_DEBUG);
		Serial1.println("--SERIAL started--");
		#endif
		return true;

		#elif defined ESP_CH_SPI
		pinMode(SLAVE_READY_PIN,OUTPUT);					//set SlaveReady pin
		digitalWrite(SLAVE_READY_PIN,LOW);				//set slaveready to HIGH level when ESP is ready to send data
		SPISlaveInit();
		#ifdef DEBUG
		Serial.begin(BAUDRATE_DEBUG);
		Serial.println("--SPI started--");
		#endif
		return true;
		#endif

	 	return false;
}

bool CommItf::available(){
		#if defined ESP_CH_UART
		return Serial.available();
		#elif defined ESP_CH_SPI
		return processing;
		#endif
}

int CommItf::read(tMsgPacket *_pckt){

			return createPacket(_pckt);
}

/* Cmd Struct Message */
/* _________________________________________________________________________________  */
/*| START CMD | C/R  | CMD  |[TOT LEN]| N.PARAM | PARAM LEN | PARAM  | .. | END CMD | */
/*|___________|______|______|_________|_________|___________|________|____|_________| */
/*|   8 bit   | 1bit | 7bit |  8bit   |  8bit   |   8bit    | nbytes | .. |   8bit  | */
/*|___________|______|______|_________|_________|___________|________|____|_________| */

int CommItf::createPacket(tMsgPacket *_reqPckt){

		#if defined ESP_CH_UART
		String raw_pckt_serial = readStringUntil(END_CMD);
		memcpy(raw_pckt, raw_pckt_serial.c_str(),raw_pckt_serial.length());
		#endif
		
		int idx = 0;
		unsigned char tmp;
		if(raw_pckt[idx] != START_CMD){			//TODO
			return -1;
		}
		_reqPckt->cmd = raw_pckt[idx];
		//The command
		_reqPckt->tcmd = raw_pckt[++idx];

		//The number of parameters for the command
		tmp = raw_pckt[++idx];
		_reqPckt->nParam = tmp;
		//Get each parameter
		for(int a=0; a<(int)_reqPckt->nParam; a++){
			//Length of the parameter
			if( _reqPckt->tcmd >= 0x40 && _reqPckt->tcmd < 0x50 ){
				//16bit tParam
				tmp = (uint16_t)((raw_pckt[++idx] << 8) + (uint8_t)raw_pckt[++idx]);
				_reqPckt->paramsData[a].dataLen = tmp;
				memcpy(_reqPckt->paramsData[a].data,raw_pckt+(++idx),tmp);
				idx = idx+(tmp-1);
			}
			else{
				//8bit tParamData
				tmp = raw_pckt[++idx];
				_reqPckt->params[a].paramLen = tmp;
				memcpy(_reqPckt->params[a].param,raw_pckt+(++idx),tmp);
				idx = idx+(tmp-1);
			}
		}
		return 0;

}

void CommItf::write(uint8_t* _pckt,int transfer_size){
		#if defined ESP_CH_UART
		Serial.write(_pckt,transfer_size);
		#elif defined ESP_CH_SPI
		SPISlaveWrite(_pckt,transfer_size);
		#endif
}

// void CommItf::end(){
// 	if(CommChannel == CH_SERIAL)
// 		Serial.end();
// }

/** Private Functions **/

#if defined ESP_CH_SPI
void CommItf::SPISlaveInit(){
	This = this;
	//SPI Data register
	SPISlave.onData([](uint8_t * data, size_t len) {
		if(data_received_size<4){
			digitalWrite(SLAVE_READY_PIN,LOW);
			memcpy(raw_pckt+(data_received_size*32),data, len);
			data_received_size++;
		}
	});

	//SPI Status register
	SPISlave.onStatus([](uint32_t data) {
		if(data==SPI_DATA_READY){
			if(raw_pckt[0]==START_CMD){
				processing = true;
			}
		}
		else if(data==SPI_DATA_RECEIVED){
			digitalWrite(SLAVE_READY_PIN,LOW);
			req_send = true;
		}
		else
			Serial.println("status error");
	});

	// Setup SPI Slave registers and pins
	SPISlave.begin();

}

void CommItf::SPISlaveWrite(uint8_t* _resPckt,int transfer_size){
	SPISlave.setData((uint8_t *)_resPckt,32);                     //send response to MCU
	digitalWrite(SLAVE_READY_PIN,HIGH);
	transfer_size = ceil((float)transfer_size/32);
	if(transfer_size > 0){														//response length greater than 32 bytes
		for(int i=1;i<transfer_size;i++){
			while(!req_send){
				delayMicroseconds(100);										//wait master
			};
			SPISlave.setData((uint8_t *)_resPckt+(i*32),32);					//split response
			digitalWrite(SLAVE_READY_PIN,HIGH);
			req_send = false;
		}
	}
	processing = false;
	data_received_size = 0;
}
#endif

#if defined ESP_CH_UART
int CommItf::timedRead(){
	int c;
	_startMillis = millis();
	do {
		c = Serial.read();
		if (c >= 0) return c;
	} while(millis() - _startMillis < _timeout);
	return -1;     // -1 indicates timeout
}

String CommItf::readStringUntil(char terminator){
	String ret;
	int c = timedRead();

	while (c >= 0 && (char)c != terminator)
	{
		ret += (char)c;
		c = timedRead();
	}
	return ret;
}
#endif

CommItf CommunicationInterface;
