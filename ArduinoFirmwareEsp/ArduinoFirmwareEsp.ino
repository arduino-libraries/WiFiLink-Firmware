#include "CommLgc.h"

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <FS.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <ESP8266WebServer.h>

#include <dfu.h>
#include <dfu-host.h>
#include <dfu-cmd.h>
#include <user_config.h>
#include <dfu-internal.h>
#include <dfu-stm32.h>
#include <stk500-device.h>
#include <dfu-stk500.h>
#include <esp8266-serial.h>
#include <dfu-esp8266.h>
#include <dfu-avrisp.h>

ESP8266WebServer server(80);    //server UI
bool SERVER_STOP = false;       //check stop server

struct dfu_data *global_dfu;
struct dfu_binary_file *global_binary_file;

static int _setup_dfu(void)
{
    if(BOARDMODEL == "StarOtto")
        global_dfu = dfu_init(&esp8266_serial_star8_interface_ops,
                        NULL,
                        NULL,
                        &stm32_dfu_target_ops,
                        &stm32f469bi_device_data,
                        &esp8266_dfu_host_ops);
    else if(BOARDMODEL == "UnoWiFi")
         global_dfu = dfu_init(&esp8266_serial_arduino_unowifi_interface_ops,
                        NULL,
                        NULL,
                        &stk500_dfu_target_ops,
                        &atmega328p_device_data,
                        &esp8266_dfu_host_ops);

  if (!global_dfu) {
    Serial1.printf("Error initializing dfu library");
    /* FIXME: Is this ok ? */
    return -1;
  }

  global_binary_file = dfu_binary_file_start_rx(&dfu_rx_method_http_arduino, global_dfu, NULL);
  if (!global_binary_file) {
    Serial1.printf("Error instantiating binary file");
    return -1;
  }
  
  if (dfu_binary_file_flush_start(global_binary_file) < 0) {
      Serial1.printf("Error in dfu_binary_file_flush_start()");
      return -1;
  }
  return 0;
}

void _finalize_dfu(void)
{
  dfu_binary_file_fini(global_binary_file);
  dfu_fini(global_dfu);
  global_dfu = NULL;
  global_binary_file = NULL;
}

void setup() {

  _setup_dfu();
  ArduinoOTA.begin();
  CommunicationLogic.begin();
  initWBServer();               //UI begin
  initMDNS();                   //set MDNS

}

void loop() {

  ArduinoOTA.handle();
  CommunicationLogic.handle();
  if(CommunicationLogic.UI_alert){			//stop UI SERVER
    if(!SERVER_STOP){
      server.stop();
      SERVER_STOP = true;
    }
  }
  else
    handleWBServer();

        if (!global_dfu)
        _setup_dfu();        
    if (!global_dfu)
      return;
   switch (dfu_idle(global_dfu)) {
    case DFU_ERROR:
      Serial1.printf("Error programming file");
      _finalize_dfu();
      break;
    case DFU_ALL_DONE:
      Serial1.printf("Programming OK");
      dfu_target_go(global_dfu);
      _finalize_dfu();
      break;
    case DFU_CONTINUE:
      break;
    }

}
