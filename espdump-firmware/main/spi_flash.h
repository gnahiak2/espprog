#pragma once
#include <stdint.h>

void spi_flash_init();

void spi_read_jedec(uint8_t *buf);

void spi_flash_read(
    uint32_t addr,
    uint8_t *buf,
    uint32_t len
);

void spi_flash_write_enable();

void spi_flash_wait_busy();

void spi_flash_sector_erase(uint32_t addr);

void spi_flash_page_program(
    uint32_t addr,
    uint8_t *buf,
    uint32_t len
);