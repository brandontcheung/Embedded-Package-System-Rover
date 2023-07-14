/*
 * debug.c
 *
 *  Created on: Oct 6, 2019
 *      Author: Brandon
 */

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>

#include <stdint.h>
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

/* Example/Board Header files */
#include "Board.h"
#include "debug.h"

#include <ti/devices/cc32xx/inc/hw_types.h>
#include <ti/devices/cc32xx/driverlib/gpio.h>
#include <ti/devices/cc32xx/inc/hw_memmap.h>

#include <task.h>

void dbgOutputLoc(unsigned int outLoc)
{
    GPIO_write(Board_GPIO6, ((outLoc >> 6 ) & 0x1));
    GPIO_write(Board_GPIO5, ((outLoc >> 5 ) & 0x1));
    GPIO_write(Board_GPIO4, ((outLoc >> 4 ) & 0x1));
    GPIO_write(Board_GPIO3, ((outLoc >> 3 ) & 0x1));
    GPIO_write(Board_GPIO2, ((outLoc >> 2 ) & 0x1));
    GPIO_write(Board_GPIO1, ((outLoc >> 1 ) & 0x1));
    GPIO_write(Board_GPIO0, (outLoc & 0x1));
    GPIO_toggle(Board_GPIO7);
}


void errorRoutine(unsigned int errorCode)
{
    //change to stop interupts and threads
    dbgOutputLoc(errorCode);
    vTaskSuspendAll();
    taskENTER_CRITICAL();
}
//GPIO_toggle(Board_GPIO_LED0);
