
/*
 * Define board model name
 */

//#define STAROTTO
//#define PRIMO
#define UNOWIFIDEVED


/*
 * Enable/Disable Debug
 */

//#define DEBUG
//#define BAUDRATE_DEBUG 115200


/*
 * Defines the communication channel between microcontroller
 * and esp82266, with concerning parameters
 */

//#define HOSTNAME "arduino" //used by mdns protocol

#if defined(STAROTTO)
  //Arduino STAR OTTO configuration parameters
  #define BOARDMODEL "STAROTTO"
  #define ESP_CH_UART
  #define BAUDRATE_COMMUNICATION 460800
  #define WIFI_LED 14
  #define SSIDNAME "Arduino-Star-Otto"
#elif defined(PRIMO)
  //Arduino PRIMO configuration parameters
  #define BOARDMODEL "PRIMO"
  #define ESP_CH_SPI
  #define WIFI_LED 2
  #define SSIDNAME "Arduino-Primo"
#elif defined(UNOWIFIDEVED)
  //Arduino UNO WIFI DEV. EDITION configuration parameters
  #define BOARDMODEL "UNOWIFIDEVED"
  #define ESP_CH_UART
  #define BAUDRATE_COMMUNICATION 19200
  #define WIFI_LED 14
  #define SSIDNAME "Arduino-Uno-WiFi"
#endif

#define BUILD_DATE "20170321"
#define FW_VERSION1 "1.0.0"
#define FW_NAME "wifilink"