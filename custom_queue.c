/*
 * custom_queue.c
 *
 *  Created on: Sep 30, 2019
 *      Author: Brandon
 */

#include <FreeRTOS.h>
#include <queue.h>
#include "custom_queue.h"
#include <ti/drivers/dpl/HwiP.h>


void sensor_queue_init()
{
    message_queue1 = xQueueCreate(QUEUE_SIZE, sizeof(uint64_t));
    if(message_queue1 == NULL)
    {
        //errorRoutine(Error_QUEUE_Ini);
    }

    message_queue2 = xQueueCreate(QUEUE_SIZE, sizeof(uint64_t));
    if(message_queue2 == NULL)
    {
        //errorRoutine(Error_QUEUE_Ini);
    }

    message_queue3 = xQueueCreate(QUEUE_SIZE, sizeof(uint64_t));
    if(message_queue3 == NULL)
    {
        //errorRoutine(Error_QUEUE_Ini);
    }

    message_queue4 = xQueueCreate(QUEUE_SIZE, sizeof(uint64_t));
    if(message_queue4 == NULL)
    {
        //errorRoutine(Error_QUEUE_Ini);
    }

    message_queue5 = xQueueCreate(QUEUE_SIZE, sizeof(uint64_t));
    if(message_queue5 == NULL)
    {
        //errorRoutine(Error_QUEUE_Ini);
    }

    message_queue6 = xQueueCreate(QUEUE_SIZE, sizeof(uint64_t));
    if(message_queue6 == NULL)
    {
        //errorRoutine(Error_QUEUE_Ini);
    }
}


int sendSPIMsgToQ1(unsigned int spiVal)
{
    uint64_t buffer = 0x1111111100000000;
    uint64_t time = (0x00000000FFFFFFFF & spiVal);
    buffer = buffer + time;
    uint64_t *pbuffer = &buffer;
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t status = xQueueSendToBackFromISR(message_queue1, (void *) pbuffer, &xHigherPriorityTaskWoken);
    if(status == pdFALSE)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int sendWIFIMsgToQ1(unsigned int wifiVal)
{
    uint64_t buffer = 0x2222222200000000;
    uint64_t time = (0x00000000FFFFFFFF & wifiVal);
    buffer = buffer + time;
    uint64_t *pbuffer = &buffer;
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t status = xQueueSendToBack(message_queue1, (void *) pbuffer, &xHigherPriorityTaskWoken);
    if(status == pdFALSE)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int sendTimeMsgToQ1(unsigned int timeVal)
{
    uint64_t buffer = 0x4444444400000000;
    uint64_t time = (0x00000000FFFFFFFF & timeVal);
    buffer = buffer + time;
    uint64_t *pbuffer = &buffer;
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t status = xQueueSendToBackFromISR(message_queue1, (void *) pbuffer, &xHigherPriorityTaskWoken);
    if(status == pdFALSE)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int sendMotorMsgToQ2(unsigned int motorVal)
{
       uint64_t buffer = 0x3333333300000000;
       uint64_t dist = (0xFFFFFFFF & motorVal);
       buffer = buffer + dist;
       uint64_t *pbuffer = &buffer;
       BaseType_t xHigherPriorityTaskWoken;
       xHigherPriorityTaskWoken = pdFALSE;
       BaseType_t status = xQueueSendToBack(message_queue2, (void *) pbuffer, &xHigherPriorityTaskWoken);
       if(status == pdFALSE)
       {
           return 0;
       }
       else
       {
           return 1;
       }
}

int sendJsonMsgToQ3(int jsonVal, unsigned int type)
{
    uint64_t buffer = 0;
    if(type == 0)
    {
        buffer = 0x1111111100000000;
    }
    else if(type == 1)
    {
        buffer = 0x2222222200000000;
    }
    else if(type == 2)
    {
        buffer = 0x3333333300000000;
    }
    else if(type == 3)
    {
        buffer = 0x4444444400000000;
    }
    else if(type == 4)
    {
        buffer = 0x5555555500000000;
    }
    else if(type == 5)
    {
        buffer = 0x6666666600000000;
    }
    else if(type == 6)
    {
        buffer = 0x7777777700000000;
    }
    else if(type == 7)
    {
        buffer = 0x8888888800000000;
    }
    else if(type == 8)
    {
        buffer = 0x9999999900000000;
    }
    else if(type == 9)
    {
        buffer = 0xAAAAAAAA00000000;
    }
    else if(type == 10)
    {
        buffer = 0xBBBBBBBB00000000;
    }
    uint64_t time = (0x00000000FFFFFFFF & jsonVal);
    buffer = buffer + time;
    uint64_t *pbuffer = &buffer;
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;;
    BaseType_t status;
    if(HwiP_inISR())
    {
        status = xQueueSendToBackFromISR(message_queue3, (void *) pbuffer, &xHigherPriorityTaskWoken);
    }
    else
    {
        status = xQueueSendToBack(message_queue3, (void *) pbuffer, &xHigherPriorityTaskWoken);
    }

    if(status == pdFALSE)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int sendWIFIMsgToQ4(unsigned int wifiVal)
{
    uint64_t buffer = 0x9999999900000000;
    uint64_t time = (0x00000000FFFFFFFF & wifiVal);
    buffer = buffer + time;
    uint64_t *pbuffer = &buffer;
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t status = xQueueSendToBack(message_queue4, (void *) pbuffer, &xHigherPriorityTaskWoken);
    if(status == pdFALSE)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int dequeueCustomQ1(unsigned int *spiVal, unsigned int *timeVal, unsigned int *wifiVal, unsigned int *type)
{
    uint64_t buffer = 0;
    uint64_t *pbuffer = &buffer;
    xQueueReceive(message_queue1, pbuffer, portMAX_DELAY);

    uint64_t number = buffer & 0xFFFFFFFF00000000;
    if(number == 0x1111111100000000)
    {
        *spiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 0;
        return 1;
    }
    else if(number == 0x4444444400000000)
    {
        *timeVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 1;
        return 1;
    }
    else if(number == 0x2222222200000000)
    {
        *wifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 2;
        return 1;
    }
    else
    {
        return 0;
    }
}

int dequeueCustomQ2(unsigned int *motorVal, unsigned int *type)
{
    uint64_t buffer = 0;
    uint64_t *pbuffer = &buffer;
    xQueueReceive(message_queue2, pbuffer, portMAX_DELAY);

    uint64_t number = buffer & 0xFFFFFFFF00000000;
    if(number == 0x3333333300000000)
    {
        *motorVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}


int dequeueCustomQ3(unsigned int *jsonVal, unsigned int *type)
{
    uint64_t buffer = 0;
   uint64_t *pbuffer = &buffer;
   xQueueReceive(message_queue3, pbuffer, portMAX_DELAY);

   uint64_t number = buffer & 0xFFFFFFFF00000000;
   if(number == 0x1111111100000000)
   {
       *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
       *type = 0;
       return 1;
   }
   else if(number == 0x2222222200000000)
   {
       *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
       *type = 1;
       return 1;
   }
   else if(number == 0x3333333300000000)
   {
       *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
       *type = 2;
       return 1;
   }
   else if(number == 0x4444444400000000)
   {
       *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
       *type = 3;
       return 1;
   }
   else if(number == 0x5555555500000000)
   {
       *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
       *type = 4;
       return 1;
   }
   else if(number == 0x6666666600000000)
   {
       *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
       *type = 5;
       return 1;
   }
   else if(number == 0x7777777700000000)
   {
       *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
       *type = 6;
       return 1;
   }
   else if(number == 0x8888888800000000)
  {
      *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
      *type = 7;
      return 1;
  }
   else if(number == 0x9999999900000000)
     {
         *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
         *type = 8;
         return 1;
     }
   else if(number == 0xAAAAAAAA00000000)
     {
         *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
         *type = 9;
         return 1;
     }
   else if(number == 0xBBBBBBBB00000000)
     {
         *jsonVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
         *type = 10;
         return 1;
     }
   else
   {
       return 0;
   }
}

int dequeueCustomQ4(unsigned int *wifiVal, unsigned int *type)
{
    uint64_t buffer = 0;
    uint64_t *pbuffer = &buffer;
    xQueueReceive(message_queue3, pbuffer, portMAX_DELAY);

    uint64_t number = buffer & 0xFFFFFFFF00000000;
    if(number == 0x9999999900000000)
    {
        *wifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

int sendSPIMsgToQ5(unsigned int spiVal)
{
    uint64_t buffer = 0x1111111100000000;
    uint64_t time = (0x00000000FFFFFFFF & spiVal);
    buffer = buffer + time;
    uint64_t *pbuffer = &buffer;
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t status = xQueueSendToBack(message_queue5, (void *) pbuffer, &xHigherPriorityTaskWoken);
    if(status == pdFALSE)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int dequeueCustomQ5(unsigned int *spiVal, unsigned int *type)
{
    uint64_t buffer = 0;
    uint64_t *pbuffer = &buffer;
    xQueueReceive(message_queue5, pbuffer, portMAX_DELAY);

    uint64_t number = buffer & 0xFFFFFFFF00000000;
    if(number == 0x1111111100000000)
    {
        *spiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

int sendWIFIMsgToQ6(unsigned int WifiVal, unsigned int type)
{
    uint64_t buffer = 0x1111111100000000;
    if(type == 1)
    {
        buffer = 0x2222222200000000;
    }
    else if(type == 2)
    {
        buffer = 0x3333333300000000;
    }
    else if(type == 3)
    {
        buffer = 0x4444444400000000;
    }
    else if(type == 4)
    {
        buffer = 0x5555555500000000;
    }
    else if(type == 5)
    {
        buffer = 0x6666666600000000;
    }
    else if(type == 6)
    {
        buffer = 0x7777777700000000;
    }
    else if(type == 7)
    {
        buffer = 0x8888888800000000;
    }
    else if(type == 8)
    {
        buffer = 0x9999999900000000;
    }
    else if(type == 9)
    {
        buffer = 0xAAAAAAAA00000000;
    }
    uint64_t time = (0x00000000FFFFFFFF & WifiVal);
    buffer = buffer + time;
    uint64_t *pbuffer = &buffer;
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;;
    BaseType_t status;
    if(HwiP_inISR())
    {
        status = xQueueSendToBackFromISR(message_queue6, (void *) pbuffer, &xHigherPriorityTaskWoken);
    }
    else
    {
        status = xQueueSendToBack(message_queue6, (void *) pbuffer, &xHigherPriorityTaskWoken);
    }

    if(status == pdFALSE)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int dequeueCustomQ6(unsigned int *WifiVal, unsigned int *type)
{
    uint64_t buffer = 0;
    uint64_t *pbuffer = &buffer;
    xQueueReceive(message_queue6, pbuffer, portMAX_DELAY);

    uint64_t number = buffer & 0xFFFFFFFF00000000;
    if(number == 0x1111111100000000)
    {
        *WifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 0;
        return 1;
    }
    else if(number == 0x2222222200000000)
    {
        *WifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 1;
        return 1;
    }
    else if(number == 0x3333333300000000)
    {
        *WifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 2;
        return 1;
    }
    else if(number == 0x4444444400000000)
    {
        *WifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 3;
        return 1;
    }
    else if(number == 0x5555555500000000)
    {
        *WifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 4;
        return 1;
    }
    else if(number == 0x6666666600000000)
    {
        *WifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 5;
        return 1;
    }
    else if(number == 0x7777777700000000)
    {
        *WifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 6;
        return 1;
    }
    else if(number == 0x8888888800000000)
    {
        *WifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 7;
        return 1;
    }
    else if(number == 0x9999999900000000)
    {
        *WifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 8;
        return 1;
    }
    else if(number == 0xAAAAAAAA00000000)
    {
        *WifiVal = (unsigned int) (0x00000000FFFFFFFF & buffer);
        *type = 9;
        return 1;
    }
    else
    {
        return 0;
    }
}
