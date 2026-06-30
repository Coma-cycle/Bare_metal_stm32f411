#Bare_metal_stm32F411CEUX

Hi. I am trying to share my journey of learning stm32F411CEUX, config and program it baremtal.

To compile the main.c file and genrate .elf file u can use the compiler below(u will need startup.s and liker_script.ld, and main.c files):

```
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -T STM32F411CEUX_FLASH.ld s>

```

or u can compile it using *stm32cubeide. :)

convert .elf to raw .bin binary:

```
arm-none-eabi-objcopy -O binary output.elf output.bin

```

put stm32F411 in DFU moe(BOOTO + NRST), u can check this via :

```
dfu-util -l

```

flash the program and run the script:

```
dfu-util -a 0 -d 0483:df11 -s 0x08000000:leave -D BM0.bin

```

the BM0 contains the script,(temp: add the datasheet info used, postponed) of making the GPIOC13 blink:

