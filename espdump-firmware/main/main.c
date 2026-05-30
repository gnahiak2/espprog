// main/c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/uart.h"
#include "spi_flash.h"

#define UART_NUM UART_NUM_0

#define BUF_SIZE (64 * 1024)

static const char *TAG = "espdump";

uint32_t flash_size_from_jedec(uint8_t cap)
{
    switch (cap)
    {
        case 0x18: return 16 * 1024 * 1024;
        case 0x17: return 8 * 1024 * 1024;
        case 0x16: return 4 * 1024 * 1024;
        default: return 16 * 1024 * 1024;
    }
}

void app_main()
{
    
}