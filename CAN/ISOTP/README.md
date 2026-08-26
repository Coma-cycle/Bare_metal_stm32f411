# mcp2515_can

Bare-metal **MCP2515** (SPI CAN controller + TJA1050 transceiver) driver with a from-scratch **ISO-TP (ISO 15765-2)** transport layer for STM32F4-family MCUs (“Black Pill” boards).

No HAL, no Arduino wrapper — direct register access, one SPI peripheral + one interrupt line.

## Features

- Full MCP2515 register-level driver  
  (reset, mode switching, TX/RX buffers, status & bit-modify instructions)
- Interrupt-driven RX with RXB0→RXB1 BUKT rollover  
  (extra buffering so back-to-back frames are not silently dropped)
- ISO-TP (ISO 15765-2)  
  - Single Frame / First Frame / Consecutive Frames  
  - Flow Control (CTS / WAIT / OVFLW)  
  - STmin pacing & block-size handling  
  - Stale-transfer timeouts
- Minimal BSP layer (`SysTick` millis, SPI1, UART2) so the protocol code has zero direct register dependencies of its own

## Status

| Component                              | State          |
|----------------------------------------|----------------|
| SPI / register-level MCP2515 driver    | ✅ Working     |
| Interrupt-driven RX, TX via first-free | ✅ Working     |
| ISO-TP single-frame & multi-frame      | ✅ Tested      |
| Hardware mask/filter configuration     | 🚧 In progress (currently accept-all) |
| >2-node bus testing                    | ⏳ Planned     |

## Project layout
