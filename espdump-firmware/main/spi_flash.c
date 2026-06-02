// spi_flash.c
#include "spi_flash.h"

#include "driver/spi_master.h"
#include "esp_err.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define PIN_MOSI 11
#define PIN_MISO 13
#define PIN_CLK  12
#define PIN_CS   10
#define PIN_WP   9
#define PIN_HOLD 14

#define CMD_JEDEC_ID         0x9F
#define CMD_READ_STATUS_1    0x05
#define CMD_READ_STATUS_2    0x35
#define CMD_WRITE_ENABLE     0x06
#define CMD_READ_FAST_QOUT   0x6B
#define CMD_SECTOR_ERASE     0x20
#define CMD_PAGE_PROGRAM     0x02
#define CMD_WRITE_STATUS_2   0x31

static spi_device_handle_t spi;

static esp_err_t spi_cmd(const uint8_t *tx, uint8_t *rx, int bytes)
{
    spi_transaction_t t = {0};
    t.length = bytes * 8;
    t.tx_buffer = tx;
    t.rx_buffer = rx;
    return spi_device_transmit(spi, &t);
}

static bool winbond_qe_enabled(void)
{
    uint8_t tx[2] = { CMD_READ_STATUS_2, 0 };
    uint8_t rx[2] = { 0 };

    if (spi_cmd(tx, rx, 2) != ESP_OK) {
        return false;
    }

    return (rx[1] & (1 << 1)) != 0;
}

static bool winbond_enable_qe(void)
{
    if (winbond_qe_enabled()) {
        return true;
    }

    uint8_t tx[2] = { CMD_READ_STATUS_2, 0 };
    uint8_t rx[2] = { 0 };

    if (spi_cmd(tx, rx, 2) != ESP_OK) {
        return false;
    }

    spi_flash_write_enable();

    uint8_t new_sr2 = (uint8_t)(rx[1] | (1 << 1));
    uint8_t wr[2] = { CMD_WRITE_STATUS_2, new_sr2 };

    if (spi_cmd(wr, NULL, 2) != ESP_OK) {
        return false;
    }

    spi_flash_wait_busy();
    return winbond_qe_enabled();
}

void spi_flash_init(void)
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = PIN_WP,
        .quadhd_io_num = PIN_HOLD,
        .max_transfer_sz = 65544,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_CS,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));
}

void spi_read_jedec(uint8_t *buf, bool *quad_supported)
{
    uint8_t tx[4] = { CMD_JEDEC_ID, 0, 0, 0 };
    uint8_t rx[4] = { 0 };

    if (spi_cmd(tx, rx, 4) != ESP_OK) {
        buf[0] = buf[1] = buf[2] = 0;
        if (quad_supported) {
            *quad_supported = false;
        }
        return;
    }

    buf[0] = rx[1];
    buf[1] = rx[2];
    buf[2] = rx[3];

    if (buf[0] == 0xEF) {
        if (quad_supported) {
            *quad_supported = winbond_enable_qe();
        }
    } else {
        if (quad_supported) {
            *quad_supported = false;
        }
    }
}

void spi_flash_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    while (len > 0) {
        uint32_t chunk = len > 4096 ? 4096 : len;

        uint8_t *tx = calloc(1, 5 + chunk);
        uint8_t *rx = calloc(1, 5 + chunk);
        if (!tx || !rx) {
            free(tx);
            free(rx);
            return;
        }

        tx[0] = CMD_READ_FAST_QOUT;
        tx[1] = (uint8_t)((addr >> 16) & 0xFF);
        tx[2] = (uint8_t)((addr >> 8) & 0xFF);
        tx[3] = (uint8_t)(addr & 0xFF);
        tx[4] = 0x00;

        spi_transaction_t t = {0};
        t.length = (5 + chunk) * 8;
        t.tx_buffer = tx;
        t.rx_buffer = rx;

        if (spi_device_transmit(spi, &t) == ESP_OK) {
            memcpy(buf, rx + 5, chunk);
        }

        free(tx);
        free(rx);

        addr += chunk;
        buf += chunk;
        len -= chunk;
    }
}

void spi_flash_write_enable(void)
{
    uint8_t cmd = CMD_WRITE_ENABLE;
    spi_cmd(&cmd, NULL, 1);
}

void spi_flash_wait_busy(void)
{
    uint8_t tx[2] = { CMD_READ_STATUS_1, 0 };
    uint8_t rx[2];

    do {
        spi_cmd(tx, rx, 2);
    } while (rx[1] & 1);
}

void spi_flash_sector_erase(uint32_t addr)
{
    spi_flash_write_enable();

    uint8_t tx[4] = {
        CMD_SECTOR_ERASE,
        (uint8_t)(addr >> 16),
        (uint8_t)(addr >> 8),
        (uint8_t)addr
    };

    spi_cmd(tx, NULL, 4);
    spi_flash_wait_busy();
}

void spi_flash_page_program(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    if (len > 256) {
        len = 256;
    }

    spi_flash_write_enable();

    uint8_t tx[260];
    tx[0] = CMD_PAGE_PROGRAM;
    tx[1] = (uint8_t)(addr >> 16);
    tx[2] = (uint8_t)(addr >> 8);
    tx[3] = (uint8_t)addr;
    memcpy(&tx[4], buf, len);

    spi_transaction_t t = {0};
    t.length = (4 + len) * 8;
    t.tx_buffer = tx;

    spi_device_transmit(spi, &t);
    spi_flash_wait_busy();
}