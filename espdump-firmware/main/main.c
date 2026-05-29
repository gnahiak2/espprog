#include <stdio.h>
#include <string.h>

#include "driver/uart.h"
#include "spi_flash.h"

#define UART_NUM UART_NUM_0
#define BUF_SIZE 256

uint32_t flash_size_from_jedec(uint8_t cap)
{
    switch(cap)
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

    uart_driver_install(UART_NUM, 4096, 0, 0, NULL, 0);

    uint8_t cmd[64];

    while (1)
    {
        int len = uart_read_bytes(
            UART_NUM,
            cmd,
            sizeof(cmd)-1,
            portMAX_DELAY
        );

        if (len <= 0)
            continue;

        cmd[len] = 0;

        // ---------------- PROBE ----------------

        if (strncmp((char*)cmd, "PROBE", 5) == 0)
        {
            uint8_t id[3];

            spi_read_jedec(id);

            uint32_t size =
                flash_size_from_jedec(id[2]);

            char out[128];

            sprintf(
                out,
                "OK\nID:%02X%02X%02X\nSIZE:%lu\n",
                id[0], id[1], id[2],
                (unsigned long)size
            );

            uart_write_bytes(UART_NUM, out, strlen(out));
        }

        // ---------------- READ ----------------

        else if (strncmp((char*)cmd, "READ", 4) == 0)
        {
            uint8_t id[3];

            spi_read_jedec(id);

            uint32_t size =
                flash_size_from_jedec(id[2]);

            uint8_t buf[BUF_SIZE];

            uart_write_bytes(UART_NUM, "START\n", 6);

            for (uint32_t addr = 0;
                 addr < size;
                 addr += BUF_SIZE)
            {
                spi_flash_read(addr, buf, BUF_SIZE);

                uart_write_bytes(
                    UART_NUM,
                    (char*)buf,
                    BUF_SIZE
                );
            }

            uart_write_bytes(UART_NUM, "DONE\n", 5);
        }

        // ---------------- WRITE ----------------

        else if (strncmp((char*)cmd, "WRITE:", 6) == 0)
        {
            uint32_t size =
                atoi((char*)&cmd[6]);

            uart_write_bytes(UART_NUM, "READY\n", 6);

            uint8_t page[256];

            for (uint32_t addr = 0;
                 addr < size;
                 addr += 256)
            {
                uart_read_bytes(
                    UART_NUM,
                    page,
                    256,
                    portMAX_DELAY
                );

                if ((addr % 4096) == 0)
                    spi_flash_sector_erase(addr);

                spi_flash_page_program(
                    addr,
                    page,
                    256
                );
            }

            uart_write_bytes(UART_NUM, "DONE\n", 5);
        }
    }
}