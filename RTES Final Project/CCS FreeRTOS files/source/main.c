//*****************************************************************************
//
// freertos_demo.c - Simple FreeRTOS example.
//
// Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * main.c
 * This file initializes semaphores, a timer for sequencing tasks, and creates
 * tasks. It also has a timer handler, which posts the semaphores required by
 * various tasks to run.
 */

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "portmacro.h"
#include "task.h"
#include "include/main.h"
//*****************************************************************************


//TODO set period for tasks
#define PRESSURE_TASK_PERIOD                (60)
#define MOTOR_TASK_PERIOD                   (60)
#define ULTRASONIC_TASK_PERIOD              (100)


/*Binary semaphores for scheduling tasks from the timer handler*/

xSemaphoreHandle g_pUltrasonicTaskSemaphore;
xSemaphoreHandle g_pPressureTaskSemaphore;
xSemaphoreHandle g_pMotorTaskSemaphore;

/*Mutex for sharing UART*/
xSemaphoreHandle g_pUARTSemaphore;

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    while (1)
    {
    }
}

/*TImer0 Interrupt Handler*/
void Timer0AIntHandler(void)
{

    if(button_flag & 0x01)
    {
        ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
        ROM_IntMasterDisable();

        ++SeqCnt;
        time_tickers = xTaskGetTickCount();

            // Servcie_1 = RT_MAX-1 @ 10 Hz
        if((SeqCnt % ULTRASONIC_TASK_PERIOD) == 0)   xSemaphoreGive(g_pUltrasonicTaskSemaphore);

            // Service_2 = RT_MAX-2 @ 10 Hz
        if((SeqCnt % PRESSURE_TASK_PERIOD) == 0)   xSemaphoreGive(g_pPressureTaskSemaphore);

        // Service_2 = RT_MAX-2 @ 10 Hz
        if((SeqCnt % MOTOR_TASK_PERIOD) == 0)   xSemaphoreGive(g_pMotorTaskSemaphore);

        ROM_IntMasterEnable();
    }
}

void diable_hardware_timer0()
{
    /*Diable TIMER0*/
    IntDisable(INT_TIMER0B);

    /*Disable TIMER0 Interrupt*/
    TimerIntDisable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);

    /*Clear pending interrupts if any*/
    TimerIntClear(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
}

//*****************************************************************************
//
// Configure Hardware Timer0 to fire every 1ms.
// Reference - Tivaware Examples for Timer peripheral
//*****************************************************************************

void initialize_hardware_timer0()
{

    /*Enable TIMER0 peripheral*/
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    /*Enable Processor Interrupts*/
    ROM_IntMasterEnable();

    /*Configure TIMER0*/
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, 50000);
    ROM_IntEnable(INT_TIMER0A);
    /*Enable Interrupt for TIMER0 when timer time out and enable
     * the interrupt for timer0 in processor*/
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    /*Enable the timer*/
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);

}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
// @source credit - TI Examples
//*****************************************************************************
void ConfigureUART(void)
{

    /*Enable GPIO for UART module*/
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    /*Enable UART module*/
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /*COnfigure GPIO alternate fucntions for UART0*/
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /*Set UART clock*/
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    /*Config UART for STDOUT*/
    UARTStdioConfig(0, 115200, 16000000);
}

//*****************************************************************************
//
// Initialize FreeRTOS and start the initial set of tasks.
//
//*****************************************************************************
int main(void)
{
    ROM_FPULazyStackingEnable();
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                     SYSCTL_OSC_MAIN);

    g_pUARTSemaphore = xSemaphoreCreateMutex();
    ConfigureUART();

    UARTprintf("\n\nStarted Executing Main thread !\n");
    UARTprintf("System Clock Value %d\n", SysCtlClockGet());

    /*Create a binary semaphore to signal the task from the timer handler*/
    g_pUARTSemaphore = xSemaphoreCreateMutex();
    g_pUltrasonicTaskSemaphore = xSemaphoreCreateBinary();
    g_pPressureTaskSemaphore = xSemaphoreCreateBinary();
    g_pMotorTaskSemaphore = xSemaphoreCreateBinary();

    /*Create a Timer*/
    initialize_hardware_timer0();
    ConfigureButton();
    setup_motors();
    ConfigureADC();
    command = 0;
    while(startup!=0);

    /*Create task for Pressure Sensor*/
    if (PressureSensorTaskInit() != 0)
    {

        while (1)
        {
        }
    }

    /*Create task for Ultrasonic Sensor*/
    if (UltrasonicTaskInit() != 0)
    {

        while (1)
        {
        }
    }

    /*Create task for Motor*/
    if (TaskMotorInit() != 0)
    {

        while (1)
        {
        }
    }

    /*Start Scheduler*/
    vTaskStartScheduler();

//    while (1)
    {
    }
}
