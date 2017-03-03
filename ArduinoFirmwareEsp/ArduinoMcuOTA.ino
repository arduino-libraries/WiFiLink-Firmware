#if defined(ESP_CH_UART)

#include <dfu.h>
#include <dfu-host.h>
#include <dfu-cmd.h>
#include <user_config.h>
#include <dfu-internal.h>
#include <esp8266-serial.h>
#include <dfu-esp8266.h>
#include <dfu-avrisp.h>
#if defined(STAROTTO)
#include <dfu-stm32.h>
#elif defined(UNOWIFIDEVED)
#include <stk500-device.h>
#include <dfu-stk500.h>
#endif

struct dfu_data *global_dfu;
struct dfu_binary_file *global_binary_file;

static int _setup_dfu(void)
{
  #if defined(STAROTTO)
  global_dfu = dfu_init(&esp8266_serial_star8_interface_ops,
                NULL,
                NULL,
                &stm32_dfu_target_ops,
                &stm32f469bi_device_data,
                &esp8266_dfu_host_ops);
  #elif defined(UNOWIFIDEVED)
  global_dfu = dfu_init(&esp8266_serial_arduino_unowifi_interface_ops,
                NULL,
                NULL,
                &stk500_dfu_target_ops,
                &atmega328p_device_data,
                &esp8266_dfu_host_ops);
  #endif

  if (!global_dfu) {
    #if defined(DEBUG)
    Serial1.printf("Error initializing dfu library");
    #endif
    /* FIXME: Is this ok ? */
    return -1;
  }

  global_binary_file = dfu_binary_file_start_rx(&dfu_rx_method_http_arduino, global_dfu, NULL);
  if (!global_binary_file) {
    #if defined(DEBUG)
    Serial1.printf("Error instantiating binary file");
    #endif
    return -1;
  }

  if (dfu_binary_file_flush_start(global_binary_file) < 0) {
    #if defined(DEBUG)
    Serial1.printf("Error in dfu_binary_file_flush_start()");
    #endif
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

void _handle_Mcu_OTA(void)
{
  if (!global_dfu)
    _setup_dfu();
  if (!global_dfu)
    return;
  switch (dfu_idle(global_dfu)){
    case DFU_ERROR:
      #if defined(DEBUG)
      Serial1.printf("Error programming file");
      #endif
      _finalize_dfu();
      break;
    case DFU_ALL_DONE:
      #if defined(DEBUG)
      Serial1.printf("Programming OK");
      #endif
      dfu_target_go(global_dfu);
      _finalize_dfu();
      break;
    case DFU_CONTINUE:
      break;
  }
}

#endif
