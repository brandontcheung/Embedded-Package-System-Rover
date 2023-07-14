/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//*****************************************************************************
//
//! \addtogroup mqtt_server
//! @{
//
//*****************************************************************************
/* Standard includes                                                         */
#include <stdlib.h>

/* Kernel (Non OS/Free-RTOS/TI-RTOS) includes                                */
#include "pthread.h"
#include "mqueue.h"

/* Common interface includes                                                 */
#include "uart_term.h"

/* Application includes                                                      */
#include "client_cbs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "custom_queue.h"

extern bool gResetApplication;

//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************
#define APP_PRINT               Report

#define OS_WAIT_FOREVER         (0xFFFFFFFF)
#define OS_NO_WAIT              (0)
#define OS_OK                   (0)

#define MQTTClientCbs_ConnackRC(data) (data & 0xff) 
/**< CONNACK: Return Code (LSB) */

//*****************************************************************************
//                 GLOBAL VARIABLES
//*****************************************************************************

/* Message Queue                                                              */
extern mqd_t g_PBQueue;
extern char *topic[];
struct client_info client_info_table[MAX_CONNECTION];

//*****************************************************************************
//                 Queue external function
//*****************************************************************************
//extern int32_t MQTT_SendMsgToQueue(struct msgQueue *queueElement);

//****************************************************************************
//                      CLIENT CALLBACKS
//****************************************************************************

//*****************************************************************************
//
//! Callback in case of various event (for clients connection with remote
//! broker)
//!
//! \param[in]  event       - is a event occurred
//! \param[in]  metaData    - is the pointer for the message buffer
//!                           (for this event)
//! \param[in]  metaDateLen - is the length of the message buffer
//! \param[in]  data        - is the pointer to the buffer for data
//!                           (for this event)
//! \param[in]  dataLen     - is the length of the buffer data
//!
//! return none
//
//*****************************************************************************
void MqttClientCallback(int32_t event,
                        void * metaData,
                        uint32_t metaDateLen,
                        void *data,
                        uint32_t dataLen)
{
    int32_t i = 0;

    switch((MQTTClient_EventCB)event)
    {
    case MQTTClient_OPERATION_CB_EVENT:
    {
        switch(((MQTTClient_OperationMetaDataCB *)metaData)->messageType)
        {
        case MQTTCLIENT_OPERATION_CONNACK:
        {
            uint16_t *ConnACK = (uint16_t*) data;
            APP_PRINT("CONNACK:\n\r");
            /* Check if Conn Ack return value is Success (0) or       */
            /* Error - Negative value                                 */
            if(0 == (MQTTClientCbs_ConnackRC(*ConnACK)))
            {
                APP_PRINT("Connection Success\n\r");
            }
            else
            {
                APP_PRINT("Connection Error: %d\n\r", *ConnACK);
            }
            break;
        }

        case MQTTCLIENT_OPERATION_EVT_PUBACK:
        {
            char *PubAck = (char *) data;
            APP_PRINT("PubAck:\n\r");
            APP_PRINT("%s\n\r", PubAck);
            break;
        }

        case MQTTCLIENT_OPERATION_SUBACK:
        {
            APP_PRINT("Sub Ack:\n\r");
            APP_PRINT("Granted QoS Levels are:\n\r");
            for(i = 0; i < dataLen; i++)
            {
                APP_PRINT("%s :QoS %d\n\r", topic[i],
                          ((unsigned char*) data)[i]);
            }
            break;
        }

        case MQTTCLIENT_OPERATION_UNSUBACK:
        {
            char *UnSub = (char *) data;
            APP_PRINT("UnSub Ack \n\r");
            APP_PRINT("%s\n\r", UnSub);
            break;
        }

        default:
            break;
        }
        break;
    }
    case MQTTClient_RECV_CB_EVENT:
    {
        MQTTClient_RecvMetaDataCB *recvMetaData =
            (MQTTClient_RecvMetaDataCB *)metaData;
        uint32_t bufSizeReqd = 0;
        uint32_t topicOffset;
        uint32_t payloadOffset;

        struct publishMsgHeader msgHead;

        char *pubBuff = NULL;
        char A[] = "                                                                                                                          ";

        topicOffset = sizeof(struct publishMsgHeader);
        payloadOffset = sizeof(struct publishMsgHeader) +
                        recvMetaData->topLen + 1;

        bufSizeReqd += sizeof(struct publishMsgHeader);
        bufSizeReqd += recvMetaData->topLen + 1;
        bufSizeReqd += dataLen + 1;
        pubBuff = &A;

        if(pubBuff == NULL)
        {
            APP_PRINT("malloc failed: recv_cb\n\r");
            return;
        }

        msgHead.topicLen = recvMetaData->topLen;
        msgHead.payLen = dataLen;
        msgHead.retain = recvMetaData->retain;
        msgHead.dup = recvMetaData->dup;
        msgHead.qos = recvMetaData->qos;
        memcpy((void*) pubBuff, &msgHead, sizeof(struct publishMsgHeader));

        /* copying the topic name into the buffer                        */
        memcpy((void*) (pubBuff + topicOffset),
               (const void*)recvMetaData->topic,
               recvMetaData->topLen);
        memset((void*) (pubBuff + topicOffset + recvMetaData->topLen),'\0',1);

        /* copying the payload into the buffer                           */
        memcpy((void*) (pubBuff + payloadOffset), (const void*) data, dataLen);
        memset((void*) (pubBuff + payloadOffset + dataLen), '\0', 1);

        char dataArray1[] = "                                                                          ";
        char dataArray2[] = "                                                                                                                                       ";
        char dataArray3[] = "                                                                          ";

        char * data1P = &dataArray1;
        char * data2P = &dataArray2;
        char * data3P = &dataArray3;

        sprintf(dataArray1, pubBuff + topicOffset);
        sprintf(dataArray2, pubBuff + payloadOffset);
        sprintf(dataArray3, "%d", recvMetaData->qos);

        if(recvMetaData->retain)
        {
            APP_PRINT("Retained\n\r");
        }

        if(recvMetaData->dup)
        {
            APP_PRINT("Duplicate\n\r");
        }
        char bufferRecipient[64] = {0};
        char bufferSender[64] = {0};
        char bufferMsgID[64] = {0};
        char bufferBody[64] = {0};
        char bufferElse[64] = {0};
        //char *JSON_STRING = "{\"Recipient\": ARM1, \"Sender\": Rover, \"MSGID\": 15637, \"Body\": \"Hi From Rover\"}";
        int test_Parser= jsonParser_MQTT(data2P, bufferRecipient, bufferSender, bufferMsgID, bufferBody, bufferElse);
        /* signal to the main task                                        */
        unsigned int type = 4;

        char * IDP = &bufferMsgID;
        unsigned int WifiVal = atoi(IDP);
        if (sendWIFIMsgToQ6(WifiVal, type) == 0)
        {
            //errorRoutine(Error_QUEUE_TIME_SEND);
        }
        int val = 0;
        unsigned int q3type = 777;
        int send = 0;
        if(bufferBody[0] == 'i' && bufferBody[1] == 'n' && bufferBody[2] == 'i' && bufferBody[3] == 't')
        {
            q3type = 0;
            send = 1;
        }
        else if(bufferBody[0] == 's' && bufferBody[1] == 'p' && bufferBody[2] == 'i' && bufferBody[3] == '1')
        {
            q3type = 1;
            send = 1;
        }
        else if(bufferBody[0] == 's' && bufferBody[1] == 'p' && bufferBody[2] == 'i' && bufferBody[3] == '2')
        {
            q3type = 2;
            send = 1;
        }
        else if(bufferBody[0] == 's' && bufferBody[1] == 'p' && bufferBody[2] == 'i' && bufferBody[3] == '3')
        {
            q3type = 3;
            send = 1;
        }
        else if(bufferBody[0] == 'g' && bufferBody[1] == 'o' && bufferBody[2] == ' ' && bufferBody[3] == 'f')
        {
            q3type = 4;
            send = 1;
        }
        else if(bufferBody[0] == 'g' && bufferBody[1] == 'o' && bufferBody[2] == ' ' && bufferBody[3] == 'b')
        {
            q3type = 6;
            send = 1;
        }
        else if(bufferBody[0] == 's' && bufferBody[1] == 't' && bufferBody[2] == 'o' && bufferBody[3] == 'p')
        {
            q3type = 5;
            send = 1;
        }
        else if(bufferBody[0] == 's' && bufferBody[1] == 'p' && bufferBody[2] == 'i' && bufferBody[3] == 'n' && bufferBody[4] == 'l')
        {
            q3type = 7;
            send = 1;
        }
        else if(bufferBody[0] == 's' && bufferBody[1] == 'p' && bufferBody[2] == 'i' && bufferBody[3] == 'n' && bufferBody[4] == 'r')
        {
            q3type = 8;
            send = 1;
        }
        else if(bufferBody[0] == 'e' && bufferBody[1] == 'r' && bufferBody[2] == 'r' && bufferBody[3] == 'o' && bufferBody[4] == 'r')
        {
            q3type = 9;
            send = 1;
        }
        else if(bufferBody[0] == '1' || bufferBody[0] == '2' || bufferBody[0] == '0')
        {
            q3type = 10;
            char * IDP = &bufferBody;
            val = 0;

            uint32_t motorMSG = 0;
            uint8_t speed1;
            uint8_t speed2;
            uint8_t speed3;
            char input1 [4];
            char input2 [4];
            char input3 [4];
            input1[0] = bufferBody[0];
            input1[1] = bufferBody[1];
            input1[2] = bufferBody[2];
            input1[3] = '\0';

            input2[0] = bufferBody[3];
            input2[1] = bufferBody[4];
            input2[2] = bufferBody[5];
            input2[3] = '\0';

            input3[0] = bufferBody[6];
            input3[1] = bufferBody[7];
            input3[2] = bufferBody[8];
            input3[3] = '\0';
            speed1 = atoi(input1);
            speed2 = atoi(input2);
            speed3 = atoi(input3);
            val = val + speed1;
            val = (val << 8) + speed2;
            val = (val << 8) + speed3;
            send = 1;
        }

        if(send == 1)
        {
            if (sendJsonMsgToQ3(val, q3type))
            {
                //errorRoutine(Error_QUEUE_TIME_SEND);
            }
        }


        break;
    }
    case MQTTClient_DISCONNECT_CB_EVENT:
    {
        gResetApplication = true;
        APP_PRINT("BRIDGE DISCONNECTION\n\r");
        break;
    }
    }
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
