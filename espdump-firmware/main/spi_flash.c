#include "spi_flash.h"
#include "driver/spi_master.h"

#define PIN_MOSI 11
#define PIN_MISO 13
#define PIN_CLK  12
#define PIN_CS   10

static spi_device_handle_t spi;

static void spi_cmd(
    uint8_t *tx,
    uint8_t *rx,
    int bytes
)
{
    spi_transaction_t t = {
        .length = bytes * 8,
        .tx_buffer = tx,
        .rx_buffer = rx
    };

    spi_device_transmit(spi, &t);
}

void spi_flash_init()
{
    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = PIN_MISO,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_CS,
        .queue_size = 1
    };

    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &devcfg, &spi);
}

void spi_read_jedec(uint8_t *buf)
{
    uint8_t tx[4] = {0x9F, 0,0,0};
    uint8_t rx[4] = {0};

    spi_cmd(tx, rx, 4);

    buf[0] = rx[1];
    buf[1] = rx[2];
    buf[2] = rx[3];
}

void spi_flash_read(
    uint32_t addr,
    uint8_t *buf,
    uint32_t len
)
{
    uint8_t tx[4] = {
        0x03,
        (addr >> 16) & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF
    };

    spi_transaction_t t = {
        .length = (4 + len) * 8,
        .tx_buffer = tx,
        .rxlength = len * 8,
        .rx_buffer = buf
    };

    spi_device_transmit(spi, &t);
}

void spi_flash_write_enable()
{
    uint8_t cmd = 0x06;
    spi_cmd(&cmd, NULL, 1);
}

void spi_flash_wait_busy()
{
    uint8_t tx[2] = {0x05, 0};
    uint8_t rx[2];

    do {
        spi_cmd(tx, rx, 2);
    } while (rx[1] & 0x01);
}

void spi_flash_sector_erase(uint32_t addr)
{
    spi_flash_write_enable();

    uint8_t tx[4] = {
        0x20,
        (addr >> 16) & 0xFF,
        (addr >> 8) & 0xFF,
        addr & 0xFF
    };

    spi_cmd(tx, NULL, 4);

    spi_flash_wait_busy();
}

void spi_flash_page_program(
    uint32_t addr,
    uint8_t *buf,
    uint32_t len
)
{
    spi_flash_write_enable();

    uint8_t tx[260];

    tx[0] = 0x02;
    tx[1] = (addr >> 16) & 0xFF;
    tx[2] = (addr >> 8) & 0xFF;
    tx[3] = addr & 0xFF;

    for (int i = 0; i < len; i++)
        tx[4 + i] = buf[i];

    spi_transaction_t t = {
        .length = (4 + len) * 8,
        .tx_buffer = tx
    };

    spi_device_transmit(spi, &t);

    spi_flash_wait_busy();
}