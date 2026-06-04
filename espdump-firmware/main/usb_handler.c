#include "usb_handler.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include" esp_err.h"

#include "tinyusb.h"
#include "tusb_cdc_acm.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"

static const char *TAG = "usb_handler";

#define USB_RX_BUFSIZE (32 * 1024)
#define USb_TX_BUFSIZE (21 * 1024)

static RingbufHandle_t rx_ringbuf = NULL;
static RingbufHandle_t tx_ringbuf = NULL;
static volatile bool usb_connected = false;