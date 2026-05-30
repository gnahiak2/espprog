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
    spi_flash_init();

    EPS_ERROR_CHECK(
        uart_driver_install(UART_NUM, 4096, 0, 0, NULL, 0);
    );
    
    uint8_t cmd[64];

    // Move buffer to static
    static uint8_t buf[BUF_SIZE];

    while (1)
    {
        int len = uart_read_bytes(
            UART_NUM,
            cmd,
            sizeof(cmd) -1;
            portMAX_DELAY
        );

        if (len <= 0)
            continue;
        
        cmd[len] = 0;

        // Probe
        if (strncmp((char*)cmd, "PROBE", 5) == 0)
        {
            uint8_t id[3];

            spi_read_jedec(id)

            uint32_t size = flash_size_from_jedec(id[2]);

            char out[128];

            snprintf(
                out,
                sizeof(out),
                "OK\n%lu\n",
                (unsigned long)size
            );

            uart_write_bytes(UART_NUM, out, strlen(out));
        }
    }

    // Read fast
    else if (strncmp((char*)cmd, "READ", 4) == 0)
    {
        uint8_t id[3];
        spi_read_jedec(id);

        uint32_t size = flash_size_from_jedec(id[2]);

        uint32_t addr = 0;

        while (addr < size)
        {
            uint32_t chunk = (size - addr > BUF_SIZE)
                ? BUF_SIZE
                : (size - addr);

            spi_flash_read(addr, buf, chunk);

            uart_write_bytes(
                UART_NUM,
                (const char) *buf,
                chunk
            );

            addr += chunk;
        }
        
    }

    // Write
    else if (strncmp((char*)cmd, "WRITE:", 6) == 0)
    {
        uint32_t size = atoi((char*)&cmd[6]);

        uart_write_bytes(UART_NUM, "READY\n", 6);

        uint32_t addr = 0;

        uint8_t
    }
    
}