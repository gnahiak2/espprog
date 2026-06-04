// spi_flash.c
#include "spi_flash.h"

#include <stdint.h>
#include <string.h>

#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"

#define PIN_MOSI 11
#define PIN_MISO 13
#define PIN_CLK  12
#define PIN_CS   10
#define PIN_WP   9
#define PIN_HOLD 14

static spi_device_handle_t spi;
static const char *TAG = "spi";
static bool g_supports_qspi = false;
static uint8_t spi_buf[4 + 512];

#define CMD_JEDEC      0x9F
#define CMD_FAST_READ  0x0B