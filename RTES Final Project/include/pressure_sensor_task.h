/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * pressure_sensor_task.h
 * This file includes headers required for pressure_sensor_task.c
 */

#ifndef INC_TEMPERATURE_TASK_H_
#define INC_TEMPERATURE_TASK_H_

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"
#include "utils/uartstdio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include <stdlib.h>

extern xSemaphoreHandle g_pUARTSemaphore;

#define TASKSTACKSIZE                    (128)
#define PRIORITY_PRESSURE_TASK            (4)

#define MOVE_FORWARD_BIT            (0b00000001)
#define MOVE_BACKWARD_BIT           (0b00000010)
#define MOVE_RIGHT_BIT              (0b00000100)
#define MOVE_LEFT_BIT               (0b00001000)
#define BREAK_BIT                   (0b00010000)
#define STOP_BIT                    (0b00100000)

extern QueueHandle_t motor_q;
extern TaskHandle_t g_MotorTaskHandle;
extern xSemaphoreHandle g_pPressureTaskSemaphore;
extern uint8_t command;

uint32_t tmp[2];

void ConfigureADC();
uint32_t PressureSensorTaskInit(void);
void PressureFunction(void *pvParameters);


#endif /* INC_TEMPERATURE_TASK_H_ */
