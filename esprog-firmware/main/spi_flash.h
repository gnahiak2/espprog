// spi_flash.h
#pragma once

#include <stdint.h>
#include <stdbool.h>

void spi_flash_init(void);
void spi_read_jedec(uint8_t *buf, bool *quad_supported);

void spi_flash_read_fast(uint32_t addr, uint8_t *buf, uint32_t len);
void spi_flash_page_program_fast(uint32_t addr, uint8_t *buf, uint32_t len);
void spi_flash_sector_erase_fast(uint32_t addr);

void spi_flash_read(uint32_t addr, uint8_t *buf, uint32_t len);
void spi_flash_write_enable(void);
void spi_flash_wait_busy(void);
void spi_flash_sector_erase(uint32_t addr);
void spi_flash_page_program(uint32_t addr, const uint8_t *buf, uint32_t len);

bool spi_flash_supports_qspi(void);