/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * ultrasonic_task.c
 * This file initializes the ultrasonic task, and waits on a semaphore
 * to be released by the sequencer. Once semaphore is released, the task
 * calls an API to get distance through the ultrasonic sensor,
 * and based on the distance taks a decision to either do nothing, or stop
 * the motors or slow them down.
 */

#include "ultrasonic_task.h"

void UltrasonicFunction(void *pvParameters)
{
    uint32_t distance_cm;
    TickType_t start;
    while (1)
    {
        if ( xSemaphoreTake(g_pUltrasonicTaskSemaphore, portMAX_DELAY) == pdTRUE)
        {
            start = xTaskGetTickCount();
            UARTprintf("\nUltrasonic Task released at %d msecs", distance_cm, start);

            distance_cm = distSensor_getDistance(HCSR04);
            if(distance_cm < 25)
            {
                obstacle = 1;
                command = STOP_BIT;
                if(xQueueSendToFront(motor_q, (char*)&command, portMAX_DELAY) != pdTRUE)//message queue to motors to slow down
                {
                    UARTprintf("\nError sending to queue from Ultrasonic Func.\n");
                }
            }
            else if(distance_cm < 50)
            {
                command = BREAK_BIT;
                //message queue to motors to stop
                if(xQueueSend(motor_q, (char*)&command, portMAX_DELAY) != pdTRUE)//message queue to motors to slow down
                {
                    UARTprintf("\nError sending to queue from Ultrasonic Func.\n");
                }
            }
            else
            {
                command = MOVE_FORWARD_BIT;
                //message queue to motors to move forward
                if(xQueueSend(motor_q, (char*)&command, portMAX_DELAY) != pdTRUE)//message queue to motors to slow down
                {
                    UARTprintf("\nError sending to queue from Ultrasonic Func.\n");
                }
            }
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\nDistance : %d cm at %d msecs", distance_cm, start);
            xSemaphoreGive(g_pUARTSemaphore);
        }
    }
    vTaskDelete(NULL);
}

uint32_t UltrasonicTaskInit()
{

    if (xTaskCreate(UltrasonicFunction, (const portCHAR *) "TASK_ULTRASONIC", TASKSTACKSIZE,
            NULL, tskIDLE_PRIORITY + PRIORITY_ULTRASONIC_TASK, NULL) != pdTRUE)
    {
        UARTprintf("ERROR: creating Ultrasonic task\n");
        return (1);
    }
    distSensor_Config(HCSR04);

    return (0);
}
