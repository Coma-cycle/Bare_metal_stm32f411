#Bare_metal_stm32F411CUX

Hi. I am trying to share my journey of learning stm32F411CUX, config and program it baremtal.

To compile the main.c file and genrate .elf file u can use the compiler below(u will need startup.s and liker_script.ld, and main.c files):

```
```

or u can compile it using *stm32cubeide
generete raw .bin binary file from .elf via:

```
```

put stm32F411 in DFU moe(BOOTO + NRST), u can check this via :

```
dgu-util -l
```

flash the program and run the script:

```
```
