// main.c
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/spi_slave.h"
#include "driver/gpio.h"

#define SPI_SLAVE_HOST HSPI_HOST

// Pins for a ESP32-S3 DevKit-C v1.1 SPI stuff
#define PIN_MOSI 11
#define PIN_MISO 13
#define PIN_CLK  12
#define PIN_CS   10

// Emulating a 16MB SPI NOR Chip
#define FLASH_SIZE (16 * 1024 * 1024)
#define SECTOR_SIZE 4096
#define PAGE_SIZE 256

static const char *TAG = "flash_emulator";

static uint8_t flash_memory[FLASH_SIZE];
static bool flash_initialized = false;

// Flash Commands
#define CMD_JEDEC      0x9F
#define CMD_FAST_READ  0x0B
#define CMD_QUAD_READ  0x6B
#define CMD_READ       0x03
#define CMD_PP         0x02
#define CMD_SE         0x20
#define CMD_WREN       0x06
#define CMD_RDSR       0x05

static uint8_t flash_status = 0x00;

static void flash_init_memory(void)
{
    // Pre-fill with recogizable pattern for testing
    for (int i = 8; i < FLASH_SIZE; i++) {
        flash_memory[i]
    }
}