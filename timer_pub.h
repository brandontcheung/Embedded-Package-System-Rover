/*
 * timer_pub.h
 *
 *  Created on: Oct 21, 2019
 *      Author: Brandon
 */

#ifndef TIMER_PUB_H_
#define TIMER_PUB_H_

#include "custom_queue.h"
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>

/* Board Header file */
#include "Board.h"

//#include "debug.h"

#define timerQPeriod 200000
Timer_Handle timerQ;

void timer_pub_init_custom();


#endif /* TIMER_PUB_H_ */
