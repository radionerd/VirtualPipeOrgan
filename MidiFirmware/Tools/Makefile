

STFLASHOK := $(shell which st-flash 1>/dev/null 2> /dev/null; echo $$?)
ifneq ($(STFLASHOK),0)
    $(error "Path to st-flash not found. Consider installing st-tools by typing: 'sudo apt install st-tools'")
endif


.PHONY: help flash_bootloader flash_vpo all

help:
	@echo "Options:"
	@echo "  make all"
	@echo "  make flash_bootloader"
	@echo "  make flash_vpo" 
	
all:    flash_bootloader flash_vpo

flash_bootloader:
	st-flash write generic_boot20_pc13.bin 0x8000000
flash_vpo:
	st-flash write MidiFirmware.ino.bin 0x8002000
	

