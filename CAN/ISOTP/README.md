
## Wiring (Black Pill – STM32F401 / F411)

| Signal          | Pin          |
|-----------------|--------------|
| SPI1 SCK        | PA5          |
| SPI1 MISO       | PA6          |
| SPI1 MOSI       | PA7          |
| MCP2515 CS      | PA4          |
| MCP2515 INT     | PB0 (EXTI0, active-low) |
| UART2 TX        | PA3          |
| UART2 RX        | PA2          |

## Integrating into your project

1. Copy `Inc/*.h` into your include path and `Src/*.c` into your build  
   (e.g. `Core/Inc` and `Core/Src` in STM32CubeIDE).

2. Open `Inc/board_config.h` and set:
   - `CPU_CLOCK_HZ` to your actual core clock
   - `ISOTP_TX_ID` / `ISOTP_RX_ID` for this node  
     (**swap TX/RX relative to the peer node**)

3. `#include "mcp2515_can.h"` from your `main.c`.

4. **Important – ISR names**  
   This library defines `SysTick_Handler` and `EXTI0_IRQHandler`.  
   Your `startup_stm32*.s` already provides weak aliases, so the linker will pick up the library versions automatically.  
   Do **not** define your own handlers with the same names.

## Minimal example

```c
#include "mcp2515_can.h"

int main(void) {
    SysTick_Init();
    SPI_INIT_BLACK_PILL();
    UART2_Init();

    /* CNF1/CNF2/CNF3: bit-timing for your crystal + target bitrate.
     * Example values below are placeholders — compute real ones for
     * your oscillator (e.g. 8 MHz) and bitrate (e.g. 500 kbit/s). */
    if (!MCP2515_Init(0x00, 0x90, 0x02)) {
        UART_SendString("MCP2515 init FAILED\r\n");
        while (1);
    }
    EXTI0_INT_Init();

    uint8_t msg[] = "hello over isotp";
    ISOTP_Send(msg, sizeof(msg));

    while (1) {
        if (can_rx_flag) {
            can_rx_flag = 0;
            CAN_HandleRxInterrupt();
        }
        ISOTP_Poll();

        if (ISOTP_RxAvailable()) {
            uint8_t rx_buf[64];
            uint16_t n = ISOTP_RxRead(rx_buf, sizeof(rx_buf));
            UART_SendString("got: ");
            UART_SendString((char *)rx_buf);
            UART_SendString("\r\n");
        }
    }
}
