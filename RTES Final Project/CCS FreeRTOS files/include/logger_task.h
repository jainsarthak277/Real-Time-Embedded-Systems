/*
 * logger_task.h
 *
 *  Created on: 08-Apr-2019
 *      Author: hardi
 */

#ifndef INC_LOGGER_TASK_H_
#define INC_LOGGER_TASK_H_

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "FreeRTOSConfig.h"


#define TASKSTACKSIZE               (128)
#define PRIORITY_LOGGER_TASK          (2)

typedef struct{

    char name[7];
    TickType_t time_stamp;
    int toggle_count;
}led_message_t;


typedef struct{

    TickType_t time_stamp;
    double temperature;
}temp_message_t;

uint32_t TaskLoggerInit(void);


#endif /* INC_LOGGER_TASK_H_ */
