CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
CFLAGS = -mcpu=cortex-m3 -mthumb -O0 -g

all: output.bin

output.elf: startup_stm32f411ceux.s main.c
	$(CC) $(CFLAGS) -T STM32F411CEUX_FLASH.ld startup_stm32f411ceux.s  main.c -o output.elf

output.bin: output.elf
	$(OBJCOPY) -O binary output.elf output.bin

flash: output.bin
	dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D output.bin

clean:
	rm -f *.elf *.bin
