/* Authors: Sarthak Jain, Vatsal Sheth and Hardik Senjaliya
 * Dated: 05/02/2019
 * button.h
 * This file includes headers required for button.c
 */

#ifndef INCLUDE_BUTTON_H_
#define INCLUDE_BUTTON_H_

#include "motor_task.h"

void Button_ISR(void);
void ConfigureButton();

uint16_t button_flag;
uint8_t startup, obstacle;


#endif /* INCLUDE_BUTTON_H_ */
