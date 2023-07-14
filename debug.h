/*
 * debug.h
 *
 *  Created on: Oct 6, 2019
 *      Author: Brandon
 */

#ifndef DEBUG_H_
#define DEBUG_H_
#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>

#define TESTTHREAD_START            0x60
#define TESTTHREAD_BEFORE_WHILE     0x61
#define TESTHREAD_BEFORE_UART       0x62
#define TESTTHREAD_AFTER_UART       0x63



#define ERROR_TEST                  0x70


void dbgOutputLoc(unsigned int outLoc);
void errorRoutine(unsigned int errorCode);

#endif /* DEBUG_H_ */
