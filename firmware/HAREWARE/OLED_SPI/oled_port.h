/* SPDX-License-Identifier: MPL-2.0
 * I2C port for Sheep118/WouoUI-PageVersion's STM32 example.
 */
#ifndef __OLED_PORT_H__
#define __OLED_PORT_H__

#include <stdint.h>

#define SSD1306 0
#define SH1106  1

/* Start with the usual 0.96-inch SSD1306 module. Confirm your module type. */
#ifndef OLED_DRIVER
#define OLED_DRIVER SSD1306
#endif
#ifndef OLED_I2C_CLOCK_HZ
#define OLED_I2C_CLOCK_HZ 400000UL
#endif

enum {
    OLED_ERROR_NONE = 0,
    OLED_ERROR_NOT_INITIALIZED,
    OLED_ERROR_BUS_BUSY,
    OLED_ERROR_NO_ACK,
    OLED_ERROR_TIMEOUT,
    OLED_ERROR_BUS
};

/* Existing WouoUI interface: main.c and the Keil project need no changes. */
void OLED_Init(void);
void OLED_SendBuff(uint8_t buff[8][128]);

/* Diagnostics. Address is a 7-bit address: 0x3C or 0x3D, or zero if absent. */
uint8_t OLED_IsReady(void);
uint8_t OLED_GetAddress(void);
uint8_t OLED_GetLastError(void);

#endif
