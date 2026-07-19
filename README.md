#Bare_metal_stm32F411CEUX   (in progress)

Hi. I am trying to share my journey of learning stm32F411CEUX,while Configuring and programming it baremtal.

To compile the main.c file and generate .elf file u can use the compiler below(u will need startup.s and linker_script.ld, and main.c files):

```
 arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -T stm32f411ceux_FLASH.ld main.c startup_stm32f411xe.s -o output.elf


```

or u can compile it using *stm32cubeide. :)

convert .elf to raw binary .bin:

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
[temp: add the datasheet info used, postponed) ]

the BM0.bin contains the script that will make GPIOC13 blink:

<img width="480" height="270" alt="blinkC13" src="https://github.com/user-attachments/assets/44c17157-0e6b-4c1e-ae08-45a34f26e019" />

In directory SPI , the first file is SPI_0.elf , which can be run on the black pill to check if the spi (serial peripheral interface) works, for this u need to connect the soldered pin , PA6 (MIMSO) and PA7 (MOSI) via a wire together , if the LED GPIOC13 blink the spi successfully transmitted data .

if you wanna run the .elf file using ST-link V2 : 

```
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program <YOUR_FILE_NAME>.elf verify reset exit"
```
the desired outpt of SPI_0.elf

<img width="480" height="270" alt="output" src="https://github.com/user-attachments/assets/13ff35c1-40fb-4ef9-963b-9999c3265728" />

