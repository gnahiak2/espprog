idf_component_register(SRCS "spi_flash.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES vfs driver esp_driver_spi esp_driver_usb_serial_jtag)
