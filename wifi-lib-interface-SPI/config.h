
/*
 * Define board model name
 *
 */

#define BOARDMODEL "STARTOTTO"
//#define BOARDMODEL "PRIMO"
//#define BOARDMODEL "UNOWFI"

/*
 * Defines the communication channel between microcontroller
 * and esp82266, with concerning parameters
 *
 */

if (BOARDMODEL == "STARTOTTO"){
  //Arduino STAR OTTO configuration parameters
  #define ESP_CH_UART
  #define BAUDRATE_COMMUNICATION 460800
  #define BAUDRATE_DEBUG 115200
  #define DEBUG
  #define WIFI_LED 14
  }
else if(BOARDMODEL == "PRIMO"){
  //Arduino PRIMO configuration parameters
  #define ESP_CH_SPI
  #define BAUDRATE_DEBUG 115200
  #define WIFI_LED 2
  #define DEBUG
  }
else if{
  //Arduino UNO WIFI configuration parameters
}
