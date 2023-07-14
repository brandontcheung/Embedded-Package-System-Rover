/*
 * custom_queue.h
 *
 *  Created on: Sep 30, 2019
 *      Author: Brandon
 */

#ifndef CUSTOM_QUEUE_H_
#define CUSTOM_QUEUE_H_


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <FreeRTOS.h>


#include <queue.h>

#define QUEUE_SIZE 10

QueueHandle_t  message_queue1;
QueueHandle_t  message_queue2;
QueueHandle_t  message_queue3;
QueueHandle_t  message_queue4;
QueueHandle_t  message_queue5;
QueueHandle_t  message_queue6;

typedef struct queueNode queueNode;




void sensor_queue_init();

int sendSPIMsgToQ1(unsigned int spiVal);
int sendWIFIMsgToQ1(unsigned int wifiVal);

int sendMotorMsgToQ2(unsigned int motorVal);
int sendTimeMsgToQ1(unsigned int timeVal);
int sendJsonMsgToQ3(int jsonVal, unsigned int type);
int sendWIFIMsgToQ4(unsigned int wifiVal);

int dequeueCustomQ1(unsigned int *spiVal, unsigned int *timeVal, unsigned int *wifiVal, unsigned int *type);
int dequeueCustomQ2(unsigned int *motorVal, unsigned int *type);
int dequeueCustomQ3(unsigned int *jsonVal, unsigned int *type);
int dequeueCustomQ4(unsigned int *wifiVal, unsigned int *type);

int sendSPIMsgToQ5(unsigned int spiVal);
int dequeueCustomQ5(unsigned int *spiVal, unsigned int *type);

int sendWIFIMsgToQ6(unsigned int WifiVal, unsigned int type);
int dequeueCustomQ6(unsigned int *WifiVal, unsigned int *type);

#endif /* CUSTOM_QUEUE_H_ */
