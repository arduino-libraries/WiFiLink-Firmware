
/*
 * Define board model name
 */

#define BOARDMODEL "STARTOTTO"
//#define BOARDMODEL "PRIMO"
//#define BOARDMODEL "UNOWFI"
//#define BOARDMODEL "UNOWFIDEVED"


/*
 * Enable/Disable Debug
 */

//#define DEBUG


/*
 * Defines the communication channel between microcontroller
 * and esp82266, with concerning parameters
 */

if (BOARDMODEL == "STARTOTTO"){
  //Arduino STAR OTTO configuration parameters
  #define ESP_CH_UART
  #define BAUDRATE_COMMUNICATION 460800
  #define BAUDRATE_DEBUG 115200
  #define WIFI_LED 14
  }
else if(BOARDMODEL == "PRIMO"){
  //Arduino PRIMO configuration parameters
  #define ESP_CH_SPI
  #define BAUDRATE_DEBUG 115200
  #define WIFI_LED 2
  }
else if(BOARDMODEL == "UNOWIFIDEVED"){
  //Arduino UNO WIFI DEV. EDITION
  #define ESP_CH_UART
  #define BAUDRATE_COMMUNICATION 19200
  #define BAUDRATE_DEBUG 115200
  #define WIFI_LED 14
}
