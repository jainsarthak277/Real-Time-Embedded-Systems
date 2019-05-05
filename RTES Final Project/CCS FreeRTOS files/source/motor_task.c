/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * motor_task.c
 * This file has APIs for setting up the GPIO pins and motors. It also
 * has APIs for motor controls, like moving in all directions.
 * It has the control logic for the motor task, which waits first on a
 * semaphore, and then a message queue for some other task to signal it to
 * run the motors. Logic is computed, and then it motor control is decided.
 */

#include "motor_task.h"

TaskHandle_t g_MotorTaskHandle;

void move_forward()
{
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWM_PERIOD_90);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, PWM_PERIOD_90);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_PIN_5);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0);
}

void move_backward()
{
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWM_PERIOD_90);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, PWM_PERIOD_90);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_6);

}

//TODO add timer for turning
void turn_left()
{
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWM_PERIOD_90);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, PWM_PERIOD_90);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0);
}

//TODO add timer for turning
void turn_right()
{
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWM_PERIOD_90);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, PWM_PERIOD_90);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_PIN_5);
//    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_6);
}

void slow_motor()
{

    /*Set duty cycle to be 50%*/
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWM_PERIOD_50);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, PWM_PERIOD_50);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_PIN_2);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_PIN_5);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_6);

}

void stop_motor()
{

    /*Set duty cycle to be 90%*/
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, PWM_PERIOD_90);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, PWM_PERIOD_90);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0);

    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0);
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0);

}

void setup_gpio()
{

    /*Set up GPIO pins for direction of motors
     * PA2 - Motor1 direction input AIN1
     * PA3 - Motor1 direction input AIN2
     * PA5 - Motor2 direction input BIN1
     * PA6 - Motor2 direction input BIN2
     * PA4 - Standby input
     * */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeGPIOOutput(
            GPIO_PORTA_BASE,
            GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6);

}

void setup_motors()
{
    /*Set the PWM clock to be system clock*/
    SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

    /*For this example PWM0 is used with PortB Pin6.  The actual port and pins
     used may be different on your part, consult the data sheet for more
     information.  GPIO port B needs to be enabled so these pins can be used.*/

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    /*Configure the GPIO pin muxing to select PWM00 functions for these pins.
     This step selects which alternate function is available for these pins.
     This is necessary if your part supports GPIO pin function muxing.
     Consult the data sheet to see which functions are allocated per pin.*/

    GPIOPinConfigure(GPIO_PB6_M0PWM0);
    GPIOPinConfigure(GPIO_PB7_M0PWM1);

    /*Configure the PWM function for this pin.
     Consult the data sheet to see which functions are allocated per pin.*/

    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6 | GPIO_PIN_7);

    /*Configure the PWM0 to count down without synchronization.*/
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0,
    PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

    /*Set PWM period*/
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 64000);

    /*
     Enable processor interrupts.
     IntMasterEnable();
     */

    /*    Allow PWM0 generated interrupts.  This configuration is done to
     differentiate fault interrupts from other PWM0 related interrupts.
     PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);*/

    /*Enable the PWM0 LOAD interrupt on PWM0.*/
    PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_LOAD);

    /*
     Enable the PWM0 interrupts on the processor (NVIC).
     IntEnable (INT_PWM0_0);

     */
    /*Enable the PWM0 output signal.*/
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);

    /*Enables the PWM generator block.*/
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);

    setup_gpio();
}

static void MotorFunction(void *pvParameters)
{
//    UARTprintf("Motor TASK\n");
    TickType_t start;
    uint32_t ulNotifiedValue = 0;

    while (1)
    {
        if ( xSemaphoreTake(g_pMotorTaskSemaphore, portMAX_DELAY) == pdTRUE)
        {

            if(xQueueReceive(motor_q, (char*)&command, portMAX_DELAY) != pdTRUE)
            {
                UARTprintf("\nError receiving in queue\n");
            }
//            UARTprintf("\nMotor task released at %d msecs\n", start);
            if (command & STOP_BIT)
            {
                stop_motor();
                xQueueReset(motor_q);
            }
            else
            {
                /*TODO switch case*/
                if (command & MOVE_FORWARD_BIT)
                {
                    move_forward();
                }

                if (command & MOVE_BACKWARD_BIT)
                {
                    move_backward();
                }

                if (command & MOVE_RIGHT_BIT)
                {
                    turn_right();
                }

                if (command & MOVE_LEFT_BIT)
                {
                    turn_left();
                }

                if (command & BREAK_BIT)
                {
                    slow_motor();
                }
            }
        }

    }
//    vTaskDelete(NULL);
}

uint32_t TaskMotorInit()
{

    if (xTaskCreate(MotorFunction, (const portCHAR *) "TASK_MOTOR",
            TASKSTACKSIZE, NULL, tskIDLE_PRIORITY + PRIORITY_MOTOR_TASK,
            &g_MotorTaskHandle) != pdTRUE)
    {
        UARTprintf("ERROR: creating Motor task\n");
        return (1);
    }

    motor_q = xQueueCreate(QUEUE_SIZE, sizeof(uint8_t));
    if(motor_q == NULL)
    {
        UARTprintf("\nError in creating message queue\n");
    }
    setup_gpio();
    setup_motors();

    return (0);
}

