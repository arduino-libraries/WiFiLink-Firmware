
/*
 * Define board model name
 */

#define STAROTTO
//#define PRIMO
//#define UNOWIFIDEVED


/*
 * Enable/Disable Debug
 */

//#define DEBUG
//#define BAUDRATE_DEBUG 115200


/*
 * Defines the communication channel between microcontroller
 * and esp82266, with concerning parameters
 */


#if defined(STAROTTO)
  //Arduino STAR OTTO configuration parameters
  #define BOARDMODEL "STAROTTO"
  #define ESP_CH_UART
  #define BAUDRATE_COMMUNICATION 115200
  #define WIFI_LED 14
  #define HOSTNAME "Arduino-Star-Otto"
#elif defined(PRIMO)
  //Arduino PRIMO configuration parameters
  #define BOARDMODEL "PRIMO"
  #define ESP_CH_SPI
  #define WIFI_LED 2
  #define HOSTNAME "Arduino-Primo"
#elif defined(UNOWIFIDEVED)
  //Arduino UNO WIFI DEV. EDITION configuration parameters
  #define BOARDMODEL "UNOWIFIDEVED"
  #define ESP_CH_UART
  #define BAUDRATE_COMMUNICATION 19200
  #define WIFI_LED 14
  #define HOSTNAME "Arduino-Uno-WiFi"
#endif
