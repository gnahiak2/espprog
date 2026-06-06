// main.c
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_err.h"

// ESP32-S3 FSPI Pins
#define PIN_CS   10
#define PIN_MOSI 11
#define PIN_CLK  12
#define PIN_MISO 13

#define SPI_HOST_USED SPI2_HOST
#define SPI_SPEED_HZ 1000000

// esprog protocol config
#define MAGIG_REQ 0x4550 // "EP"
#define MAGIC_RES 0x4552 // "ER"
#define PROTO_VER 1
#define MAX_PAYLOAD 4096

// esprog protocol commands
// Sent from CLI to ESP32-S3
#define CMD_PING      0x01
#define CMD_INFO      0x02
#define CMD_JEDEC_ID  0x03
#define CMD_STATUS    0x04
#define CMD_READ      0x05
#define CMD_WRITE     0x06
#define CMD_ERASE_4K  0x07

// Responce status codes
#define ST_OK         0x00
#define ST_BAD_MAGIC  0x01
#define ST_BAD_CRC    0x02
#define ST_BAD_CMD    0x03
#define ST_BAD_LEN    0x04
#define ST_SPI_ERR    0x05
#define ST_NO_ME      0x06

// SPI NOR commands
// Sent from ESP32-S3 to external flash chip
#define SPI_CMD_READ_JEDEC_ID  0x9F
#define SPI_CMD_READ_STATUS1   0x05
#define SPI_CMD_WRITE_ENABLE   0x06
#define SPI_CMD_WRITE_DISABLE  0x04
#define SPI_CMD_READ_DATA      0x03
#define SPI_CMD_PAGE_PROGRAM   0x02
#define SPI_CMD_ERASE_4K       0x20
#define SPI_CMD_READ_SFDP      0x5A

typedef struct __attribute__((packed)) {
    uint16_t magic;
    uint8_t version;
    uint8_t command;
    uint32_t sequence;
    uint32_t address;
    uint32_t length;
    uint32_t crc32;
} packet_header_t;

static spi_device_handle_t flash_dev;

// Utility functions

static uint32_t crc32_calc(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];

        
    }
}