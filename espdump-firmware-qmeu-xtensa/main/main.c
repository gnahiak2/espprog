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
        flash_memory[i] = (uint8_t)((i ^ (i >> 8)) & 0xFF)
    }
    ESP_LOGI(TAG, "Simulated flash initiated (%d bytes)", FLASH_SIZE);
}

static void sim_flash_command(
    const uint8_t *cmd_bud,
    int cmd_len,
    uint8_t *resp_buf,
    int *resp_len
)
{
    if (cmd_len < 1) {
        *resp_len = 0;
        return;
    }

    uint8_t cmd = cmd_buf[0];

    switch (cmd) {
        case CMD_JEDEC: {
            // JEDEC ID: manufecturer = 0xEF, device = 0x40, capacity = 0x18 (16MB)
            resp_buf[0] = 0xEF;
            resp_buf[1] = 0x40;
            resp_buf[2] = 0x18;
            *resp_len = 3;
            ESP_LOGI(TAG_FLASH, "JEDEC read");
            break;
        }

        case CMD_FAST_READ: {
            if (cmd_len < 5) {
                ESP_LOGW(TAG_FLASH, "FAST_READ: incomplete");
                *resp_len = 0;
                break;
            }

            uint32_t addr = ((uint32_t)cmd_buf[1] << 16) |
                            ((uint32_t)cmd_buf[2] << 8) |
                            ((uint32_t)cmd_buf[3]);
            
            if (addr >= FLASH_SIZE) {
                ESP_LOGW(TAG_FLASH, "FAST_READ: addr out of bounds 0x%06X", addr);
                *resp_len = 0;
                break;
            }

            int to_read

        }
    }
}