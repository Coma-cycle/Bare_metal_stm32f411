#include "Sep_5_MPU_lib.h"

/* ------------------------------------------------------------------ */
/*  Clock                                                              */
/* ------------------------------------------------------------------ */
void Clk_Init_HSE(void)
{
    RCC->CR |= (1U << 16);                          /* HSEON */
    while (!(RCC->CR & (1U << 17)));                /* HSERDY */

    RCC->CFGR &= ~(0xFU << 4);                      /* HPRE  = /1 */
    RCC->CFGR &= ~(0x7U << 10);                     /* PPRE1 = /1 */
    RCC->CFGR &= ~(0x7U << 13);                     /* PPRE2 = /1 */

    RCC->CFGR &= ~(0x3U << 0);
    RCC->CFGR |=  (0x1U << 0);                      /* SW = HSE */
    while ((RCC->CFGR & (0x3U << 2)) != (0x1U << 2));
}

/* ------------------------------------------------------------------ */
/*  UART2  (PA2=TX, PA3=RX, 115200 8N1)                                */
/* ------------------------------------------------------------------ */
void UART2_Init(void)
{
    RCC->AHB1ENR |= (1U << 0);                      /* GPIOA */
    RCC->APB1ENR |= (1U << 17);                     /* USART2 */

    GPIOA->MODER &= ~((3U << (2*2)) | (3U << (3*2)));
    GPIOA->MODER |=  ((2U << (2*2)) | (2U << (3*2)));

    GPIOA->AFR[0] &= ~((0xFU << (4*2)) | (0xFU << (4*3)));
    GPIOA->AFR[0] |=  ((0x7U << (4*2)) | (0x7U << (4*3))); /* AF7 */

    USART2->BRR = (uint16_t)((APB1_CLK_HZ + (UART_BAUD / 2)) / UART_BAUD);
    USART2->CR1 = (1U << 3) | (1U << 2) | (1U << 13); /* TE | RE | UE */
}

void UART_SendChar(char c)
{
    while (!(USART2->SR & (1U << 7)));
    USART2->DR = (uint8_t)c;
}

void UART_SendString(const char *str)
{
    while (*str) UART_SendChar(*str++);
}

/* ------------------------------------------------------------------ */
/*  I2C helpers                                                        */
/* ------------------------------------------------------------------ */
#define I2C_TIMEOUT_ITERS  100000UL

static void dbg(const char *msg)
{
    UART_SendString(msg);
    UART_SendString("\r\n");
}

static uint8_t I2C_WaitFlag(volatile uint32_t *reg, uint32_t mask, const char *where)
{
    uint32_t count = I2C_TIMEOUT_ITERS;
    while (!(*reg & mask)) {
        if (--count == 0) {
            UART_SendString("I2C timeout: ");
            UART_SendString(where);
            UART_SendString("\r\n");
            return 0;
        }
    }
    return 1;
}

static void I2C_WaitStop(void)
{
    /* Wait until hardware clears the STOP bit */
    uint32_t t = I2C_TIMEOUT_ITERS;
    while ((I2C1->CR1 & (1U << 9)) && --t);

    /* Small extra delay so the lines really go high */
    for (volatile int i = 0; i < 300; i++);
}

static uint8_t I2C_WaitBusFree(void)
{
    uint32_t count = I2C_TIMEOUT_ITERS * 2;

    while (I2C1->SR2 & (1U << 1)) {                 /* BUSY */
        if (--count == 0) {
            UART_SendString("I2C timeout: bus busy → recovering\r\n");
            I2C_BusRecover();

            /* After recovery try once more */
            count = 8000;
            while ((I2C1->SR2 & (1U << 1)) && --count);
            return (count != 0);
        }
    }
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Bus recovery                                                       */
/* ------------------------------------------------------------------ */
void I2C_BusRecover(void)
{
    I2C1->CR1 &= ~(1U << 0);                        /* PE = 0 */

    /* PB6/PB7 as open-drain outputs */
    GPIOB->MODER  &= ~((3U << (6*2)) | (3U << (7*2)));
    GPIOB->MODER  |=  ((1U << (6*2)) | (1U << (7*2)));
    GPIOB->OTYPER |=  ((1U << 6) | (1U << 7));
    GPIOB->PUPDR  &= ~((3U << (6*2)) | (3U << (7*2)));
    GPIOB->PUPDR  |=  ((1U << (6*2)) | (1U << (7*2)));

    /* 9 clock pulses */
    GPIOB->BSRR = (1U << 6) | (1U << 7);            /* SCL=1 SDA=1 */
    for (int i = 0; i < 9; i++) {
        GPIOB->BSRR = (1U << (6 + 16));             /* SCL = 0 */
        for (volatile int d = 0; d < 250; d++);
        GPIOB->BSRR = (1U << 6);                    /* SCL = 1 */
        for (volatile int d = 0; d < 250; d++);
    }

    /* STOP condition */
    GPIOB->BSRR = (1U << (7 + 16));                 /* SDA = 0 */
    for (volatile int d = 0; d < 120; d++);
    GPIOB->BSRR = (1U << 6);                        /* SCL = 1 */
    for (volatile int d = 0; d < 120; d++);
    GPIOB->BSRR = (1U << 7);                        /* SDA = 1 */

    /* Restore AF4 */
    GPIOB->MODER  &= ~((3U << (6*2)) | (3U << (7*2)));
    GPIOB->MODER  |=  ((2U << (6*2)) | (2U << (7*2)));
    GPIOB->AFR[0] &= ~((0xFU << (4*6)) | (0xFU << (4*7)));
    GPIOB->AFR[0] |=  ((0x4U << (4*6)) | (0x4U << (4*7)));

    /* Soft-reset peripheral */
    I2C1->CR1 |=  (1U << 15);
    I2C1->CR1 &= ~(1U << 15);

    I2C1->CR2   = (uint16_t)(APB1_CLK_HZ / 1000000UL);
    I2C1->CCR   = (uint16_t)(APB1_CLK_HZ / (2 * I2C_SPEED_HZ));
    I2C1->TRISE = (uint16_t)((APB1_CLK_HZ / 1000000UL) + 1);
    I2C1->CR1  |= (1U << 0);                        /* PE = 1 */
}

/* ------------------------------------------------------------------ */
/*  I2C Init                                                           */
/* ------------------------------------------------------------------ */
void I2C_Init(void)
{
    RCC->AHB1ENR |= (1U << 1);                      /* GPIOB */
    RCC->APB1ENR |= (1U << 21);                     /* I2C1 */

    /* PB6/PB7 AF4, open-drain, pull-up, high-speed */
    GPIOB->MODER   &= ~((3U << (6*2)) | (3U << (7*2)));
    GPIOB->MODER   |=  ((2U << (6*2)) | (2U << (7*2)));
    GPIOB->OTYPER  |=  ((1U << 6) | (1U << 7));
    GPIOB->OSPEEDR |=  ((3U << (6*2)) | (3U << (7*2)));
    GPIOB->PUPDR   &= ~((3U << (6*2)) | (3U << (7*2)));
    GPIOB->PUPDR   |=  ((1U << (6*2)) | (1U << (7*2)));

    GPIOB->AFR[0] &= ~((0xFU << (4*6)) | (0xFU << (4*7)));
    GPIOB->AFR[0] |=  ((0x4U << (4*6)) | (0x4U << (4*7)));

    /* Soft reset */
    I2C1->CR1 |=  (1U << 15);
    I2C1->CR1 &= ~(1U << 15);

    I2C1->CR2   = (uint16_t)(APB1_CLK_HZ / 1000000UL);
    I2C1->CCR   = (uint16_t)(APB1_CLK_HZ / (2 * I2C_SPEED_HZ));
    I2C1->TRISE = (uint16_t)((APB1_CLK_HZ / 1000000UL) + 1);

    I2C1->CR1 |= (1U << 0);                         /* PE */
}

/* ------------------------------------------------------------------ */
/*  Write one register                                                 */
/* ------------------------------------------------------------------ */
void I2C_WriteReg(uint8_t dev, uint8_t reg, uint8_t val)
{
    if (!I2C_WaitBusFree()) return;

    I2C1->CR1 |= (1U << 8);                         /* START */
    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 0), "SB write")) return;

    I2C1->DR = dev;
    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 1), "ADDR write")) return;
    (void)I2C1->SR1; (void)I2C1->SR2;

    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 7), "TXE reg")) return;
    I2C1->DR = reg;

    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 7), "TXE val")) return;
    I2C1->DR = val;

    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 2), "BTF write")) return;

    I2C1->CR1 |= (1U << 9);                         /* STOP */
    I2C_WaitStop();
}

/* ------------------------------------------------------------------ */
/*  Read N bytes                                                       */
/* ------------------------------------------------------------------ */
void I2C_ReadBytes(uint8_t dev, uint8_t reg, uint8_t *buf, uint8_t len)
{
    if (len == 0) return;

    dbg("1: enter I2C_ReadBytes");

    if (!I2C_WaitBusFree()) {
        dbg("1a: bus never went free");
        return;
    }
    dbg("2: bus confirmed free");

    /* ----- Write phase: send register address ----- */
    I2C1->CR1 |= (1U << 8);                         /* START */
    dbg("3: START bit set");
    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 0), "SB (write phase)")) {
        dbg("3a: SB never set"); return;
    }

    I2C1->DR = dev;
    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 1), "ADDR (write phase)")) {
        dbg("5a: ADDR never set — device did not ACK"); return;
    }
    (void)I2C1->SR1; (void)I2C1->SR2;
    dbg("7: ADDR cleared");

    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 7), "TXE (reg pointer)")) {
        dbg("7a: TXE never set"); return;
    }
    I2C1->DR = reg;
    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 2), "BTF (reg pointer)")) {
        dbg("8a: BTF never set"); return;
    }
    dbg("9: register pointer fully sent");

    /* ----- Repeated START + Read phase ----- */
    I2C1->CR1 |= (1U << 8);                         /* repeated START */
    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 0), "SB (repeated start)")) {
        dbg("10a: repeated SB never set"); return;
    }
    dbg("11: repeated SB set");

    I2C1->DR = dev | 1;                             /* read address */
    if (!I2C_WaitFlag(&I2C1->SR1, (1U << 1), "ADDR (read phase)")) {
        dbg("12a: ADDR never set — device did not ACK read address"); return;
    }
    dbg("13: ADDR set (device ACKed read address)");

    if (len == 1) {
        /* Single-byte read */
        I2C1->CR1 &= ~(1U << 10);                   /* ACK = 0 */
        (void)I2C1->SR1; (void)I2C1->SR2;           /* clear ADDR */
        I2C1->CR1 |=  (1U << 9);                    /* STOP */

        if (!I2C_WaitFlag(&I2C1->SR1, (1U << 6), "RXNE single")) {
            dbg("15a: RXNE never set"); return;
        }
        buf[0] = I2C1->DR;
        I2C_WaitStop();
        dbg("16: single byte read complete");
    }
    else {
        /* Multi-byte read */
        I2C1->CR1 |= (1U << 10);                    /* ACK = 1 */
        (void)I2C1->SR1; (void)I2C1->SR2;           /* clear ADDR */
        dbg("14: ADDR cleared, ACK enabled");

        /* Read all but the last two bytes */
        for (uint8_t i = 0; i < len - 2; i++) {
            if (!I2C_WaitFlag(&I2C1->SR1, (1U << 6), "RXNE bulk")) {
                dbg("bulk: RXNE never set"); return;
            }
            buf[i] = I2C1->DR;
        }
        dbg("17: bulk loop finished");

        /* Wait until both remaining bytes are ready (BTF) */
        if (!I2C_WaitFlag(&I2C1->SR1, (1U << 2), "BTF final pair")) {
            dbg("17a: final-pair BTF never set"); return;
        }

        /* NACK + STOP */
        I2C1->CR1 &= ~(1U << 10);                   /* ACK = 0 */
        I2C1->CR1 |=  (1U << 9);                    /* STOP */
        dbg("19: ACK cleared, STOP set");

        buf[len - 2] = I2C1->DR;
        buf[len - 1] = I2C1->DR;

        I2C_WaitStop();
        dbg("21: final byte read, function complete");
    }
}

/* ------------------------------------------------------------------ */
/*  MPU-6050 high-level functions                                      */
/* ------------------------------------------------------------------ */
void MPU6050_WakeUp(void)
{
    I2C_WriteReg(MPU6050_ADDR, MPU6050_REG_PWR_MGMT_1, 0x00);
}

void MPU6050_ConfigAccel(uint8_t range)
{
    I2C_WriteReg(MPU6050_ADDR, MPU6050_REG_ACCEL_CFG, (range & 0x03) << 3);
}

void MPU6050_ConfigGyro(uint8_t range)
{
    I2C_WriteReg(MPU6050_ADDR, MPU6050_REG_GYRO_CFG, (range & 0x03) << 3);
}

void MPU6050_ConfigDLPF(uint8_t dlpf)
{
    I2C_WriteReg(MPU6050_ADDR, MPU6050_REG_CONFIG, dlpf & 0x07);
}

void MPU6050_Init(void)
{
    MPU6050_WakeUp();
    MPU6050_ConfigDLPF(3);          /* ~44 Hz */
    MPU6050_ConfigAccel(0);         /* ±2 g */
    MPU6050_ConfigGyro(0);          /* ±250 dps */
}

void MPU6050_ReadAll(mpu_raw_t *out)
{
    uint8_t buf[14];
    I2C_ReadBytes(MPU6050_ADDR, MPU6050_REG_ACCEL_XOUT_H, buf, 14);

    out->ax   = (int16_t)((buf[0]  << 8) | buf[1]);
    out->ay   = (int16_t)((buf[2]  << 8) | buf[3]);
    out->az   = (int16_t)((buf[4]  << 8) | buf[5]);
    out->temp = (int16_t)((buf[6]  << 8) | buf[7]);
    out->gx   = (int16_t)((buf[8]  << 8) | buf[9]);
    out->gy   = (int16_t)((buf[10] << 8) | buf[11]);
    out->gz   = (int16_t)((buf[12] << 8) | buf[13]);
}

float MPU6050_AccelToG(int16_t raw)  { return raw / 16384.0f; }
float MPU6050_GyroToDps(int16_t raw) { return raw / 131.0f;   }
float MPU6050_TempToC(int16_t raw)   { return raw / 340.0f + 36.53f; }
