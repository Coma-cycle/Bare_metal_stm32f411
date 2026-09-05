#ifndef SEP_5_MPU_LIB_H
#define SEP_5_MPU_LIB_H

#include "stm32f4xx.h"

/* ---- Clock config ---- */
#define SYS_CLK_HSE_HZ   25000000UL
#define APB1_CLK_HZ      SYS_CLK_HSE_HZ
#define UART_BAUD        115200UL

#define MPU6050_ADDR     (0x68 << 1)   /* 0xD0 when AD0 = GND */
#define I2C_SPEED_HZ     100000UL

/* ---- Register map ---- */
#define MPU6050_REG_PWR_MGMT_1   0x6B
#define MPU6050_REG_ACCEL_CFG    0x1C
#define MPU6050_REG_GYRO_CFG     0x1B
#define MPU6050_REG_CONFIG       0x1A
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_WHO_AM_I     0x75

typedef struct {
    int16_t ax, ay, az;
    int16_t temp;
    int16_t gx, gy, gz;
} mpu_raw_t;

/* Clock */
void Clk_Init_HSE(void);

/* UART */
void UART2_Init(void);
void UART_SendChar(char c);
void UART_SendString(const char *str);

/* I2C */
void I2C_Init(void);
void I2C_BusRecover(void);
void I2C_WriteReg(uint8_t dev, uint8_t reg, uint8_t val);
void I2C_ReadBytes(uint8_t dev, uint8_t reg, uint8_t *buf, uint8_t len);

/* MPU-6050 */
void MPU6050_WakeUp(void);
void MPU6050_ConfigAccel(uint8_t range);
void MPU6050_ConfigGyro(uint8_t range);
void MPU6050_ConfigDLPF(uint8_t dlpf);
void MPU6050_Init(void);
void MPU6050_ReadAll(mpu_raw_t *out);

float MPU6050_AccelToG(int16_t raw);
float MPU6050_GyroToDps(int16_t raw);
float MPU6050_TempToC(int16_t raw);

#endif /* SEP_5_MPU_LIB_H */
