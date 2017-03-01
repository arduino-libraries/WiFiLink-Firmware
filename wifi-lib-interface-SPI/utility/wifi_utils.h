/*
	wifi_utils.h - Library for Arduino WiFi ESP8266 Firmware
	Copyright (c) 2016 Arduino Srl.  All right reserved.

	based on:

	wifi_spi.h - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef H_WIFI_UTILS_H
#define H_WIFI_UTILS_H

#include <inttypes.h>

#define CMD_FLAG          0
#define REPLY_FLAG        1<<7
#define DATA_FLAG 		    0x40

#define WIFI_SPI_ACK      1
#define WIFI_SPI_ERR      0xFF

//#define TIMEOUT_CHAR      1000

#define	MAX_SOCK_NUMBER   4	/**< Maxmium number of socket  */
#define NO_SOCKET_AVAIL   255

#define START_CMD         0xE0
#define END_CMD           0xEE
#define ERR_CMD   	      0xEF

#define RESPONSE_LENGHT   384   //array for response length

#define SPI_BUFFER_SIZE   128
#define SPI_DATA_READY    1
#define SPI_DATA_RECEIVED 2
#define SLAVE_READY_PIN     5

enum numParams{
    PARAM_NUMS_0 = 0,
    PARAM_NUMS_1,
    PARAM_NUMS_2,
    PARAM_NUMS_3,
    PARAM_NUMS_4,
    PARAM_NUMS_5,
    MAX_PARAM_NUMS
};

enum sizeParams{
    PARAM_SIZE_0 = 0,
    PARAM_SIZE_1,
    PARAM_SIZE_2,
    PARAM_SIZE_3,
    PARAM_SIZE_4,
    PARAM_SIZE_5,
    PARAM_SIZE_6
};


#define MAX_PARAMS MAX_PARAM_NUMS-1
//#define PARAM_LEN_SIZE 1

typedef struct  __attribute__((__packed__))
{
	uint8_t	paramLen;
	uint8_t	param[128];
	//String	param;
  //char*	param;
}	tParam;

typedef struct  __attribute__((__packed__))
{
	uint16_t	dataLen;
	uint8_t		data[128];
  //char* data;
} tDataParam;

typedef struct  __attribute__((__packed__))
{
	unsigned char	cmd;
	unsigned char	tcmd;
	unsigned char	nParam;
	tParam	params[MAX_PARAMS];
	tDataParam	paramsData[MAX_PARAMS];
}	tMsgPacket;//tSpiMsg;

typedef struct  __attribute__((__packed__))
{
	unsigned char		cmd;
	unsigned char		tcmd;
	unsigned char		nParam;
	tDataParam			params[MAX_PARAMS];
}	tMsgPacketData;//tSpiMsgData;

// typedef struct  __attribute__((__packed__))
// {
// 	unsigned char		cmd;
// 	unsigned char		tcmd;
// 	//unsigned char	totLen;
// 	unsigned char		nParam;
// }	tSpiHdr;
//
// typedef struct  __attribute__((__packed__))
// {
// 	uint8_t		paramLen;
// 	uint32_t	param;
// }	tLongParam;
//
// typedef struct  __attribute__((__packed__))
// {
// 	uint8_t		paramLen;
// 	uint16_t	param;
// }	tIntParam;
//
// typedef struct  __attribute__((__packed__))
// {
// 	uint8_t		paramLen;
// 	uint8_t		param;
// }	tByteParam;

#endif
