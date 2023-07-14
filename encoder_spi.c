/*
 * encoder_spi.c
 *
 *  Created on: Oct 8, 2019
 *      Author: Brandon
 */

#include "encoder_spi.h"
#include "motor_uart.h"
#include "custom_queue.h"



void slaveCallback(SPI_Handle spi, SPI_Transaction *tran)
{
    char test = tran->arg;
    uint32_t c2;
    uint32_t * c1;
    uint8_t * c0;
    if(test == 'i')
    {
        GPIO_write(CS1, 1);
    }
    else if(test == 'j')
    {
        GPIO_write(CS2, 1);
    }
    else if(test == 'k')
    {
        GPIO_write(CS3, 1);
    }
    else
    {
        c0 = (uint8_t *) (tran->rxBuf);
        c1 = &c0[1];
        c2 = *c1;
        uint32_t c3 = 0x000000FF & c2;
        c3 = c3 << 8;
        c3 = c3 + (0x000000FF & (c2 >> 8));
        c3 = c3 << 8;
        c3 = c3 + (0x000000FF & (c2 >> 16));
        c3 = c3 << 8;
        c3 = c3 + (0x000000FF & (c2 >> 24));
        int c4 = (int) c3;

        if(test == '1')
        {
            GPIO_write(CS1, 1);
        }
        else if(test == '2')
        {
            GPIO_write(CS2, 1);
        }
        else if(test == '3')
       {
           GPIO_write(CS3, 1);
       }
        sendSPIMsgToQ1(c3);
    }
    sem_post(&masterSem);
}

void initSpi()
{
    SPI_Params spiParams;
    bool transferOK;
    static char ex1 = 'i';
    static char ex2 = 'j';
    static char ex3 = 'k';
    SPI_init();
    sem_init(&masterSem, 0, 1);



    SPI_Params_init(&spiParams);
    spiParams.mode = SPI_MASTER;
    spiParams.transferMode = SPI_MODE_CALLBACK;
    spiParams.transferCallbackFxn = slaveCallback;
    spiParams.bitRate = 1000000;
    spiParams.frameFormat = SPI_POL0_PHA0;
    masterSpi = SPI_open(Board_SPI0, &spiParams);
    if (masterSpi == NULL)
    {
        while (1);
    }

    sem_wait(&masterSem);
    masterTxBufferinit[0] = 0x88;
    masterTxBufferinit[1] = 0x03;

    transaction.arg   = (void *) ex1;
    transaction.count = 2;
    transaction.txBuf = masterTxBufferinit;
    transaction.rxBuf = masterRxBufferinit;
    GPIO_write(CS1, 0);
    transferOK = SPI_transfer(masterSpi, &transaction);
    if(!transferOK)
    {
        while(1);
    }

    sem_wait(&masterSem);
    GPIO_write(CS2, 0);
    transaction.arg   = (void *) ex2;
    transferOK = SPI_transfer(masterSpi, &transaction);
    if(!transferOK)
    {
        while(1);
    }

    sem_wait(&masterSem);
    GPIO_write(CS3, 0);
    transaction.arg   = (void *) ex3;
    transferOK = SPI_transfer(masterSpi, &transaction);
    if(!transferOK)
    {
        while(1);
    }
}

void spiOP(int chipselect)
{
    bool transferOK;
    if(chipselect == 1)
    {
        static char enc = '1';
        sem_wait(&masterSem);

        masterTxBuffer[0] = 0x60;
        masterTxBuffer[1] = 0x00;
        masterTxBuffer[2] = 0x00;
        masterTxBuffer[3] = 0x00;
        masterTxBuffer[4] = 0x00;
        transaction.arg   = (void *) enc;
        transaction.count = SPI_MSG_LENGTH;
        transaction.txBuf = masterTxBuffer;
        transaction.rxBuf = masterRxBuffer;

        GPIO_write(CS1, 0);
        transferOK = SPI_transfer(masterSpi, &transaction);
        if(!transferOK)
        {
            while(1);
        }
    }
    else if (chipselect == 2)
    {
        static char enc = '2';
        sem_wait(&masterSem);

        masterTxBuffer[0] = 0x60;
        masterTxBuffer[1] = 0x00;
        masterTxBuffer[2] = 0x00;
        masterTxBuffer[3] = 0x00;
        masterTxBuffer[4] = 0x00;
        transaction.arg   = (void *) enc;
        transaction.count = SPI_MSG_LENGTH;
        transaction.txBuf = masterTxBuffer;
        transaction.rxBuf = masterRxBuffer;

        GPIO_write(CS2, 0);
        transferOK = SPI_transfer(masterSpi, &transaction);
        if(!transferOK)
        {
            while(1);
        }
    }
    else if (chipselect == 3)
    {
        static char enc = '3';
        sem_wait(&masterSem);

        masterTxBuffer[0] = 0x60;
        masterTxBuffer[1] = 0x00;
        masterTxBuffer[2] = 0x00;
        masterTxBuffer[3] = 0x00;
        masterTxBuffer[4] = 0x00;
        transaction.arg   = (void *) enc;
        transaction.count = SPI_MSG_LENGTH;
        transaction.txBuf = masterTxBuffer;
        transaction.rxBuf = masterRxBuffer;

        GPIO_write(CS3, 0);
        transferOK = SPI_transfer(masterSpi, &transaction);
        if(!transferOK)
        {
            while(1);
        }
    }
    else
    {
        //throw error
    }

}

void spiR3()
{
    //write to motor 3 addr = 130
    spiOP(1);
    //write to motor 2 addr = 129
    spiOP(2);
    //write to motor 1 addr = 128
    spiOP(3);
}
