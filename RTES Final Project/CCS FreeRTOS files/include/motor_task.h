/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * motor_task.h
 * This file includes headers required for motor_task.c
 */

#ifndef INCLUDE_MOTOR_TASK_H_
#define INCLUDE_MOTOR_TASK_H_

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "timer.h"
#include "FreeRTOSConfig.h"
#include "utils/uartstdio.h"
#include "motor_task.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"


#define TASKSTACKSIZE               (128)
#define PRIORITY_MOTOR_TASK         (4)
#define QUEUE_SIZE                  (10)

#define MOVE_FORWARD_BIT            (0b00000001)
#define MOVE_BACKWARD_BIT           (0b00000010)
#define MOVE_RIGHT_BIT              (0b00000100)
#define MOVE_LEFT_BIT               (0b00001000)
#define BREAK_BIT                   (0b00010000)
#define STOP_BIT                    (0b00100000)


#define PWM_PERIOD_90               (57600)
#define PWM_PERIOD_50               (32000)

uint32_t TaskMotorInit(void);
void setup_motors(void);
void setup_gpio(void);
void move_forward(void);
void move_backward(void);
void turn_left(void);
void turn_right(void);
void slow_motor(void);
void stop_motor(void);

QueueHandle_t motor_q;
extern xSemaphoreHandle g_pMotorTaskSemaphore;
extern xSemaphoreHandle g_pUARTSemaphore;
extern uint8_t command;

#endif /* INCLUDE_MOTOR_TASK_H_ */
