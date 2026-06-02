// main.c
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_system.h"
#include "driver/usb_serial_jtag.h"

#include "spi_flash.h"

#define TXBUF 4096
#define RXBUF 4096

// serprog commands
#define S_CMD_NOP          0x00
#define S_CMD_Q_IFACE      0x01
#define S_CMD_Q_CMDMAP     0x02
#define S_CMD_Q_PGMNAME    0x03
#define S_CMD_Q_SERBUF     0x04
#define S_CMD_Q_BUSTYPE    0x05
#define S_CMD_SYNCNOP      0x10
#define S_CMD_O_SPIOP      0x13
#define S_CMD_S_BUSTYPE    0x14
#define S_CMD_SPIOP       0x10
#define S_CMD_S_SPI_FREQ  0x24

#define S_BUSTYPE_SPI      0x01

static uint8_t rxbuf[RXBUF];
static size_t rxlen = 0;

static void usb_write_all(const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    while (len > 0) {
        int n = usb_serial_jtag_write_bytes(p, len, portMAX_DELAY);
        if (n > 0) {
            p += n;
            len -= n;
        }
    }
}

static int usb_read_exact(void *data, size_t len)
{
    uint8_t *p = (uint8_t *)data;
    size_t got = 0;
    while (got < len) {
        int n = usb_serial_jtag_read_bytes(p + got, len - got, portMAX_DELAY);
        if (n > 0) got += n;
    }
    return 0;
}

static uint8_t read_u8(void)
{
    uint8_t b;
    usb_read_exact(&b, 1);
    return b;
}

static uint32_t read_u32(void)
{
    uint8_t b[4];
    usb_read_exact(b, 4);
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

static void write_u16(uint16_t v)
{
    uint8_t b[2] = { (uint8_t)(v & 0xFF), (uint8_t)(v >> 8) };
    usb_write_all(b, 2);
}

static void write_u32(uint32_t v)
{
    uint8_t b[4] = {
        (uint8_t)(v & 0xFF),
        (uint8_t)((v >> 8) & 0xFF),
        (uint8_t)((v >> 16) & 0xFF),
        (uint8_t)((v >> 24) & 0xFF),
    };
    usb_write_all(b, 4);
}

static void serprog_handle_command(uint8_t cmd)
{
    static uint8_t tmp[TXBUF];

    switch (cmd) {
    case S_CMD_NOP:
    case S_CMD_SYNCNOP:
        usb_write_all("\x01", 1);
        break;

    case S_CMD_Q_IFACE:
        usb_write_all("\x01", 1); // interface version
        break;

    case S_CMD_Q_SERBUF:
        write_u32(TXBUF);
        break;

    case S_CMD_Q_BUSTYPE:
        usb_write_all("\x01", 1); // SPI supported
        break;

    case S_CMD_S_BUSTYPE: {
        uint8_t bustype = read_u8();
        if (bustype != S_BUSTYPE_SPI) {
            usb_write_all("\x00", 1);
        } else {
            usb_write_all("\x01", 1);
        }
        break;
    }

    case S_CMD_Q_CMDMAP: {
        uint8_t map[32] = {0};
        map[0] = (1 << S_CMD_NOP) | (1 << S_CMD_Q_IFACE) | (1 << S_CMD_Q_CMDMAP) | (1 << S_CMD_Q_PGMNAME);
        map[1] = (1 << (S_CMD_Q_SERBUF - 8)) | (1 << (S_CMD_Q_BUSTYPE - 8)) | (1 << (S_CMD_SYNCNOP - 8));
        map[2] = (1 << (S_CMD_O_SPIOP - 16)) | (1 << (S_CMD_S_BUSTYPE - 16)) | (1 << (S_CMD_S_SPI_FREQ - 16));
        usb_write_all(map, sizeof(map));
        break;
    }

    case S_CMD_Q_PGMNAME:
        usb_write_all("espdump\0", 8);
        break;

    case S_CMD_S_SPI_FREQ:
        // ignore, just acknowledge
        usb_write_all("\x01", 1);
        break;

    case S_CMD_O_SPIOP: {
        uint32_t op_len = read_u32();
        uint32_t read_len = read_u32();
        uint32_t write_len = read_u32();
        uint32_t addr = read_u32();

        if (write_len > 0) {
            if (write_len > sizeof(tmp)) {
                usb_write_all("\x00", 1);
                for (uint32_t i = 0; i < write_len; i++) (void)read_u8();
                break;
            }
            usb_read_exact(tmp, write_len);
        }

        if (read_len > 0) {
            spi_flash_read(addr, tmp, read_len);
            usb_write_all(tmp, read_len);
        }

        usb_write_all("\x01", 1);
        break;
    }

    default:
        // unknown command -> fail
        usb_write_all("\x00", 1);
        break;
    }
}

void app_main(void)
{
    spi_flash_init();

    usb_serial_jtag_driver_config_t cfg = {
        .tx_buffer_size = 65536,
        .rx_buffer_size = 4096
    };
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&cfg));

    // Winbond-only / QE setup happens in spi_read_jedec()
    uint8_t id[3];
    bool quad = false;
    spi_read_jedec(id, &quad);

    // optional: just keep running even if not Winbond
    while (1) {
        uint8_t cmd = read_u8();
        serprog_handle_command(cmd);
    }
}