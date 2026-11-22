/* SPDX-License-Identifier: MPL-2.0
 * I2C adaptation of the OLED port from Sheep118/WouoUI-PageVersion.
 * Upstream: https://github.com/Sheep118/WouoUI-PageVersion
 * Transport: STM32F103 I2C1, PB6=SCL, PB7=SDA, 3.3 V module supply.
 * The directory may retain its original OLED_SPI name for this first change.
 */
#include "oled_port.h"
#include "stm32f10x.h"
#include "main.h"

#if OLED_DRIVER != SSD1306 && OLED_DRIVER != SH1106
#error Unsupported OLED_DRIVER
#endif

#define OLED_I2C_TIMEOUT_LOOPS 50000UL
#define OLED_CONTROL_COMMAND  0x00U
#define OLED_CONTROL_DATA     0x40U

static uint8_t oled_ready = 0;
static uint8_t oled_address = 0;
static uint8_t oled_error = OLED_ERROR_NOT_INITIALIZED;

static void OLED_StopTransfer(void)
{
    I2C_GenerateSTOP(I2C1, ENABLE);
    I2C_ClearFlag(I2C1, I2C_FLAG_AF | I2C_FLAG_BERR |
                        I2C_FLAG_ARLO | I2C_FLAG_OVR);
}

static uint8_t OLED_WaitEvent(uint32_t event)
{
    uint32_t remaining = OLED_I2C_TIMEOUT_LOOPS;
    for (;;) {
        if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF) == SET) {
            oled_error = OLED_ERROR_NO_ACK;
            return 0;
        }
        if ((I2C1->SR1 & (I2C_SR1_BERR | I2C_SR1_ARLO | I2C_SR1_OVR)) != 0) {
            oled_error = OLED_ERROR_BUS;
            return 0;
        }
        if (I2C_CheckEvent(I2C1, event) == SUCCESS) {
            return 1;
        }
        if (--remaining == 0) {
            oled_error = OLED_ERROR_TIMEOUT;
            return 0;
        }
    }
}

static uint8_t OLED_StartWrite(uint8_t address)
{
    uint32_t remaining = OLED_I2C_TIMEOUT_LOOPS;
    oled_error = OLED_ERROR_NONE;
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) == SET) {
        if (--remaining == 0) {
            oled_error = OLED_ERROR_BUS_BUSY;
            return 0;
        }
    }
    I2C_ClearFlag(I2C1, I2C_FLAG_AF | I2C_FLAG_BERR |
                        I2C_FLAG_ARLO | I2C_FLAG_OVR);
    I2C_GenerateSTART(I2C1, ENABLE);
    if (!OLED_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) {
        OLED_StopTransfer();
        return 0;
    }
    /* SPL expects the seven-bit address shifted left by one bit. */
    I2C_Send7bitAddress(I2C1, (uint8_t)(address << 1), I2C_Direction_Transmitter);
    if (!OLED_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        OLED_StopTransfer();
        return 0;
    }
    return 1;
}

static uint8_t OLED_SendByte(uint8_t value)
{
    I2C_SendData(I2C1, value);
    /* Wait for BTF before issuing STOP, including for the final byte. */
    return OLED_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED);
}

static uint8_t OLED_WriteBuffer(uint8_t control, const uint8_t *data, uint16_t length)
{
    uint16_t index;
    if (!OLED_StartWrite(oled_address)) {
        return 0;
    }
    if (!OLED_SendByte(control)) {
        OLED_StopTransfer();
        return 0;
    }
    for (index = 0; index < length; ++index) {
        if (!OLED_SendByte(data[index])) {
            OLED_StopTransfer();
            return 0;
        }
    }
    OLED_StopTransfer();
    return 1;
}

static void OLED_I2CInit(void)
{
    GPIO_InitTypeDef pins;
    I2C_InitTypeDef bus;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    I2C_DeInit(I2C1);

    GPIO_StructInit(&pins);
    pins.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    pins.GPIO_Mode = GPIO_Mode_AF_OD;
    pins.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &pins);

    I2C_StructInit(&bus);
    bus.I2C_ClockSpeed = OLED_I2C_CLOCK_HZ;
    bus.I2C_Mode = I2C_Mode_I2C;
    bus.I2C_DutyCycle = I2C_DutyCycle_2;
    bus.I2C_OwnAddress1 = 0;
    bus.I2C_Ack = I2C_Ack_Enable;
    bus.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &bus);
    I2C_Cmd(I2C1, ENABLE);
}

void OLED_Init(void)
{
    static const uint8_t commands[] = {
        0xAE,             /* Display off. */
        0xD5, 0x80,       /* Display clock. */
        0xA8, 0x3F,       /* 64 display rows. */
        0xD3, 0x00,       /* Display offset. */
        0x40,             /* Start line zero. */
#if OLED_DRIVER == SSD1306
        0x8D, 0x14,       /* SSD1306 internal charge pump. */
        0x20, 0x02,       /* Page addressing. */
#else
        0xAD, 0x8B,       /* SH1106 DC-DC converter. */
#endif
        0xA1, 0xC8,       /* Segment and COM orientation. */
        0xDA, 0x12,
        0x81, 0xCF,       /* Contrast. */
        0xD9, 0xF1,       /* Pre-charge. */
        0xDB, 0x40,       /* VCOM level. */
        0xA4, 0xA6,       /* RAM content, normal pixels. */
        0xAF              /* Display on. */
    };
    oled_ready = 0;
    oled_address = 0;
    oled_error = OLED_ERROR_NOT_INITIALIZED;
    OLED_I2CInit();
    delay_ms(100);

    if (OLED_StartWrite(0x3C)) {
        oled_address = 0x3C;
        OLED_StopTransfer();
    } else if (OLED_StartWrite(0x3D)) {
        oled_address = 0x3D;
        OLED_StopTransfer();
    } else {
        return;
    }

    if (OLED_WriteBuffer(OLED_CONTROL_COMMAND, commands, (uint16_t)sizeof(commands))) {
        oled_ready = 1;
    }
}

void OLED_SendBuff(uint8_t buff[8][128])
{
    uint8_t page;
    uint8_t position[3];
    if (!oled_ready) {
        return;
    }
    for (page = 0; page < 8; ++page) {
        position[0] = (uint8_t)(0xB0 | page);
#if OLED_DRIVER == SSD1306
        position[1] = 0x00;
#else
        position[1] = 0x02;
#endif
        position[2] = 0x10;
        if (!OLED_WriteBuffer(OLED_CONTROL_COMMAND, position, 3) ||
            !OLED_WriteBuffer(OLED_CONTROL_DATA, buff[page], 128)) {
            oled_ready = 0;
            return;
        }
    }
}

uint8_t OLED_IsReady(void) { return oled_ready; }
uint8_t OLED_GetAddress(void) { return oled_address; }
uint8_t OLED_GetLastError(void) { return oled_error; }
