/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * main.h
 * This file includes headers required for main.c
 */

#ifndef INC_MAIN_H_
#define INC_MAIN_H_

#include "motor_task.h"
#include "pressure_sensor_task.h"
#include "ultrasonic_task.h"
#include "button.h"


volatile unsigned long long SeqCnt=0;
volatile TickType_t time_tickers;
uint8_t command;

/*Function Prototypes*/
void configure_uart(void);
void initialize_hardware_timer0(void);
void disable_hardware_timer0(void);


#endif /* INC_MAIN_H_ */
