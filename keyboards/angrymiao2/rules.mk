# Keyboard matrix uses shift-registers read via SPI

QUANTUM_LIB_SRC += spi_master.c

# This file intentionally left blank
DEBOUNCE_TYPE = sym_eager_pk
DEBOUNCE = 3

SRC = \
	SEGGER_RTT.c \
	SEGGER_RTT_printf.c\
	aw20216.c\
	led.c\
	ui_data.c\
	spi_flash_drv.c\
	spi_master.c\
	app_flash.c
