/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * pressure_sensor_task.c
 * This file has the pressure sensor task, which configures the ADC, gets
 * data from it, processes and sends a message to the motor task via a message queue
 * to either start the motors, stop them, or run one of the two motors.
 */

#include "FreeRTOSConfig.h"
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "math.h"
#include "pressure_sensor_task.h"

void ConfigureADC()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);    //Ch 0 LEFT
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);    //Ch 1 RIGHT
    ROM_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH0 | ADC_CTL_IE);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH1 | ADC_CTL_IE | ADC_CTL_END);

    ROM_ADCIntClear(ADC0_BASE, 1);
    //ROM_IntEnable(INT_ADC0SS1);
    ROM_ADCIntEnable(ADC0_BASE, 1);
    ROM_ADCSequenceEnable(ADC0_BASE, 1);
}

void PressureFunction(void *pvParameters)
{
    TickType_t start;
    while (1)
    {
        if ( xSemaphoreTake(g_pPressureTaskSemaphore, portMAX_DELAY) == pdTRUE)
        {
            start = xTaskGetTickCount();
            xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
//            UARTprintf("\nPressure Task at %d msecs\n", start);
            xSemaphoreGive(g_pUARTSemaphore);
            //TODO task notification
            ROM_ADCProcessorTrigger(ADC0_BASE, 1);
            while (!ADCIntStatus(ADC0_BASE, 1, false));

            ROM_ADCIntClear(ADC0_BASE, 1);
            ROM_ADCSequenceDataGet(ADC0_BASE, 1, tmp);
//            UARTprintf("Value: %d %d\n",tmp[0], tmp[1]);
            {
                if ((tmp[0] > 1500) && (tmp[1] > 1500))
                {
                    command = MOVE_FORWARD_BIT;
                }
                else if (tmp[0] > 1500)
                {
                    command = MOVE_LEFT_BIT;
                }
                else if (tmp[1] > 1500)
                {
                    command = MOVE_RIGHT_BIT;
                }
                if (xQueueSend(motor_q, (char* )&command, portMAX_DELAY) != pdTRUE) //message queue to motors to slow down
                {
                    UARTprintf("\nError sending to queue from Ultrasonic Func.\n");
                }
            }
        }
    }
//    vTaskDelete(NULL);
}

uint32_t PressureSensorTaskInit()
{

    if (xTaskCreate(PressureFunction, (const portCHAR *) "TASK_PRESSURE", TASKSTACKSIZE,
                    NULL, tskIDLE_PRIORITY + PRIORITY_PRESSURE_TASK, NULL) != pdTRUE)
    {
        UARTprintf("ERROR: creating Pressure task\n");
        return (1);
    }

    return (0);
}

