/*
 * encoder_spi.h
 *
 *  Created on: Oct 8, 2019
 *      Author: Brandon
 */

#ifndef ENCODER_SPI_H_
#define ENCODER_SPI_H_

#include <ti/drivers/SPI.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>

/* Example/Board Header files */
#include "Board.h"

SPI_Handle masterSpi;
//#define SPI_MSG_LENGTH  0x3
#define SPI_MSG_LENGTH  0x5


sem_t masterSem;
uint8_t masterRxBuffer[SPI_MSG_LENGTH];
uint8_t masterTxBuffer[SPI_MSG_LENGTH];

uint8_t masterRxBufferinit[SPI_MSG_LENGTH - 3];
uint8_t masterTxBufferinit[SPI_MSG_LENGTH - 3];
SPI_Transaction transaction;

void initSpi();
void spiOP(int chipselect);
void spiR3();




#endif /* ENCODER_SPI_H_ */
