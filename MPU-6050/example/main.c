#include "Sep_5_MPU_lib.h"
#include <stdio.h>
#include <stdint.h>

/* Formats a byte as "0xNN\r\n" into buf. buf must be >= 8 bytes. */
static void format_hex_byte(uint8_t val, char *buf) {
    const char hex_digits[] = "0123456789ABCDEF";
    buf[0] = '0';
    buf[1] = 'x';
    buf[2] = hex_digits[(val >> 4) & 0xF];
    buf[3] = hex_digits[val & 0xF];
    buf[4] = '\r';
    buf[5] = '\n';
    buf[6] = '\0';
}
static void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 4000; i++) {
        __asm volatile ("nop");
    }
}


int main(void) {
    SCB->CPACR |= (0xFUL << 20);
    __asm volatile ("DSB");
    __asm volatile ("ISB");
    /* Setup HSE clk 25MHz */
    Clk_Init_HSE();

    /* UART */
    UART2_Init();
    UART_SendString("=====UART INITIALIZED====\r\n");

    /* I2C */
    I2C_Init();
    I2C_BusRecover();          // ← important
    MPU6050_Init();
    uint8_t who = 0;
    I2C_ReadBytes(MPU6050_ADDR, 0x75, &who, 1);   /* WHO_AM_I */

    char msg[8];
    format_hex_byte(who, msg);
    UART_SendString("WHO_AM_I: ");
    UART_SendString(msg);

    MPU6050_Init();
    uint32_t loop_count = 0;

    while (1) {
        mpu_raw_t d;
        MPU6050_ReadAll(&d);

        float ax_g = MPU6050_AccelToG(d.ax);
        float ay_g = MPU6050_AccelToG(d.ay);
        float az_g = MPU6050_AccelToG(d.az);

        char buff[80];
        sprintf(buff, "[%lu] X=%.3f g, Y=%.3f g, Z=%.3f g\r\n",
                (unsigned long)loop_count++, ax_g, ay_g, az_g);
        UART_SendString(buff);

        delay_ms(300);   /* now slow enough to actually read */
    }
}
