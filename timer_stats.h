/*
 * timer_stats.h
 *
 *  Created on: Oct 22, 2019
 *      Author: Brandon
 */

#ifndef TIMER_STATS_H_
#define TIMER_STATS_H_


#include "custom_queue.h"
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>

/* Board Header file */
#include "Board.h"

//#include "debug.h"

#define timerSPeriod 1000000
Timer_Handle timerS;

void timer_stat_init_custom();


#endif /* TIMER_STATS_H_ */
