/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * distance.h
 * This file includes headers required for distance.c
 */

#ifndef INCLUDE_DISTANCE_H_
#define INCLUDE_DISTANCE_H_

//*************************** Standard Libraries ********************************

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

//************************* Hardware Libraries ***********************************

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_ints.h"
#include "inc/hw_timer.h"

//*************************** Driver Libraries ***********************************

#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

//********************************************************************************

#define TIMER_CLOCK_FREQUENCY                   (20000000)          // Timer Clock Frequency
#define TICKS_PER_USEC                          (1000000)           // Timer Ticks per microsecond

typedef struct distSensor{

    uint8_t trigGpioPin;
    uint8_t echoGpioPin;
    uint32_t trigGpioPortBase;
    uint32_t echoGpioPortBase;
    uint32_t trigGpioPortPeriph;
    uint32_t echoGpioPortPeriph;
    uint32_t echoIntType;
    uint32_t timerPortPeriph;
    uint32_t timerBase;
    uint32_t timerSubType;
    uint32_t timerConfig;
    uint32_t trigDelay;

}distSensor_t;

extern const distSensor_t HCSR04;
extern volatile uint16_t echo_flag;
extern volatile uint32_t echo_duration;
extern volatile uint32_t echo_1;
extern volatile uint32_t echo_2;

/*
 * @brief: Triggers the ultrasound sensor
 */

void distSensor_triggerPulse(const distSensor_t HC_SR04);

/*
 * @brief: Interrupt Handler for echo input pulse
 */
void distSensor_echoIntHandler(void);

/*
 * @brief: Configures gpio and timer for ultrasounf sensor
 */
void distSensor_Config(const distSensor_t HC_SR04);

/*
 * @brief: Calculates distance by duration of the echo pulse
 */
uint32_t distSensor_getDistance(const distSensor_t HC_SR04);

/*******************************************************
 *
 * uint32_t distance_cm;

    distSensor_Config(HCSR04);
    distance_cm = distSensor_getDistance(HCSR04);

    UARTprintf("\nDistance : %d cm", distance_cm);
 *
 ******************************************************/

#endif /* INCLUDE_DISTANCE_H_ */
