/*
 * logger_task.c
 *
 *  Created on: 08-Apr-2019
 *      Author: hardi
 */

#include "include/logger_task.h"
#include "utils/uartstdio.h"

extern QueueHandle_t  g_pLEDTaskQueue;
extern QueueHandle_t  g_pTempTaskQueue;

extern xSemaphoreHandle g_pUARTSemaphore;

static void TaskFunction(void *pvParameters)
{

    led_message_t led_message;
    temp_message_t temp_message;
    int temp_int = 0, temp_dec = 0;

    while (1)
    {

       // UARTprintf("Logger Task\n");

        if (xQueueReceive(g_pLEDTaskQueue, &led_message,
                portMAX_DELAY) == pdPASS)
        {
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("Name : %s Received Timer Tick is : %d and LED Toggle Count is : %d\n", led_message.name, led_message.time_stamp, led_message.toggle_count);
            xSemaphoreGive(g_pUARTSemaphore);
        }

        if (xQueueReceive(g_pTempTaskQueue, &temp_message,
                0) == pdPASS)
        {

            temp_int = temp_message.temperature;
            temp_dec = (temp_message.temperature * 1000 - temp_message.temperature);

            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
            UARTprintf("\n\nReceived Timer Tick is : %d and Temperature Value is : %d.%d\n\n", temp_message.time_stamp, temp_int, temp_dec);
            xSemaphoreGive(g_pUARTSemaphore);
        }
    }
}

uint32_t TaskLoggerInit()
{

    if (xTaskCreate(TaskFunction, (const portCHAR *) "TASK_LOGGER",
            TASKSTACKSIZE, NULL,
            tskIDLE_PRIORITY + PRIORITY_LOGGER_TASK, NULL) != pdTRUE)
    {
        UARTprintf("ERROR: creating Logger task\n");
        return (1);
    }

    return (0);
}
