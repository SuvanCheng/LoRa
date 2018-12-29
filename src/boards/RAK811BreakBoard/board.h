/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "stm32l1xx.h"
#include "stm32l1xx_hal.h"
#include "utilities.h"
#include "timer.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "radio.h"
#include "sx1276/sx1276.h"
#include "gps.h"
#include "lis3dh.h"
#include "gps-board.h"
#include "rtc-board.h"
#include "sx1276-board.h"
#include "uart-board.h"

#if defined( USE_USB_CDC )
#include "uart-usb-board.h"
#endif

#ifdef LORAWAN_DEBUGGER

#define DPRINTF(...)  printf(__VA_ARGS__) 
#else
#define DPRINTF(...)

#endif

/*!
 * Define indicating if an external IO expander is to be used
 */
//#define BOARD_IOE_EXT

/*!
 * Generic definition
 */
#ifndef SUCCESS
#define SUCCESS                                     1
#endif

#ifndef FAIL
#define FAIL                                        0
#endif

/*!
 * Board IO pins definitions
 */

#define LED_1                                       PA_12
#define LED_2                                       PB_4

/*!
 * Board MCU pins definitions
 */

#define RADIO_RESET                                 PB_13
#define RADIO_XTAL_EN                               PH_1

#define RADIO_MOSI                                  PA_7
#define RADIO_MISO                                  PA_6
#define RADIO_SCLK                                  PA_5
#define RADIO_NSS                                   PB_0

#define RADIO_DIO_0                                 PA_11
#define RADIO_DIO_1                                 PB_1
#define RADIO_DIO_2                                 PA_3
#define RADIO_DIO_3                                 PH_0
#define RADIO_DIO_4                                 PC_13

#define RADIO_RF_CRX_RX                             PB_6  //CRF3
#define RADIO_RF_CBT_HF                             PB_7  //CRF2 HF 
#define RADIO_RF_CTX_PA                             PA_4  //CRF1 PA 

#define OSC_LSE_IN                                  PC_14
#define OSC_LSE_OUT                                 PC_15

#define OSC_HSE_IN                                  PH_0
#define OSC_HSE_OUT                                 PH_1

//#define USB_DM                                      PA_11
//#define USB_DP                                      PA_12

#define I2C_SCL                                     PB_8
#define I2C_SDA                                     PB_9
#define LIS3DH_INT1_PIN		                    			PB_14
#define LIS3DH_INT2_PIN		                    			PB_15

#define UART_TX                                     PA_9
#define UART_RX                                     PA_10

#define GPS_UART        		            						UART_3 
#define GPS_POWER_ON_PIN		            						PA_15
#define GPS_UART_TX                                 PB_10
#define GPS_UART_RX                                 PB_11
#define GPS_PPS_PIN                                 PA_0

#define BAT_LEVEL_PIN                               PA_2
#define BAT_LEVEL_CHANNEL                           ADC_CHANNEL_2

#define ADC_LEVEL_PA1                               PA_1
#define ADC_LEVEL_PB12                              PB_12
//#define ADC_LEVEL_PB14                              PA_2
//#define ADC_LEVEL_PB15                              PB_15
#define EXTERNAL_ADC_PA1                            ADC_CHANNEL_1   //@Suvan||
#define EXTERNAL_ADC_PB12                           ADC_CHANNEL_18  //@Suvan||
//#define EXTERNAL_ADC_PB14                           ADC_CHANNEL_2  //@Suvan||
//#define EXTERNAL_ADC_PB15                           ADC_CHANNEL_21  //@Suvan||

#define SWDIO                                       PA_13
#define SWCLK                                       PA_14


/*!
 * LED GPIO pins objects
 */
extern Gpio_t Led1;
extern Gpio_t Led2;

/*!
 * MCU objects
 */
extern Adc_t Adc;
extern I2c_t I2c;
extern Uart_t Uart1;
extern Uart_t GpsUart;
#if defined( USE_USB_CDC )
extern Uart_t UartUsb;
#endif

/*!
 * Possible power sources
 */
enum BoardPowerSources
{
    USB_POWER = 0,
    BATTERY_POWER,
};

/*!
 * \brief Disable interrupts
 *
 * \remark IRQ nesting is managed
 */
void BoardDisableIrq( void );

/*!
 * \brief Enable interrupts
 *
 * \remark IRQ nesting is managed
 */
void BoardEnableIrq( void );

/*!
 * \brief Initializes the target board peripherals.
 */
void BoardInitMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Measure the Battery voltage
 *
 * \retval value  battery voltage in volts
 */
uint32_t BoardGetBatteryVoltage( void );
uint16_t BoardBatteryMeasureVolage( void );
uint32_t CustomizeMeasureVolage( uint32_t channel );


/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level [  0: USB,
 *                                 1: Min level,
 *                                 x: level
 *                               254: fully charged,
 *                               255: Error]
 */
uint8_t BoardGetBatteryLevel( void );

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

/*!
 * \brief Get the board power source
 *
 * \retval value  power source [0: USB_POWER, 1: BATTERY_POWER]
 */
uint8_t GetBoardPowerSource( void );

#endif // __BOARD_H__
