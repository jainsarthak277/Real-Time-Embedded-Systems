/*Author: Sarthak Jain      Dated: 3/21/2019                            */
/*This code is written for the course of Real time embedded systems,    */
/*under Prof. Tim Sherr.                                                */
/*Reference is given to freertos_demo.c - Simple FreeRTOS example. by   */
/*by Texas Instruments, the FreeRTOS.org website for manual reference,  */
/*and RT_Simple_Thread code provided by Prof. Sam Siewert for Fibonacci */
/*series calculation                                                    */

/*The code creates 2 task which wait on a semaphore that is released    */
/*by the other task. Each task computes a Fibonacci series for 10/40 ms */
/*respectively, and then releases the semaphore for the other task.     */


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "driverlib/timer.h"
#include "utils/uartstdio.h"
#include "led_task.h"
#include "priorities.h"
#include "switch_task.h"
#include "FreeRTOS.h"
#include "FreeRTOSconfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "time.h"

#define configLOGGING_INCLUDE_TIME_AND_TASK_NAME    (1)

#define TOTAL_TIME          (30)        //in seconds
#define FREQUENCY           (30)        //in Hertz, eg. 30 Hz = 33.33 ms
#define FREQ_1              (10)
#define FREQ_2              (30)
#define FREQ_3              (60)
#define FREQ_4              (30)
#define FREQ_5              (60)
#define FREQ_6              (30)
#define FREQ_7              (300)

#define MICROSEC_PER_SEC    (1000000)
#define PRIO_MAX            (10)
#define FALSE               (0)
#define TRUE                (1)

int abortTest=FALSE;
int abortS1=FALSE, abortS2=FALSE, abortS3=FALSE, abortS4=FALSE, abortS5=FALSE, abortS6=FALSE, abortS7=FALSE;
xSemaphoreHandle g_pUARTSemaphore, semaphore_1, semaphore_2, semaphore_3, semaphore_4, semaphore_5, semaphore_6, semaphore_7;
volatile TickType_t time_tickers, sec_tickers, msec_tickers;

volatile unsigned long long SeqCnt=0;
int rc, delay_cnt=0;
uint32_t g_ui32Flags;
double residual = 0.0;

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif

void
vApplicationStackOverflowHook(xTaskHandle *pxTask, char *pcTaskName)
{
    while(1)
    {
    }
}

void
ConfigureUART(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    UARTStdioConfig(0, 115200, 16000000);
}


void
Timer0IntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_IntMasterDisable();

    ++SeqCnt;
    time_tickers = xTaskGetTickCount();
    sec_tickers = time_tickers/1000;
    msec_tickers = time_tickers%1000;
//    xSemaphoreTakeFromISR(g_pUARTSemaphore, pdFALSE);
//    UARTprintf("\n<TIME: %d >   [TIMER 0] Sequencer cycle %d\n", time_tickers, SeqCnt);
//    xSemaphoreGiveFromISR(g_pUARTSemaphore, pdFALSE);

        // Release each service at a sub-rate of the generic sequencer rate

        // Servcie_1 = RT_MAX-1 @ 30 Hz
    if((SeqCnt % FREQ_1) == 0)   xSemaphoreGive(semaphore_1);

        // Service_2 = RT_MAX-2 @ 10 Hz
    if((SeqCnt % FREQ_2) == 0)   xSemaphoreGive(semaphore_2);

        // Service_3 = RT_MAX-3 @ 5 Hz
    if((SeqCnt % FREQ_3) == 0)   xSemaphoreGive(semaphore_3);

        // Service_4 = RT_MAX-2 @ 10 Hz
    if((SeqCnt % FREQ_4) == 0)   xSemaphoreGive(semaphore_4);

        // Service_5 = RT_MAX-3 @ 5 Hz
    if((SeqCnt % FREQ_5) == 0)   xSemaphoreGive(semaphore_5);

        // Service_6 = RT_MAX-2 @ 10 Hz
    if((SeqCnt % FREQ_6) == 0)   xSemaphoreGive(semaphore_6);

        // Service_7 = RT_MIN   1 Hz
    if((SeqCnt % FREQ_7) == 0)   xSemaphoreGive(semaphore_7);

    if(SeqCnt > (TOTAL_TIME*30*(FREQUENCY/30)))
    {
        xSemaphoreGive(semaphore_1);
        xSemaphoreGive(semaphore_2);
        xSemaphoreGive(semaphore_3);
        xSemaphoreGive(semaphore_4);
        xSemaphoreGive(semaphore_5);
        xSemaphoreGive(semaphore_6);
        xSemaphoreGive(semaphore_7);
        abortS1=TRUE;
        abortS2=TRUE;
        abortS3=TRUE;
        abortS4=TRUE;
        abortS5=TRUE;
        abortS6=TRUE;
        abortS7=TRUE;
        ROM_TimerDisable(TIMER0_BASE, TIMER_A);
        ROM_TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
        UARTprintf("\nTEST COMPLETE!!!\n");
        UARTprintf("\n\n");
    }
    ROM_IntMasterEnable();
}

static void
Process_Task1(void *pvParameters)
{
    volatile TickType_t tickers = xTaskGetTickCount();
    TickType_t start, stop, diff_ticks = 0;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[1]", tickers/1000, tickers%1000);
    xSemaphoreGive(g_pUARTSemaphore);
    unsigned long S1Cnt=0;
    while(!abortS1)
    {
        if(xSemaphoreTake(semaphore_1, portMAX_DELAY) != pdTRUE)
        {
            UARTprintf("\nERROR: Unable to obtain semaphore 1.\n");
        }
        start = xTaskGetTickCount();
        tickers = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[1] release", tickers/1000, tickers%1000);
        xSemaphoreGive(g_pUARTSemaphore);
        S1Cnt++;
        stop = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\nExecution time [1]: %d msecs\n", stop - start);
        xSemaphoreGive(g_pUARTSemaphore);
        if((stop - start) > diff_ticks)     diff_ticks = stop - start;
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n");
    UARTprintf("\nTask [1] ran %d ", S1Cnt);
    UARTprintf("times with WCET = %d", diff_ticks);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

static void
Process_Task2(void *pvParameters)
{
    volatile TickType_t tickers = xTaskGetTickCount();
    TickType_t start, stop, diff_ticks = 0;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
   UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[2]", tickers/1000, tickers%1000);
    xSemaphoreGive(g_pUARTSemaphore);
    unsigned long S2Cnt=0;
    while(!abortS2)
    {
        if(xSemaphoreTake(semaphore_2, portMAX_DELAY) != pdTRUE)
        {
            UARTprintf("\nERROR: Unable to obtain semaphore 2.\n");
        }
        start = xTaskGetTickCount();
        tickers = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[2] release", tickers/1000, tickers%1000);
        xSemaphoreGive(g_pUARTSemaphore);
        S2Cnt++;
        stop = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\nExecution time [2]: %d msecs\n", stop - start);
        xSemaphoreGive(g_pUARTSemaphore);
        if((stop - start) > diff_ticks)     diff_ticks = stop - start;
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n");
    UARTprintf("\nTask [2] ran %d ", S2Cnt);
    UARTprintf("times with WCET = %d", diff_ticks);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

static void
Process_Task3(void *pvParameters)
{
    volatile TickType_t tickers = xTaskGetTickCount();
    TickType_t start, stop, diff_ticks = 0;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[3]", tickers/1000, tickers%1000);
    xSemaphoreGive(g_pUARTSemaphore);
    unsigned long S3Cnt=0;
    while(!abortS3)
    {
        if(xSemaphoreTake(semaphore_3, portMAX_DELAY) != pdTRUE)
        {
            UARTprintf("\nERROR: Unable to obtain semaphore 3.\n");
        }
        start = xTaskGetTickCount();
        tickers = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[3] release", tickers/1000, tickers%1000);
        xSemaphoreGive(g_pUARTSemaphore);
        S3Cnt++;
        stop = xTaskGetTickCount();
       xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
       UARTprintf("\nExecution time [3]: %d msecs\n", stop - start);
       xSemaphoreGive(g_pUARTSemaphore);
       if((stop - start) > diff_ticks)     diff_ticks = stop - start;
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n");
    UARTprintf("\nTask [3] ran %d ", S3Cnt);
    UARTprintf("times with WCET = %d", diff_ticks);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

static void
Process_Task4(void *pvParameters)
{
    volatile TickType_t tickers = xTaskGetTickCount();
    TickType_t start, stop, diff_ticks = 0;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[4]", tickers/1000, tickers%1000);
    xSemaphoreGive(g_pUARTSemaphore);
    unsigned long S4Cnt=0;
    while(!abortS4)
    {
        if(xSemaphoreTake(semaphore_4, portMAX_DELAY) != pdTRUE)
        {
            UARTprintf("\nERROR: Unable to obtain semaphore 4.\n");
        }
        start = xTaskGetTickCount();
        tickers = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[4] release", tickers/1000, tickers%1000);
        xSemaphoreGive(g_pUARTSemaphore);
        S4Cnt++;
        stop = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\nExecution time [4]: %d msecs\n", stop - start);
        xSemaphoreGive(g_pUARTSemaphore);
        if((stop - start) > diff_ticks)     diff_ticks = stop - start;
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n");
    UARTprintf("\nTask [4] ran %d ", S4Cnt);
    UARTprintf("times with WCET = %d", diff_ticks);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

static void
Process_Task5(void *pvParameters)
{
    volatile TickType_t tickers = xTaskGetTickCount();
    TickType_t start, stop, diff_ticks = 0;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[5]", tickers/1000, tickers%1000);
    xSemaphoreGive(g_pUARTSemaphore);
    unsigned long S5Cnt=0;
    while(!abortS5)
    {
        if(xSemaphoreTake(semaphore_5, portMAX_DELAY) != pdTRUE)
        {
            UARTprintf("\nERROR: Unable to obtain semaphore 5.\n");
        }
        start = xTaskGetTickCount();
        tickers = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[5] release", tickers/1000, tickers%1000);
        xSemaphoreGive(g_pUARTSemaphore);
        S5Cnt++;
        stop = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\nExecution time [5]: %d msecs\n", stop - start);
        xSemaphoreGive(g_pUARTSemaphore);
        if((stop - start) > diff_ticks)     diff_ticks = stop - start;
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n");
    UARTprintf("\nTask [5] ran %d ", S5Cnt);
    UARTprintf("times with WCET = %d", diff_ticks);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

static void
Process_Task6(void *pvParameters)
{
    volatile TickType_t tickers = xTaskGetTickCount();
    TickType_t start, stop, diff_ticks = 0;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[6]", tickers/1000, tickers%1000);
    xSemaphoreGive(g_pUARTSemaphore);
    unsigned long S6Cnt=0;
    while(!abortS6)
    {
        if(xSemaphoreTake(semaphore_6, portMAX_DELAY) != pdTRUE)
        {
            UARTprintf("\nERROR: Unable to obtain semaphore 6.\n");
        }
        start = xTaskGetTickCount();
        tickers = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[6] release", tickers/1000, tickers%1000);
        xSemaphoreGive(g_pUARTSemaphore);
        S6Cnt++;
        stop = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\nExecution time [6]: %d msecs\n", stop - start);
        xSemaphoreGive(g_pUARTSemaphore);
        if((stop - start) > diff_ticks)     diff_ticks = stop - start;
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n");
    UARTprintf("\nTask [6] ran %d ", S6Cnt);
    UARTprintf("times with WCET = %d", diff_ticks);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}

static void
Process_Task7(void *pvParameters)
{
    volatile TickType_t tickers = xTaskGetTickCount();
    TickType_t start, stop, diff_ticks = 0;
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[7]", tickers/1000, tickers%1000);
    xSemaphoreGive(g_pUARTSemaphore);
    unsigned long S7Cnt=0;
    while(!abortS7)
    {
        if(xSemaphoreTake(semaphore_7, portMAX_DELAY) != pdTRUE)
        {
            UARTprintf("\nERROR: Unable to obtain semaphore 7.\n");
        }
        start = xTaskGetTickCount();
        tickers = xTaskGetTickCount();
        xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\n<TIME: %d seconds; %d milliseconds>   Thread[7] release", tickers/1000, tickers%1000);
        xSemaphoreGive(g_pUARTSemaphore);
        S7Cnt++;
        stop = xTaskGetTickCount();
       xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
        UARTprintf("\nExecution time [7]: %d msecs\n", stop - start);
        xSemaphoreGive(g_pUARTSemaphore);
        if((stop - start) > diff_ticks)     diff_ticks = stop - start;
    }
    xSemaphoreTake(g_pUARTSemaphore, portMAX_DELAY);
    UARTprintf("\n");
    UARTprintf("\nTask [7] ran %d ", S7Cnt);
    UARTprintf("times with WCET = %d", diff_ticks);
    xSemaphoreGive(g_pUARTSemaphore);
    vTaskDelete(NULL);
}


int
main(void)
{
    ROM_FPULazyStackingEnable();
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                     SYSCTL_OSC_MAIN);

    g_pUARTSemaphore = xSemaphoreCreateMutex();
    ConfigureUART();

    UARTprintf("\n\nWelcome to the EK-TM4C123GXL FreeRTOS Exercise displaying sequencer demo!\n");

    semaphore_1 = xSemaphoreCreateBinary();
    semaphore_2 = xSemaphoreCreateBinary();
    semaphore_3 = xSemaphoreCreateBinary();
    semaphore_4 = xSemaphoreCreateBinary();
    semaphore_5 = xSemaphoreCreateBinary();
    semaphore_6 = xSemaphoreCreateBinary();
    semaphore_7 = xSemaphoreCreateBinary();

    if(xTaskCreate(Process_Task1, (const portCHAR *)"PROCESS 1", 128, NULL,
                   PRIO_MAX - 1, NULL) != pdTRUE)
    {
        UARTprintf("\nERROR: Processing task 1 not created.\n");
    }

    if(xTaskCreate(Process_Task2, (const portCHAR *)"PROCESS 1", 128, NULL,
                   PRIO_MAX - 2, NULL) != pdTRUE)
    {
        UARTprintf("\nERROR: Processing task 2 not created.\n");
    }

    if(xTaskCreate(Process_Task3, (const portCHAR *)"PROCESS 1", 128, NULL,
                   PRIO_MAX - 3, NULL) != pdTRUE)
    {
        UARTprintf("\nERROR: Processing task 3 not created.\n");
    }

    if(xTaskCreate(Process_Task4, (const portCHAR *)"PROCESS 1", 128, NULL,
                   PRIO_MAX - 4, NULL) != pdTRUE)
    {
        UARTprintf("\nERROR: Processing task 4 not created.\n");
    }

    if(xTaskCreate(Process_Task5, (const portCHAR *)"PROCESS 1", 128, NULL,
                   PRIO_MAX - 5, NULL) != pdTRUE)
    {
        UARTprintf("\nERROR: Processing task 5 not created.\n");
    }

    if(xTaskCreate(Process_Task6, (const portCHAR *)"PROCESS 1", 128, NULL,
                   PRIO_MAX - 6, NULL) != pdTRUE)
    {
        UARTprintf("\nERROR: Processing task 6 not created.\n");
    }

    if(xTaskCreate(Process_Task7, (const portCHAR *)"PROCESS 2", 128, NULL,
                   PRIO_MAX - 7, NULL) != pdTRUE)
    {
        UARTprintf("\nERROR: Processing task 7 not created.\n");
    }

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    ROM_IntMasterEnable();

    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, 50000*(1000/FREQUENCY));//((float)ROM_SysCtlClockGet())/*/(MICROSEC_PER_SEC*33)*/);
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);

    vTaskStartScheduler();

    while(1)
    {
    }
}
