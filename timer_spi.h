/*
 * timer_spi.h
 *
 *  Created on: Nov 6, 2019
 *      Author: Brandon
 */

#ifndef TIMER_SPI_H_
#define TIMER_SPI_H_

#include "custom_queue.h"
#include <stddef.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Timer.h>

/* Board Header file */
#include "Board.h"

//#include "debug.h"

#define timerSPIPeriod 100000
Timer_Handle timerSPI;

void timer_spi_init_custom();



#endif /* TIMER_SPI_H_ */
