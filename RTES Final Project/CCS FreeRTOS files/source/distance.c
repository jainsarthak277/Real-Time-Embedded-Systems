/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * distance.c
 * This file includes APIs for initializing the distance sensor,
 * sending the trigger pulse, and calculating the distance from object
 * based on time taken for pulse to return to echo pin.
 */


#include "include/distance.h"

#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#ifdef PART_TM4C123GH6PM

const distSensor_t HCSR04 = {
                                     .trigGpioPin = GPIO_PIN_1,
                                     .echoGpioPin = GPIO_PIN_0,
                                     .trigGpioPortBase = GPIO_PORTB_BASE,
                                     .echoGpioPortBase = GPIO_PORTB_BASE,
                                     .echoGpioPortPeriph = SYSCTL_PERIPH_GPIOB,
                                     .trigGpioPortPeriph = SYSCTL_PERIPH_GPIOB,
                                     .echoIntType = GPIO_BOTH_EDGES,
                                     .timerPortPeriph = SYSCTL_PERIPH_TIMER1,
                                     .timerBase = TIMER1_BASE,
                                     .timerSubType = TIMER_A,
                                     .timerConfig = TIMER_CFG_PERIODIC_UP,
                                     .trigDelay = 200,

};

#endif

#ifdef PART_TM4C1294NCPDT

const distSensor_t HCSR04 = {
                                     .trigGpioPin = GPIO_PIN_4,
                                     .echoGpioPin = GPIO_PIN_5,
                                     .trigGpioPortBase = GPIO_PORTN_BASE,
                                     .echoGpioPortBase = GPIO_PORTN_BASE,
                                     .echoGpioPortPeriph = SYSCTL_PERIPH_GPION,
                                     .trigGpioPortPeriph = SYSCTL_PERIPH_GPION,
                                     .echoIntType = GPIO_BOTH_EDGES,
                                     .timerPortPeriph = SYSCTL_PERIPH_TIMER1,
                                     .timerBase = TIMER1_BASE,
                                     .timerSubType = TIMER_A,
                                     .timerConfig = TIMER_CFG_PERIODIC_UP,
                                     .trigDelay = 1200,

};

#endif

volatile uint16_t echo_flag;
volatile uint32_t echo_duration;
volatile uint32_t echo_1;
volatile uint32_t echo_2;


void distSensor_Config(const distSensor_t HC_SR04){

    SysCtlPeripheralEnable(HC_SR04.trigGpioPortPeriph);
    SysCtlPeripheralEnable(HC_SR04.echoGpioPortPeriph);
    SysCtlPeripheralEnable(HC_SR04.timerPortPeriph);

    GPIOPinTypeGPIOOutput(HC_SR04.trigGpioPortBase, HC_SR04.trigGpioPin);
    GPIOPinTypeGPIOInput(HC_SR04.echoGpioPortBase, HC_SR04.echoGpioPin);

    TimerConfigure(HC_SR04.timerBase, HC_SR04.timerConfig);
    TimerEnable(HC_SR04.timerBase, HC_SR04.timerSubType);

    //GPIOIntTypeSet(HC_SR04.echoGpioPortBase, HC_SR04.echoGpioPin, HC_SR04.echoIntType);
    //GPIOIntRegister(HC_SR04.echoGpioPortBase, distSensor_echoIntHandler);
    //GPIOIntEnable(HC_SR04.echoGpioPortBase, HC_SR04.echoGpioPin);

}

void distSensor_triggerPulse(const distSensor_t HC_SR04){

    /* Pull up trigger pin to trigger ultrasound sensor */
    GPIOPinWrite(HC_SR04.trigGpioPortBase, HC_SR04.trigGpioPin, HC_SR04.trigGpioPin);

    /* Wait for 10 ms */
    SysCtlDelay(HC_SR04.trigDelay);

    /* Pull down trigger pin */
    GPIOPinWrite(HC_SR04.trigGpioPortBase, HC_SR04.trigGpioPin, 0x00);

}

uint32_t distSensor_getDistance(const distSensor_t HC_SR04){

         int32_t echoPinRead;
         uint32_t distance_cm;
         uint32_t echo_start;
         uint32_t echo_end;

         distance_cm = 0;

         /* Send trigger pulse to ultrasonic sensor */
         distSensor_triggerPulse(HC_SR04);

         /* Wait for rising edge of echo pulse */
         while((echoPinRead = GPIOPinRead(HC_SR04.echoGpioPortBase, HC_SR04.echoGpioPin)) != HC_SR04.echoGpioPin);

         /* Reload timer ticks */
         HWREG(HC_SR04.timerBase + TIMER_O_TAV) = 0;
         TimerEnable(HC_SR04.timerBase, HC_SR04.timerSubType);

         /* Read timer tick counts for echo start */
         echo_start  = TimerValueGet(HC_SR04.timerBase, HC_SR04.timerSubType);

         /* Wait for falling edge of echo pulse */
         while((echoPinRead = GPIOPinRead(HC_SR04.echoGpioPortBase, HC_SR04.echoGpioPin)) == HC_SR04.echoGpioPin);

         /* Read timer tick counts for echo end */
         echo_end = TimerValueGet(HC_SR04.timerBase, HC_SR04.timerSubType);

         /* Calculate distance from echo pulse start and end timer tick count (@120 Mhz) */
         distance_cm = (((echo_end - echo_start)/(TIMER_CLOCK_FREQUENCY/TICKS_PER_USEC))/58);

         return distance_cm;
}


void distSensor_echoIntHandler(void){

    uint32_t status=0;
    uint8_t pinVal;
    status = GPIOIntStatus(GPIO_PORTN_BASE,true);
    GPIOIntClear(GPIO_PORTN_BASE, status);

    /* Check for echo input interrupt */
    if(status & GPIO_INT_PIN_5){
        // PB0 - Echo Pin
        pinVal = (uint8_t)(GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_5) & GPIO_PIN_5);
        /* Rising edge */
        if(pinVal){
            echo_flag = 1;
            HWREG(TIMER1_BASE + TIMER_O_TAV) = 0;
            TimerEnable(TIMER1_BASE,TIMER_A);
            echo_1  = TimerValueGet(TIMER1_BASE,TIMER_A);
        }
        /* Falling edge */
        else{
            echo_flag = 0;
            echo_2 = TimerValueGet(TIMER1_BASE,TIMER_A);
            TimerDisable(TIMER1_BASE,TIMER_A);
        }

    }

}



