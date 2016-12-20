//Communication Interface
#include "CommLgc.h"
#include "CommItf.h"
//#include "utility/wifi_utils.h"
#include "SPISlave.h"
/*
	TODO: select the correct driver, depends on the target MCU. supported drivers are:
	SPI: SPI Driver
	SER: Serial Driver
*/

//TODO the following const must be setted elsewhere
//#define END_CMD 0xEE
#define END_CMD 0xEE
#define START_CMD 0xE0

unsigned long _startMillis;
unsigned long _timeout = 1000; //1 Second Serial Timeout

//array for spi response
//char response[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

CHANNEL CommChannel;
MCU McuType;
int BaudRate;
int SlaveSelectPin;
String raw_pckt_spi ="";  //packet received from spi master
char resp[256];
int status_ready = 0;   //to check the spi trasmission status
tMsgPacket *resPckt_tmp;
bool en = 0;

int createPacketFromSPI(tMsgPacket *_pckt){
	//TODO parse the message and create the packet
      int idx = 0;
      unsigned char tmp;

      //Start Command
      if(raw_pckt_spi[idx] != START_CMD){
        //Error
        return -1;
      }
        _pckt->cmd = raw_pckt_spi[idx];
        //The command
        _pckt->tcmd = raw_pckt_spi[++idx];
        //The number of parameters for the command
        tmp = raw_pckt_spi[++idx];
        _pckt->nParam = tmp;
        //Get each parameter
        for(int a=0; a<(int)_pckt->nParam; a++){
          //Length of the parameter
          if( _pckt->tcmd >= 0x40 && _pckt->tcmd < 0x50 ){ //16bit tParam
            tmp = (uint16_t)((raw_pckt_spi[++idx] << 8) + (uint8_t)raw_pckt_spi[++idx]);
            _pckt->paramsData[a].dataLen = tmp;

            //_pckt->paramsData[a].data = (char*)malloc(_pckt->paramsData[a].dataLen);
            //Value of the parameter
            for(int b=0; b<(int)_pckt->paramsData[a].dataLen; b++){
              tmp = raw_pckt_spi[++idx];
              _pckt->paramsData[a].data[b] = (char)tmp;
            }
          }else{ //8bit tParamData
            tmp = raw_pckt_spi[++idx];
            _pckt->params[a].paramLen = tmp;
            //_pckt->params[a].param = (char*)malloc(_pckt->params[a].paramLen);
            //Value of the parameter
            for(int b=0; b<(int)_pckt->params[a].paramLen; b++){
              tmp = raw_pckt_spi[++idx];
              _pckt->params[a].param[b] = (char)tmp;

            }
          }
        }
      //OK
      status_ready = 0;   //packet received from master device
      raw_pckt_spi ="";
      return 0;
}

void CommItf::initSPISlave(){

    SPISlave.onData([](uint8_t * data, size_t len) {
       for(int i=0;i<len;i++){
           raw_pckt_spi += (char)data[i];
        }
    });

    SPISlave.onStatus([](uint32_t data) {
        tMsgPacket pckt1;                             //initialize struct to receive a command from MCU
        tMsgPacket *reqPckt = &pckt1;
        memset(resp,0,sizeof(resp));    //reset response array
        createPacketFromSPI(reqPckt);                 //parse the command received
        CommunicationLogic.process(reqPckt, resp);    //process the command
        SPISlave.setData(resp);                       //send response to MCU

    });

    /*SPISlave.onDataSent([]() {
    });
    SPISlave.onStatusSent([]() {
    });*/

    // Setup SPI Slave registers and pins
    SPISlave.begin();

    // Set the status register (if the master reads it, it will read this value)
    //SPISlave.setStatus(millis());

    // Sets the data registers. Limited to 32 bytes at a time.
    // SPISlave.setData(uint8_t * data, size_t len); is also available with the same limitation
    //SPISlave.setData("");

}

CommItf::CommItf(){
	//TODO: MCU selection by compiler flags

	//TEST
	 McuType = NRF52;

	 switch(int(McuType)){
	 	case AVR328P:
	 		CommChannel = CH_SERIAL;
	 		BaudRate = 9600;
	 	break;
	 	case NRF52:
	 		CommChannel = CH_SPI;
      //SlaveSelectPin = 31;	//Internal GPIO of the NRF52
	 	break;
	 	case SAMD21:	//TODO test these identical cases
	 	case STM32:
	 		CommChannel = CH_SERIAL;
	 		BaudRate = 115200;
	 	break;
	 }
}

bool CommItf::begin(){
	 if(CommChannel == CH_SERIAL){
	 	Serial.begin(BaudRate);

		//NOTE: debug only, remove for production env.
		Serial1.begin(BaudRate);

		while(!Serial);
		return true;
	 }
	 else if(CommChannel == CH_SPI){
      Serial.begin(115200);
      Serial.println("init SPI");
      initSPISlave();
      return true;
	}
	return false;
}

int CommItf::read(tMsgPacket *_pckt){
	if(CommChannel == CH_SERIAL){
		if(Serial.available()){
			return createPacketFromSerial(_pckt);
		}else{
			return -1;
		}
	}
	 else if(CommChannel == CH_SPI){
		return createPacketFromSPI(_pckt);
	}
}

int CommItf::createPacketFromSerial(tMsgPacket *_pckt){

		String raw_pckt = readStringUntil(END_CMD);

		int idx = 0;
		unsigned char tmp;

		//Start Command
		if(raw_pckt[idx] != START_CMD){
			//Error
			return -1;
		}
			_pckt->cmd = raw_pckt[idx];;

			//The command
			_pckt->tcmd = raw_pckt[++idx];

			//The number of parameters for the command
			tmp = raw_pckt[++idx];
			_pckt->nParam = tmp;

			//Get each parameter
			for(int a=0; a<(int)_pckt->nParam; a++){
				//Length of the parameter
				if( _pckt->tcmd >= 0x40 && _pckt->tcmd < 0x50 ){ //16bit tParam
					tmp = (uint16_t)( (raw_pckt[++idx] << 8 ) + (uint8_t)raw_pckt[++idx]);
					_pckt->paramsData[a].dataLen = tmp;

					//_pckt->paramsData[a].data = (char*)malloc(_pckt->paramsData[a].dataLen);
					//Value of the parameter
					for(int b=0; b<(int)_pckt->paramsData[a].dataLen; b++){
						tmp = raw_pckt[++idx];
						_pckt->paramsData[a].data[b] = (char)tmp;
					}
				}else{ //8bit tParamData
					tmp = raw_pckt[++idx];
					_pckt->params[a].paramLen = tmp;

					//_pckt->params[a].param = (char*)malloc(_pckt->params[a].paramLen);
					//Value of the parameter
					for(int b=0; b<(int)_pckt->params[a].paramLen; b++){
						tmp = raw_pckt[++idx];
						_pckt->params[a].param[b] = (char)tmp;
					}
				}
			}
		//OK
		return 0;

}

// int CommItf::createPacketFromSPI(tMsgPacket *_pckt){
// 	//TODO parse the message and create the packet
//   if(status_ready){
//       int idx = 0;
//       unsigned char tmp;
//
//       //Start Command
//       if(raw_pckt_spi[idx] != START_CMD){
//         //Error
//         return -1;
//       }
//         _pckt->cmd = raw_pckt_spi[idx];
//         //The command
//         _pckt->tcmd = raw_pckt_spi[++idx];
//         //The number of parameters for the command
//         tmp = raw_pckt_spi[++idx];
//         _pckt->nParam = tmp;
//         //Get each parameter
//         for(int a=0; a<(int)_pckt->nParam; a++){
//           //Length of the parameter
//           if( _pckt->tcmd >= 0x40 && _pckt->tcmd < 0x50 ){ //16bit tParam
//             tmp = (uint16_t)((raw_pckt_spi[++idx] << 8) + (uint8_t)raw_pckt_spi[++idx]);
//             _pckt->paramsData[a].dataLen = tmp;
//
//             _pckt->paramsData[a].data = (char*)malloc(_pckt->paramsData[a].dataLen);
//             //Value of the parameter
//             for(int b=0; b<(int)_pckt->paramsData[a].dataLen; b++){
//               tmp = raw_pckt_spi[++idx];
//               _pckt->paramsData[a].data[b] = (char)tmp;
//             }
//           }else{ //8bit tParamData
//             tmp = raw_pckt_spi[++idx];
//             _pckt->params[a].paramLen = tmp;
//             _pckt->params[a].param = (char*)malloc(_pckt->params[a].paramLen);
//             //Value of the parameter
//             for(int b=0; b<(int)_pckt->params[a].paramLen; b++){
//               tmp = raw_pckt_spi[++idx];
//               _pckt->params[a].param[b] = (char)tmp;
//
//             }
//           }
//         }
//       //OK
//       status_ready = 0;   //packet received from master device
//       raw_pckt_spi ="";
//       return 0;
//   }
//   return -1;
// }

void CommItf::write(tMsgPacket *_pckt){
  if(CommChannel == CH_SERIAL){
    Serial.write(_pckt->cmd);
    Serial.write(_pckt->tcmd);
    Serial.write(_pckt->nParam);
    for(int i=0; i<(int)_pckt->nParam; i++){
      //16 bit
      if(_pckt->tcmd >= (0x40 | REPLY_FLAG) && _pckt->tcmd < (0x50 | REPLY_FLAG) && _pckt->tcmd != (0x44 | REPLY_FLAG) ){
        Serial.write( (uint8_t)((_pckt->paramsData[i].dataLen & 0xFF00) >> 8));
        Serial.write( (uint8_t)(_pckt->paramsData[i].dataLen & 0xFF));
        for(int j=0; j< (int)_pckt->paramsData[i].dataLen; j++)
          Serial.write( _pckt->paramsData[i].data[j]);
      }
      //8 Bit
      else{
        Serial.write(_pckt->params[i].paramLen);
        for(int j=0; j< (int)_pckt->params[i].paramLen; j++)
          Serial.write( _pckt->params[i].param[j]);
      }
    }
    Serial.write(END_CMD);
  }
  //else if(CommChannel == CH_SPI){
  //   memset(response,0,sizeof(response)); //initialize to zero the response array
  //   int idx=0;
  //   response[idx++]= (_pckt->cmd);
  //   response[idx++]=(_pckt->tcmd);
  //   response[idx++]=(_pckt->nParam);
  //   for(int i=0; i<(int)_pckt->nParam; i++){
  //     //16 bit
  //     if(_pckt->tcmd >= (0x40 | REPLY_FLAG) && _pckt->tcmd < (0x50 | REPLY_FLAG) && _pckt->tcmd != (0x44 | REPLY_FLAG) ){
  //       response[idx++]=( (uint8_t)((_pckt->paramsData[i].dataLen & 0xFF00) >> 8));
  //       response[idx++]=( (uint8_t)(_pckt->paramsData[i].dataLen & 0xFF));
  //       for(int j=0; j< (int)_pckt->paramsData[i].dataLen; j++)
  //         response[idx++]=( _pckt->paramsData[i].data[j]);
  //     }
  //     //8 Bit
  //     else{
  //       response[idx++]=(_pckt->params[i].paramLen);
  //       for(int j=0; j< (int)_pckt->params[i].paramLen; j++)
  //         response[idx++]=( _pckt->params[i].param[j]);
  //     }
  //   }
  //   response[idx++]=(END_CMD);
  //   SPISlave.setData(response);
  //   //SPISlave.setData(response);
  //   //SPISlave.setStatus(2);
  //
  //   //Serial.print(response[4],HEX);
  //
  // }

}

void CommItf::end(){
	if(CommChannel == CH_SERIAL){
		Serial.end();
	}
	else if(CommChannel == CH_SPI){
		//TODO
		//SPISlave.end()
	 }
}

/** Private Functions **/

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

CommItf CommunicationInterface;
