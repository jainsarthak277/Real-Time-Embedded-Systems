/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * ultrasonic_task.h
 * This file includes headers required for ultrasonic_task.c
 */

#include <stdlib.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"
#include "utils/uartstdio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "distance.h"

#define TASKSTACKSIZE                       (128)
#define PRIORITY_ULTRASONIC_TASK           (5)

#define MOVE_FORWARD_BIT            (0b00000001)
#define MOVE_BACKWARD_BIT           (0b00000010)
#define MOVE_RIGHT_BIT              (0b00000100)
#define MOVE_LEFT_BIT               (0b00001000)
#define BREAK_BIT                   (0b00010000)
#define STOP_BIT                    (0b00100000)

extern xSemaphoreHandle g_pUARTSemaphore;
extern xSemaphoreHandle g_pUltrasonicTaskSemaphore;
extern QueueHandle_t motor_q;
extern uint8_t obstacle;
extern uint8_t command;

uint32_t UltrasonicTaskInit(void);
void UltrasonicFunction(void *pvParameters);
