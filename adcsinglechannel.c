/*
 * Copyright (c) 2016-2019, Texas Instruments Incorporated
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

/*
 *  ======== adcsinglechannel.c ========
 */
#include <stdint.h>
#include <stddef.h>

/* POSIX Header files */
#include <pthread.h>

/* Driver Header files */
#include <ti/drivers/ADC.h>
#include <ti/display/Display.h>

/* Example/Board Header files */
#include "Board.h"

#define THREADSTACKSIZE   (768)

#include "custom_queue.h"
#include "encoder_spi.h"
#include "motor_uart.h"
#include "PartialDefinitions.h"
#include "debug.h"
#include <stdio.h>

/* Standard includes                                                         */
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>
#include <time.h>
#include <unistd.h>
#include "timer_pub.h"
#include "timer_stats.h"
#include "timer_spi.h"

#include "jsonParser.h"

#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>

/* Simplelink includes                                                       */
#include <ti/drivers/net/wifi/simplelink.h>

/* SlNetSock includes                                                        */
#include <ti/drivers/net/wifi/slnetifwifi.h>
/* MQTT Library includes                                                     */
#include <ti/net/mqtt/mqttclient.h>

/* Common interface includes                                                 */
#include "network_if.h"
#include "uart_term.h"

/* Application includes                                                      */
#include "Board.h"
#include "client_cbs.h"
#include "speedmath.h"

//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************
/* enables secured client                             mQQ                       */
//#define SECURE_CLIENT

/* enables client authentication by the server                               */
//#define CLNT_USR_PWD

#define CLIENT_INIT_STATE        (0x01)
#define MQTT_INIT_STATE          (0x04)

#define APPLICATION_VERSION      "1.1.1"
#define APPLICATION_NAME         "MQTT client"

#define SLNET_IF_WIFI_PRIO       (5)

/* Operate Lib in MQTT 3.1 mode.                                             */
#define MQTT_3_1_1               false
#define MQTT_3_1                 true

#define WILL_TOPIC               "Client"
#define WILL_MSG                 "Client Stopped"
#define WILL_QOS                 MQTT_QOS_1
#define WILL_RETAIN              false

/* Defining Broker IP address and port Number                                */
//#define SERVER_ADDRESS          "messagesight.demos.ibm.com"
#define SERVER_ADDRESS           "192.168.127.1"
#define SERVER_IP_ADDRESS        "192.168.127.1"
#define PORT_NUMBER              1883
#define SECURED_PORT_NUMBER      8883
#define LOOPBACK_PORT            1882

/* Clean session flag                                                        */
#define CLEAN_SESSION            true

/* Retain Flag. Used in publish message.                                     */
#define RETAIN_ENABLE            1

/* Defining Number of subscription topics                                    */
#define SUBSCRIPTION_TOPIC_COUNT 4

/* Defining Subscription Topic Values                                        */
#define SUBSCRIPTION_TOPIC0      "/topic/Board/R/R"
#define SUBSCRIPTION_TOPIC1      "topic/ARM1"
#define SUBSCRIPTION_TOPIC2      "/Team11/Brandon_sub"
#define SUBSCRIPTION_TOPIC3      "/topic/Test"

/* Defining Publish Topic Values                                             */
#define PUBLISH_TOPIC0           "/topic/Board/R/R"
#define PUBLISH_TOPIC0_DATA      "Self_Pub_Tested"

/* Spawn task priority and Task and Thread Stack Size                        */
#define TASKSTACKSIZE            2048
#define RXTASKSIZE               8192 //4096 //make two times bigger
#define MQTTTHREADSIZE           2048
#define SPAWN_TASK_PRIORITY      9

/* secured client requires time configuration, in order to verify server     */
/* certificate validity (date).                                              */

/* Day of month (DD format) range 1-31                                       */
#define DAY                      1
/* Month (MM format) in the range of 1-12                                    */
#define MONTH                    5
/* Year (YYYY format)                                                        */
#define YEAR                     2017
/* Hours in the range of 0-23                                                */
#define HOUR                     12
/* Minutes in the range of 0-59                                              */
#define MINUTES                  33
/* Seconds in the range of 0-59                                              */
#define SEC                      21

/* Number of files used for secure connection                                */
#define CLIENT_NUM_SECURE_FILES  1


//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*****************************************************************************
static void DisplayBanner(char * AppName);
void * MqttClient(void *pvParameters);
void Mqtt_ClientStop(uint8_t disconnect);
void Mqtt_ServerStop();
void Mqtt_Stop();
void Mqtt_start();
int32_t Mqtt_IF_Connect();
int32_t MqttServer_start();
int32_t MqttClient_start();


/* Connection state: (0) - connected, (negative) - disconnected              */
int32_t gApConnectionState = -1;
uint32_t memPtrCounterfree = 0;
bool gResetApplication = false;
static MQTTClient_Handle gMqttClient;
MQTTClient_Params MqttClientExmple_params;
//unsigned short g_usTimerInts;

/* Receive task handle                                                       */
pthread_t g_rx_task_hndl = (pthread_t) NULL;
uint32_t gUiConnFlag = 0;

/* AP Security Parameters                                                    */
SlWlanSecParams_t SecurityParams = { 0 };

/* Client ID                                                                 */
/* If ClientId isn't set, the MAC address of the device will be copied into  */
/* the ClientID parameter.                                                   */
char ClientId[13] = {'\0'};

/* Client User Name and Password                                             */
const char *ClientUsername = "username1";
const char *ClientPassword = "pwd1";

/* Subscription topics and qos values                                        */
char *topic[SUBSCRIPTION_TOPIC_COUNT] =
{ SUBSCRIPTION_TOPIC0, SUBSCRIPTION_TOPIC1, \
    SUBSCRIPTION_TOPIC2, SUBSCRIPTION_TOPIC3 };

unsigned char qos[SUBSCRIPTION_TOPIC_COUNT] =
{ MQTT_QOS_1, MQTT_QOS_1, MQTT_QOS_1, MQTT_QOS_1 };

/* Publishing topics and messages                                            */
const char *publish_topic = { PUBLISH_TOPIC0 };
const char *publish_data = { PUBLISH_TOPIC0_DATA };

pthread_t mqttThread = (pthread_t) NULL;
pthread_t appThread = (pthread_t) NULL;

/* Printing new line                                                         */
char lineBreak[] = "\n\r";

//*****************************************************************************
//                 Banner VARIABLES
//*****************************************************************************
#ifdef  SECURE_CLIENT

char *Mqtt_Client_secure_files[CLIENT_NUM_SECURE_FILES] = {"ca-cert.pem"};

/*Initialization structure to be used with sl_ExtMqtt_Init API. In order to  */
/*use secured socket method, the flag MQTTCLIENT_NETCONN_SEC, cipher,        */
/*n_files and secure_files must be configured.                               */
/*certificates also must be programmed  ("ca-cert.pem").                     */
/*The first parameter is a bit mask which configures server address type and */
/*security mode.                                                             */
/*Server address type: IPv4, IPv6 and URL must be declared with The          */
/*corresponding flag.                                                        */
/*Security mode: The flag MQTTCLIENT_NETCONN_SEC enables the security (TLS)  */
/*which includes domain name verification and certificate catalog            */
/*verification, those verifications can be disabled by adding to the bit mask*/
/*MQTTCLIENT_NETCONN_SKIP_DOMAIN_NAME_VERIFICATION and                       */
/*MQTTCLIENT_NETCONN_SKIP_CERTIFICATE_CATALOG_VERIFICATION flags             */
/*Example: MQTTCLIENT_NETCONN_IP6 | MQTTCLIENT_NETCONN_SEC |                 */
/*MQTTCLIENT_NETCONN_SKIP_CERTIFICATE_CATALOG_VERIFICATION                   */
/*For this bit mask, the IPv6 address type will be in use, the security      */
/*feature will be enable and the certificate catalog verification will be    */
/*skipped.                                                                   */
/*Note: The domain name verification requires URL Server address type        */
/*      otherwise, this verification will be disabled.                       */
MQTTClient_ConnParams Mqtt_ClientCtx =
{
    MQTTCLIENT_NETCONN_IP4 | MQTTCLIENT_NETCONN_SEC,
    SERVER_IP_ADDRESS,  //SERVER_ADDRESS,
    SECURED_PORT_NUMBER, //  PORT_NUMBER
    SLNETSOCK_SEC_METHOD_SSLv3_TLSV1_2,
    SLNETSOCK_SEC_CIPHER_FULL_LIST,
    CLIENT_NUM_SECURE_FILES,
    Mqtt_Client_secure_files
};

void setTime()
{
    SlDateTime_t dateTime = {0};
    dateTime.tm_day = (uint32_t)DAY;
    dateTime.tm_mon = (uint32_t)MONTH;
    dateTime.tm_year = (uint32_t)YEAR;
    dateTime.tm_hour = (uint32_t)HOUR;
    dateTime.tm_min = (uint32_t)MINUTES;
    dateTime.tm_sec = (uint32_t)SEC;
    sl_DeviceSet(SL_DEVICE_GENERAL, SL_DEVICE_GENERAL_DATE_TIME,
                 sizeof(SlDateTime_t), (uint8_t *)(&dateTime));
}

#else
MQTTClient_ConnParams Mqtt_ClientCtx =
{
    MQTTCLIENT_NETCONN_URL,
    SERVER_ADDRESS,
    PORT_NUMBER, 0, 0, 0,
    NULL
};
#endif

/* Initialize the will_param structure to the default will parameters        */
MQTTClient_Will will_param =
{
    WILL_TOPIC,
    WILL_MSG,
    WILL_QOS,
    WILL_RETAIN
};

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t    CC32xx %s Application       \n\r", AppName);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}

void * MqttClientThread(void * pvParameters)
{
    MQTTClient_run((MQTTClient_Handle)pvParameters);

    unsigned int type = 0;
    unsigned int WifiVal = 0;
    if (sendWIFIMsgToQ6(WifiVal, type) == 0)
    {
        //errorRoutine(Error_QUEUE_TIME_SEND);
    }
    pthread_exit(0);

    return(NULL);
}

//*****************************************************************************
//
//! Task implementing MQTT Server plus client bridge
//!
//! This function
//!    1. Initializes network driver and connects to the default AP
//!    2. Initializes the mqtt client ans server libraries and set up MQTT
//!       with the remote broker.
//!    3. set up the button events and their callbacks(for publishing)
//!    4. handles the callback signals
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
void * MqttClient(void *pvParameters)
{
    long lRetVal = -1;

    /*Initializing Client and Subscribing to the Broker.                     */
    if(gApConnectionState >= 0)
    {
        lRetVal = MqttClient_start();
        if(lRetVal == -1)
        {
            UART_PRINT("MQTT Client lib initialization failed\n\r");
            pthread_exit(0);
            return(NULL);
        }
    }

    unsigned int type = 777;
    unsigned int WifiVal = 777;

    unsigned int *typep = &type;
    unsigned int *WifiValp = &WifiVal;
    char * pubTopic = "/Team11/Brandon_pub";
    char * pubTopic2 = "topic/Brandon/Stats\0";
    char * pubTopic3 = "/Team11/Brandon_SPI_pub";
    char * pubTopicArm1 = "/Team11/Arm1_sub";
    char * pubTopicArm2 = "/Team11/Arm2_sub";
    char * pubData = "Recieved from rover";


    char dataArray3[] = "                                                                         ";
    unsigned int count = 0;
    unsigned int revCount = 0;

    unsigned int Last_Id = 0;
    unsigned int missed_MSG = 0;
    int spicount = 0;
    while(1)
    {

        dequeueCustomQ6(WifiValp, typep);
        char dataArray[] = "                                                                          ";
        char dataArray2[] = "                                                                         ";
        switch(type)
        {
        case 0:
            count++;
            sprintf(dataArray, "{\"Recipient\": \"Guans\", \"Sender\": \"Brandon\", \"MSGID\": %d, \"Body\": \"Hi From Brandon\"}", count);
            pubData = &dataArray;

            MQTTClient_publish(gMqttClient, (char*) pubTopic, strlen(
                                                              (char*)pubTopic),
                                                          (char*)pubData,
                                                          strlen((char*) pubData), MQTT_QOS_1 |
                                                          ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));
            break;
        case 1:
            sprintf(dataArray, "{\"Recipient\": \"SERVER\", \"Sender\": \"Brandon\", \"Pub_Attempts\": %d, \"Sub_Recv\": %d, \"Sub_Missed\": %d}", count, revCount, missed_MSG);
            pubData = &dataArray;
            MQTTClient_publish(gMqttClient, (char*) pubTopic2, strlen(
                                                  (char*)pubTopic2),
                                              (char*)pubData,
                                              strlen((char*) pubData), MQTT_QOS_1 |
                                              ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));
            break;
        case 2:
            UART_PRINT("\n\rOn-board Client Disconnected\n\r\r\n");
            gUiConnFlag = 0;
            break;
        case 3:
            gUiConnFlag = 0;
            pthread_exit(0);

            return(NULL);
        case 4:
            revCount++;
            unsigned int current_ID = WifiVal;
            if(current_ID != Last_Id + 1)
            {
                if(current_ID != 1)
                {
                    missed_MSG++;
                }
            }
            Last_Id = current_ID;
            break;
        case 5:
            if(spicount < 3)
            {
                spicount++;
            }
            else
            {
                spicount = 1;
            }
            count++;
            unsigned int spiVal = WifiVal;
            //sprintf(dataArray3, "{\"Recipient\": \"SERVER\", \"Sender\": \"Brandon\", \"MSGID\": %d, \"SPI %d\": %d}", count, spicount ,spiVal);
            sprintf(dataArray3, "%d," , spiVal);
            pubData = &dataArray3;

            MQTTClient_publish(gMqttClient, (char*) pubTopic3, strlen(
                                                              (char*)pubTopic3),
                                                          (char*)pubData,
                                                          strlen((char*) pubData), MQTT_QOS_1 |
                                                          ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));
            break;
        case 6:
            count++;
            sprintf(dataArray, "{\"Recipient\": \"ARM2\", \"Sender\": \"Brandon\", \"MSGID\": %d, \"Body\": \"unload\"}", count);
            pubData = &dataArray;

            MQTTClient_publish(gMqttClient, (char*) pubTopicArm2, strlen(
                                                              (char*)pubTopicArm2),
                                                          (char*)pubData,
                                                          strlen((char*) pubData), MQTT_QOS_1 |
                                                          ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));

            sprintf(dataArray2, "{\"Recipient\": \"Ultrasonic\", \"Sender\": \"Brandon\", \"MSGID\": %d, \"Body\": \"stop\"}", count);
            pubData = &dataArray2;

            MQTTClient_publish(gMqttClient, (char*) pubTopic, strlen(
                                                              (char*)pubTopic),
                                                          (char*)pubData,
                                                          strlen((char*) pubData), MQTT_QOS_1 |
                                                          ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));
            break;
        case 7:
            count++;
            sprintf(dataArray, "{\"Recipient\": \"ARM1\", \"Sender\": \"Brandon\", \"MSGID\": %d, \"Body\": \"load\"}", count);
            pubData = &dataArray;

            MQTTClient_publish(gMqttClient, (char*) pubTopicArm1, strlen(
                                                              (char*)pubTopicArm1),
                                                          (char*)pubData,
                                                          strlen((char*) pubData), MQTT_QOS_1 |
                                                          ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));
            sprintf(dataArray2, "{\"Recipient\": \"Ultrasonic\", \"Sender\": \"Brandon\", \"MSGID\": %d, \"Body\": \"stop\"}", count);
            pubData = &dataArray2;

            MQTTClient_publish(gMqttClient, (char*) pubTopic, strlen(
                                                              (char*)pubTopic),
                                                          (char*)pubData,
                                                          strlen((char*) pubData), MQTT_QOS_1 |
                                                          ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));
            break;
        case 8:
            count++;
            sprintf(dataArray, "{\"Recipient\": \"Ultrasonic\", \"Sender\": \"Brandon\", \"MSGID\": %d, \"Body\": \"Going Forward\"}", count);
            pubData = &dataArray;

            MQTTClient_publish(gMqttClient, (char*) pubTopic, strlen(
                                                              (char*)pubTopic),
                                                          (char*)pubData,
                                                          strlen((char*) pubData), MQTT_QOS_1 |
                                                          ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));
            break;
        case 9:
            count++;
            sprintf(dataArray, "{\"Recipient\": \"Ultrasonic\", \"Sender\": \"Brandon\", \"MSGID\": %d, \"Body\": \"Going Backwards\"}", count);
            pubData = &dataArray;

            MQTTClient_publish(gMqttClient, (char*) pubTopic, strlen(
                                                              (char*)pubTopic),
                                                          (char*)pubData,
                                                          strlen((char*) pubData), MQTT_QOS_1 |
                                                          ((RETAIN_ENABLE) ? MQTT_PUBLISH_RETAIN : 0));

            break;
        default:
            break;

        }
    }
}

//*****************************************************************************
//
//! This function connect the MQTT device to an AP with the SSID which was
//! configured in SSID_NAME definition which can be found in Network_if.h file,
//! if the device can't connect to to this AP a request from the user for other
//! SSID will appear.
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
int32_t Mqtt_IF_Connect()
{
    int32_t lRetVal;
    char SSID_Remote_Name[32];
    int8_t Str_Length;

    memset(SSID_Remote_Name, '\0', sizeof(SSID_Remote_Name));
    Str_Length = strlen(SSID_NAME);

    if(Str_Length)
    {
        /*Copy the Default SSID to the local variable                        */
        strncpy(SSID_Remote_Name, SSID_NAME, Str_Length);
    }

    /*Display Application Banner                                             */
    DisplayBanner(APPLICATION_NAME);

    GPIO_write(Board_LEDR, Board_GPIO_LED_OFF);
    GPIO_write(Board_LEDY, Board_GPIO_LED_OFF);
    GPIO_write(Board_LEDG, Board_GPIO_LED_OFF);

    /*Reset The state of the machine                                         */
    Network_IF_ResetMCUStateMachine();

    /*Start the driver                                                       */
    lRetVal = Network_IF_InitDriver(ROLE_STA);
    if(lRetVal < 0)
    {
        UART_PRINT("Failed to start SimpleLink Device\n\r", lRetVal);
        return(-1);
    }

    /*switch on Board_GPIO_LED2 to indicate Simplelink is properly up.       */
    GPIO_write(Board_LEDG, Board_GPIO_LED_ON);


    /*Initialize AP security params                                          */
    SecurityParams.Key = (signed char *) SECURITY_KEY;
    SecurityParams.KeyLen = strlen(SECURITY_KEY);
    SecurityParams.Type = SECURITY_TYPE;

    /*Connect to the Access Point                                            */
    lRetVal = Network_IF_ConnectAP(SSID_Remote_Name, SecurityParams);
    if(lRetVal < 0)
    {
        UART_PRINT("Connection to an AP failed\n\r");
        return(-1);
    }


    /*Switch ON Board_GPIO_LED0 to indicate that Device acquired an IP.      */
    GPIO_write(Board_LEDR, Board_GPIO_LED_ON);

    sleep(1);

    GPIO_write(Board_LEDR, Board_GPIO_LED_OFF);
    GPIO_write(Board_LEDY, Board_GPIO_LED_OFF);
    GPIO_write(Board_LEDG, Board_GPIO_LED_OFF);

    return(0);
}

//*****************************************************************************
//!
//! MQTT Start - Initialize and create all the items required to run the MQTT
//! protocol
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************
void Mqtt_start()
{
    int32_t threadArg = 100;
    pthread_attr_t pAttrs;
    struct sched_param priParam;
    int32_t retc = 0;

    //.......................................................................///
    mq_attr attr;
    unsigned mode = 0;

    /*sync object for inter thread communication                             */
    //......................................................................////

    /*Set priority and stack size attributes                                 */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 2;
    retc = pthread_attr_setschedparam(&pAttrs, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs, MQTTTHREADSIZE);
    retc |= pthread_attr_setdetachstate(&pAttrs, PTHREAD_CREATE_DETACHED);

    if(retc != 0)
    {
        return;
    }

    retc = pthread_create(&mqttThread, &pAttrs, MqttClient, (void *) &threadArg);
    if(retc != 0)
    {
        return;
    }
}

//*****************************************************************************
//!
//! MQTT Stop - Close the client instance and free all the items required to
//! run the MQTT protocol
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************

void Mqtt_Stop()
{
    if(gApConnectionState >= 0)
    {
        Mqtt_ClientStop(1);
    }

    unsigned int type = 3;
    unsigned int WifiVal = 0;
    if (sendWIFIMsgToQ6(WifiVal, type) == 0)
    {
    }

    sl_Stop(SL_STOP_TIMEOUT);
    UART_PRINT("\n\r Client Stop completed\r\n");
}

int32_t MqttClient_start()
{
    int32_t lRetVal = -1;
    int32_t iCount = 0;

    int32_t threadArg = 100;
    pthread_attr_t pAttrs;
    struct sched_param priParam;

    MqttClientExmple_params.clientId = ClientId;
    MqttClientExmple_params.connParams = &Mqtt_ClientCtx;
    MqttClientExmple_params.mqttMode31 = MQTT_3_1;
    MqttClientExmple_params.blockingSend = true;


    /*Initialize MQTT client lib                                             */
    gMqttClient = MQTTClient_create(MqttClientCallback,
                                    &MqttClientExmple_params);
    if(gMqttClient == NULL)
    {
        /*lib initialization failed                                          */
        return(-1);
    }

    /*Open Client Receive Thread start the receive task. Set priority and    */
    /*stack size attributes                                                  */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = 2;
    lRetVal = pthread_attr_setschedparam(&pAttrs, &priParam);
    lRetVal |= pthread_attr_setstacksize(&pAttrs, RXTASKSIZE);
    lRetVal |= pthread_attr_setdetachstate(&pAttrs, PTHREAD_CREATE_DETACHED);
    lRetVal |=
        pthread_create(&g_rx_task_hndl, &pAttrs, MqttClientThread,
                       (void *) &threadArg);
    if(lRetVal != 0)
    {
        UART_PRINT("Client Thread Create Failed failed\n\r");
        return(-1);
    }
#ifdef SECURE_CLIENT
    setTime();
#endif

    /*setting will parameters                                                */
    MQTTClient_set(gMqttClient, MQTTClient_WILL_PARAM, &will_param,
                   sizeof(will_param));

#ifdef CLNT_USR_PWD
    /*Set user name for client connection                                    */
    MQTTClient_set(gMqttClient, MQTTClient_USER_NAME, (void *)ClientUsername,
                   strlen(
                       (char*)ClientUsername));

    /*Set password                                                           */
    MQTTClient_set(gMqttClient, MQTTClient_PASSWORD, (void *)ClientPassword,
                   strlen(
                       (char*)ClientPassword));
#endif
    /*Initiate MQTT Connect                                                  */
    if(gApConnectionState >= 0)
    {
#if CLEAN_SESSION == false
        bool clean = CLEAN_SESSION;
        MQTTClient_set(gMqttClient, MQTTClient_CLEAN_CONNECT, (void *)&clean,
                       sizeof(bool));
#endif
        /*The return code of MQTTClient_connect is the ConnACK value that
           returns from the server */
        lRetVal = MQTTClient_connect(gMqttClient);

        /*negative lRetVal means error,
           0 means connection successful without session stored by the server,
           greater than 0 means successful connection with session stored by
           the server */
        if(0 > lRetVal)
        {
            /*lib initialization failed                                      */
            UART_PRINT("Connection to broker failed, Error code: %d\n\r",
                       lRetVal);

            gUiConnFlag = 0;
        }
        else
        {
            gUiConnFlag = 1;
        }
        /*Subscribe to topics when session is not stored by the server       */
        if((gUiConnFlag == 1) && (0 == lRetVal))
        {
            uint8_t subIndex;
            MQTTClient_SubscribeParams subscriptionInfo[
                SUBSCRIPTION_TOPIC_COUNT];

            for(subIndex = 0; subIndex < SUBSCRIPTION_TOPIC_COUNT; subIndex++)
            {
                subscriptionInfo[subIndex].topic = topic[subIndex];
                subscriptionInfo[subIndex].qos = qos[subIndex];
            }

            if(MQTTClient_subscribe(gMqttClient, subscriptionInfo,
                                    SUBSCRIPTION_TOPIC_COUNT) < 0)
            {
                UART_PRINT("\n\r Subscription Error \n\r");
                MQTTClient_disconnect(gMqttClient);
                gUiConnFlag = 0;
            }
            else
            {
                for(iCount = 0; iCount < SUBSCRIPTION_TOPIC_COUNT; iCount++)
                {
                    UART_PRINT("Client subscribed on %s\n\r,", topic[iCount]);
                }
            }
        }
    }

    return(0);
}

//*****************************************************************************
//!
//! MQTT Client stop - Unsubscribe from the subscription topics and exit the
//! MQTT client lib.
//!
//! \param  none
//!
//! \return None
//!
//*****************************************************************************

void Mqtt_ClientStop(uint8_t disconnect)
{
    uint32_t iCount;

    MQTTClient_UnsubscribeParams subscriptionInfo[SUBSCRIPTION_TOPIC_COUNT];

    for(iCount = 0; iCount < SUBSCRIPTION_TOPIC_COUNT; iCount++)
    {
        subscriptionInfo[iCount].topic = topic[iCount];
    }

    MQTTClient_unsubscribe(gMqttClient, subscriptionInfo,
                           SUBSCRIPTION_TOPIC_COUNT);
    for(iCount = 0; iCount < SUBSCRIPTION_TOPIC_COUNT; iCount++)
    {
        UART_PRINT("Unsubscribed from the topic %s\r\n", topic[iCount]);
    }
    gUiConnFlag = 0;

    /*exiting the Client library                                             */
    MQTTClient_delete(gMqttClient);
}

//*****************************************************************************
//!
//! Utility function which prints the borders
//!
//! \param[in] ch  -  hold the charater for the border.
//! \param[in] n   -  hold the size of the border.
//!
//! \return none.
//!
//*****************************************************************************

void printBorder(char ch,
                 int n)
{
    int i = 0;

    for(i = 0; i < n; i++)
    {
        putch(ch);
    }
}

//*****************************************************************************
//!
//! Set the ClientId with its own mac address
//! This routine converts the mac address which is given
//! by an integer type variable in hexadecimal base to ASCII
//! representation, and copies it into the ClientId parameter.
//!
//! \param  macAddress  -   Points to string Hex.
//!
//! \return void.
//!
//*****************************************************************************
int32_t SetClientIdNamefromMacAddress()
{
    int32_t ret = 0;
    uint8_t Client_Mac_Name[2];
    uint8_t Index;
    uint16_t macAddressLen = SL_MAC_ADDR_LEN;
    uint8_t macAddress[SL_MAC_ADDR_LEN];

    /*Get the device Mac address */
    //ret = sl_NetCfgGet(SL_NETCFG_MAC_ADDRESS_GET, 0, &macAddressLen,
    //                   &macAddress[0]);

    /*When ClientID isn't set, use the mac address as ClientID               */
    if(ClientId[0] == '\0')
    {
        /*6 bytes is the length of the mac address                           */
        for(Index = 0; Index < SL_MAC_ADDR_LEN; Index++)
        {
            /*Each mac address byte contains two hexadecimal characters      */
            /*Copy the 4 MSB - the most significant character                */
            Client_Mac_Name[0] = (macAddress[Index] >> 4) & 0xf;
            /*Copy the 4 LSB - the least significant character               */
            Client_Mac_Name[1] = macAddress[Index] & 0xf;

            if(Client_Mac_Name[0] > 9)
            {
                /*Converts and copies from number that is greater than 9 in  */
                /*hexadecimal representation (a to f) into ascii character   */
                ClientId[2 * Index] = Client_Mac_Name[0] + 'a' - 10;
            }
            else
            {
                /*Converts and copies from number 0 - 9 in hexadecimal       */
                /*representation into ascii character                        */
                ClientId[2 * Index] = Client_Mac_Name[0] + '0';
            }
            if(Client_Mac_Name[1] > 9)
            {
                /*Converts and copies from number that is greater than 9 in  */
                /*hexadecimal representation (a to f) into ascii character   */
                ClientId[2 * Index + 1] = Client_Mac_Name[1] + 'a' - 10;
            }
            else
            {
                /*Converts and copies from number 0 - 9 in hexadecimal       */
                /*representation into ascii character                        */
                ClientId[2 * Index + 1] = Client_Mac_Name[1] + '0';
            }
        }
    }
    return(ret);
}

//*****************************************************************************
//!
//! Utility function which Display the app banner
//!
//! \param[in] appName     -  holds the application name.
//! \param[in] appVersion  -  holds the application version.
//!
//! \return none.
//!
//*****************************************************************************

int32_t DisplayAppBanner(char* appName,
                         char* appVersion)
{
    int32_t ret = 0;
    uint8_t macAddress[SL_MAC_ADDR_LEN];
    uint16_t macAddressLen = SL_MAC_ADDR_LEN;
    uint16_t ConfigSize = 0;
    uint8_t ConfigOpt = SL_DEVICE_GENERAL_VERSION;
    SlDeviceVersion_t ver = {0};

    ConfigSize = sizeof(SlDeviceVersion_t);

    /*Print device version info. */
    ret =
        sl_DeviceGet(SL_DEVICE_GENERAL, &ConfigOpt, &ConfigSize,
                     (uint8_t*)(&ver));

    /*Print device Mac address */
    ret |= (int32_t)sl_NetCfgGet(SL_NETCFG_MAC_ADDRESS_GET, 0, &macAddressLen,
                       &macAddress[0]);

    UART_PRINT(lineBreak);
    UART_PRINT("\t");
    printBorder('=', 44);
    UART_PRINT(lineBreak);
    UART_PRINT("\t   %s Example Ver: %s",appName, appVersion);
    UART_PRINT(lineBreak);
    UART_PRINT("\t");
    printBorder('=', 44);
    UART_PRINT(lineBreak);
    UART_PRINT(lineBreak);
    UART_PRINT("\t CHIP: 0x%x",ver.ChipId);
    UART_PRINT(lineBreak);
    UART_PRINT("\t MAC:  %d.%d.%d.%d",ver.FwVersion[0],ver.FwVersion[1],
               ver.FwVersion[2],
               ver.FwVersion[3]);
    UART_PRINT(lineBreak);
    UART_PRINT("\t PHY:  %d.%d.%d.%d",ver.PhyVersion[0],ver.PhyVersion[1],
               ver.PhyVersion[2],
               ver.PhyVersion[3]);
    UART_PRINT(lineBreak);
    UART_PRINT("\t NWP:  %d.%d.%d.%d",ver.NwpVersion[0],ver.NwpVersion[1],
               ver.NwpVersion[2],
               ver.NwpVersion[3]);
    UART_PRINT(lineBreak);
    UART_PRINT("\t ROM:  %d",ver.RomVersion);
    UART_PRINT(lineBreak);
    UART_PRINT("\t HOST: %s", SL_DRIVER_VERSION);
    UART_PRINT(lineBreak);
    UART_PRINT("\t MAC address: %02x:%02x:%02x:%02x:%02x:%02x", macAddress[0],
               macAddress[1], macAddress[2], macAddress[3], macAddress[4],
               macAddress[5]);
    UART_PRINT(lineBreak);
    UART_PRINT(lineBreak);
    UART_PRINT("\t");
    printBorder('=', 44);
    UART_PRINT(lineBreak);
    UART_PRINT(lineBreak);

    return(ret);
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************



















/*
 *  ======== threadFxn0 ========
 *  reads in from spi and does the PID calculations
 */
void *threadFxn0(void *arg0)
{
    unsigned int spiVal = 0;
    unsigned int timeVal = 0;
    unsigned int wifiVal = 0;
    unsigned int lastWifiVal = MACRO_STOP;
    unsigned int type = DEFAULT_UINT;

    unsigned int *spiValp = &spiVal;
    unsigned int *timeValp = &timeVal;
    unsigned int *wifiValp = &wifiVal;
    unsigned int *typep = &type;

    int lastSPI1 = 0;
    int speed1 = 0;
    int currentSPI1 = 0;

    int lastSPI2 = 0;
    int speed2 = 0;
    int currentSPI2 = 0;

    int lastSPI3 = 0;
    int speed3 = 0;
    int currentSPI3 = 0;

    int counter = 1;
    int errorLast1 = 0;
    int errorLast2 = 0;
    int errorLast3 = 0;


    uint32_t motorMSG = 0;
    static uint8_t speedM1 = 127;
    static uint8_t speedM2 = 127;
    static uint8_t speedM3 = 127;

    static uint8_t speedA1 = 127;
    int speedA1C = 127;
    static uint8_t speedA2 = 127;
    int speedA2C = 127;
    static uint8_t speedA3 = 127;
    int speedA3C = 127;

    double kp = 0.07;
    double ki = 0.005;
    double kd = 0.05;

    //double ki = 0.2;
    //double kp = 0.05;
    //double kd = 0.01;


    double integral1 = 0;
    double derivative1 = 0;

    double integral2 = 0;
    double derivative2 = 0;

    double integral3 = 0;
    double derivative3 = 0;

    int spicounter = 1;

    int enable_counter = 0;

    int stop_counter = 0;

    while(1)
    {
        dequeueCustomQ1(spiValp, timeValp, wifiValp, typep);
        //set up spi here
        if(type == Q1_TYPE_SPI)
        {
            if(lastWifiVal != MACRO_STOP)
            {
                if(counter == 1)
                            {
                                speedA1C = speedA1;
                                currentSPI1 = spiVal;
                                speed1 = currentSPI1 - lastSPI1;
                                lastSPI1 = currentSPI1;
                                int targetspeed = motortospeedconv(speedM1);
                                int error1 = 0;
                                if(enable_counter == 0)
                                {
                                    error1 = targetspeed - speed1;
                                }
                                //sendWIFIMsgToQ6(error1, 5);

                                integral1 = integral1 + error1;

                                derivative1 = error1 - errorLast1;

                                int controlVar1 = (kp * error1) + (ki * integral1) + (kd * derivative1);
                                speedA1C += controlVar1;
                                if(speedA1C > 255)
                                {
                                    speedA1 = 255;
                                }
                                else if(speedA1C < 0)
                                {
                                    speedA1 = 0;
                                }
                                else
                                {
                                    speedA1 = speedA1C;
                                }
                                errorLast1 = error1;
                                //sendWIFIMsgToQ6(speedA1, 5);
                            }
                            else if(counter == 2)
                            {
                                speedA2C = speedA2;
                                currentSPI2 = spiVal;
                                speed2 = currentSPI2 - lastSPI2;
                                sendWIFIMsgToQ6(speed2, 5);
                                lastSPI2 = currentSPI2;
                                int targetspeed = motortospeedconv(speedM2);

                                int error2 = 0;
                                if(enable_counter == 0)
                                {
                                    error2 = targetspeed - speed2;
                                }
                                //sendWIFIMsgToQ6(error2, 5);
                                integral2 = integral2 + error2;

                                derivative2 = error2 - errorLast2;

                                int controlVar2 = (kp * error2) + (ki * integral2) + (kd * derivative2);
                                //sendWIFIMsgToQ6(controlVar2, 5);
                                speedA2C += controlVar2;
                                if(speedA2C > 255)
                                {
                                    speedA2 = 255;
                                }
                                else if(speedA2C < 0)
                                {
                                    speedA2 = 0;
                                }
                                else
                                {
                                    speedA2 = speedA2C;
                                }
                                errorLast2 = error2;
                                //sendWIFIMsgToQ6(speedA2, 5);
                            }
                            else if(counter == 3)
                            {
                                speedA3C = speedA3;
                                currentSPI3 = spiVal;
                                speed3 = currentSPI3 - lastSPI3;
                                //sendWIFIMsgToQ6(speed3, 5);
                                lastSPI3 = currentSPI3;
                                int targetspeed = motortospeedconv(speedM3);

                                int error3 = 0;
                                if(enable_counter == 0)
                                {
                                    error3 = targetspeed - speed3;
                                }
                                //sendWIFIMsgToQ6(error3, 5);
                                integral3 = integral3 + error3;

                                derivative3 = error3 - errorLast3;

                                int controlVar3 = (kp * error3) + (ki * integral3) + (kd * derivative3);
                                speedA3C += controlVar3;
                                if(speedA3C > 255)
                                {
                                    speedA3 = 255;
                                }
                                else if(speedA3C < 0)
                                {
                                    speedA3 = 0;
                                }
                                else
                                {
                                    speedA3 = speedA3C;
                                }
                                errorLast3 = error3;
                                //sendWIFIMsgToQ6(speedA3, 5);
                            }
                            if(enable_counter > 0)
                            {
                                motorMSG = 0;
                                motorMSG = motorMSG + speedA1;
                                motorMSG = (motorMSG << 8) + speedA2;
                                motorMSG = (motorMSG << 8) + speedA3;
                                sendMotorMsgToQ2(motorMSG);
                                    enable_counter--;
                            }
            }


            if(counter == 3)
            {
                counter = 1;
            }
            else
            {
                counter++;
            }

        }
        else if(type == Q1_TYPE_WIFI )
        {
            lastWifiVal = wifiVal;
            if(wifiVal == MACRO_FOREWARD)
            {
                speedM1 = 127;
                speedM2 = 67;
                speedM3 = 190;
                enable_counter = 3;
                speedA1 = 127;
                speedA2 = 67;
                speedA3 = 187;
                sendWIFIMsgToQ6(0, 8);
                stop_counter = 0;
            }
            else if(wifiVal == MACRO_STOP)
            {
                speedM1 = 127;
                speedM2 = 127;
                speedM3 = 127;
                speedA1 = speedM1;
                speedA2 = speedM2;
                speedA3 = speedM3;

                integral1 = 0;
                derivative1 = 0;
                errorLast1 = 0;

                integral2 = 0;
                derivative2 = 0;
                errorLast2 = 0;

                integral3 = 0;
                derivative3 = 0;
                errorLast3 = 0;
                if(stop_counter == 0)
                {
                    sendWIFIMsgToQ6(0, 6);
                    stop_counter = 777;
                }
                else if(stop_counter == 1)
                {
                    sendWIFIMsgToQ6(0, 7);
                    stop_counter = 666;
                }
            }
            else if(wifiVal == MACRO_BACK)
            {
                speedM1 = 127;
                speedM2 = 187;
                speedM3 = 67;
                enable_counter = 3;
                speedA1 = speedM1;
                speedA2 = speedM2;
                speedA3 = speedM3;
                sendWIFIMsgToQ6(0, 9);
                stop_counter = 1;
            }
            else if(wifiVal == MACRO_SPIN_LEFT)
            {
                speedM1 = 187;
                speedM2 = 187;
                speedM3 = 187;
                speedA1 = speedM1;
                speedA2 = speedM2;
                speedA3 = speedM3;
            }
            else if(wifiVal == MACRO_SPIN_RIGHT)
            {
                speedM1 = 67;
                speedM2 = 67;
                speedM3 = 67;
                speedA1 = speedM1;
                speedA2 = speedM2;
                speedA3 = speedM3;
            }

            motorMSG = 0;
            motorMSG = motorMSG + speedM1;
            motorMSG = (motorMSG << 8) + speedM2;
            motorMSG = (motorMSG << 8) + speedM3;
            sendMotorMsgToQ2(motorMSG);

        }
        else if(type == 1)
        {
            spiOP(spicounter);
            if(spicounter == 3)
            {
                spicounter = 1;
            }
            else
            {
                spicounter++;
            }
        }
    }
    return (NULL);
}

/*
 *  ======== threadFxn1 ========
 *  takes in the motor control messages and writes to the UART
 */
void *threadFxn1(void *arg0)
{
    unsigned int motorVal = 0;
    unsigned int type = 0;
    unsigned int *motorValp = &motorVal;
    unsigned int *typep = &type;
    while(1)
    {
        dequeueCustomQ2(motorValp, typep);
        uint8_t speed3 = motorVal;
        uint8_t speed2 = (motorVal >> 8);
        uint8_t speed1 = (motorVal >> 16);
        allMotorOperations(speed1, speed2, speed3);
    }
    return (NULL);
}

/*
 *  ======== threadFxn2 ========
 *  Does the Wifi Decryption for communication with the server
 */
void *threadFxn2(void *arg0)
{


    unsigned int jsonVal = 0;

    int *jsonValp = &jsonVal;

    GPIO_init();
    unsigned int type = 0;
    unsigned int *typep = &type;

    int spiOut = SPI_W_DISABLE;

    initSpi();
    Uartinit0();
    timer_spi_init_custom();

    while(1)
    {
        dequeueCustomQ3(jsonValp, typep);

        if(type == 0)
        {
            //initSpi();
            //Uartinit0();
            //timer_spi_init_custom();
        }
        else if(type == 1)
        {
            //send message to spi thread
            spiOP(1);
        }
        else if(type == 2)
        {
            spiOP(2);
        }
        else if(type == 3)
        {
            //send message to spi thread
            spiOP(3);
        }
        else if(type == 4)
        {
            uint32_t wifiVal = MACRO_FOREWARD;
            sendWIFIMsgToQ1(wifiVal);
        }
        else if(type == 5)
        {
            uint32_t wifiVal = MACRO_STOP;
            sendWIFIMsgToQ1(wifiVal);
        }
        else if(type == 6)
        {
            uint32_t wifiVal = MACRO_BACK;
            sendWIFIMsgToQ1(wifiVal);
        }
        else if(type == 7)
        {
            uint32_t wifiVal = MACRO_SPIN_LEFT;
            sendWIFIMsgToQ1(wifiVal);
        }
        else if(type == 8)
        {
            uint32_t wifiVal = MACRO_SPIN_RIGHT;
            sendWIFIMsgToQ1(wifiVal);
        }
        else if(type == 9)
        {
            errorRoutine(ERROR_TEST);
        }
        else if(type == 10)
        {
            //send message to motor thread
            uint32_t motorMSG = jsonVal;
            sendMotorMsgToQ2(motorMSG);
        }
    }
    return (NULL);
}


/*
 *  ======== threadFxn3 ========
 *  Does the Wifi writing
 */
void *threadFxn3(void *arg0)
{
    unsigned int wifiVal = 0;
    unsigned int type = 0;

    unsigned int *wifiValp = &wifiVal;
    unsigned int *typep = &type;
    while(1)
    {
        dequeueCustomQ4(wifiValp, typep);
    }
    return (NULL);
}


/*
 *  ======== threadFxn4 ========
 *  Test thread that writes to other threads
 */
/*
void *threadFxn4(void *arg0)
{
    GPIO_init();

    char inputTotal[9];
    char out[12] = "Enter Input:";

    unsigned int spiVal = 0;
    unsigned int spiVal1 = DEFAULT_UINT;
    unsigned int spiVal2 = DEFAULT_UINT;
    unsigned int spiVal3 = DEFAULT_UINT;
    unsigned int spiVal1Last = DEFAULT_UINT;
    unsigned int spiVal2Last = DEFAULT_UINT;
    unsigned int spiVal3Last = DEFAULT_UINT;
    unsigned int type = 0;

    unsigned int *spiValp = &spiVal;
    unsigned int *typep = &type;

    GPIO_write(Board_LEDG, Board_GPIO_LED_OFF);
    GPIO_write(Board_LEDY, Board_GPIO_LED_OFF);

    int trigger = TRIGGER_LEDNONE;

    int spiOut = SPI_W_DISABLE;

    while(1)
    {
        if(spiOut == SPI_W_ENABLE)
        {
            if(trigger == TRIGGER_LEDG)
            {
                GPIO_toggle(Board_LEDG);
            }
            else if(trigger == TRIGGER_LEDY)
            {
                GPIO_toggle(Board_LEDY);
            }
            else if(trigger == TRIGGER_LEDGY)
            {
                GPIO_toggle(Board_LEDY);
                GPIO_toggle(Board_LEDG);
            }
            spiOut = SPI_W_DISABLE;
        }
        UART_write(UART1, &out, sizeof(out));

        UART_read(UART1, inputTotal, sizeof(inputTotal));
        UART_write(UART1, &inputTotal, sizeof(inputTotal));
        UART_write(UART1, "\n\r", 2);
        if(inputTotal[0] == 'i')
        {
            initSpi();
            Uartinit0();
        }
        else if(inputTotal[0] == 's')
        {
            //send message to spi thread
            spiOP(1);
            dequeueCustomQ5(spiValp, typep);
            spiVal1 = spiVal;
            if(spiVal1 != SPI_R_FAILURE)
            {
                if(spiVal1 == spiVal1Last)
                {
                    trigger = TRIGGER_LEDG;
                    spiOut = SPI_W_ENABLE;
                }
                else
                {
                    spiVal1Last = spiVal1;
                }
            }
        }
        else if(inputTotal[0] == 'd')
        {
            spiOP(2);
            dequeueCustomQ5(spiValp, typep);
            spiVal2 = spiVal;
            if(spiVal2 != SPI_R_FAILURE)
            {
                if(spiVal2 == spiVal2Last)
                {
                    trigger = TRIGGER_LEDY;
                    spiOut = SPI_W_ENABLE;
                }
                else
                {
                    spiVal2Last = spiVal2;
                }
            }
        }
        else if(inputTotal[0] == 'f')
        {
            //send message to spi thread
            spiOP(3);
            dequeueCustomQ5(spiValp, typep);
            spiVal3 = spiVal;
            if(spiVal3 != SPI_R_FAILURE)
            {
                if(spiVal3 == spiVal3Last)
                {
                    trigger = TRIGGER_LEDGY;
                    spiOut = SPI_W_ENABLE;
                }
                else
                {
                    spiVal3Last = spiVal3;
                }
            }
        }
        else if(inputTotal[0] == 'z')
        {
            uint32_t wifiVal = MACRO_FOREWARD;
            sendWIFIMsgToQ1(wifiVal);
        }
        else if(inputTotal[0] == 'x')
        {
            uint32_t wifiVal = MACRO_STOP;
            sendWIFIMsgToQ1(wifiVal);
        }
        else if(inputTotal[0] == 'c')
        {
            uint32_t wifiVal = MACRO_BACK;
            sendWIFIMsgToQ1(wifiVal);
        }
        else if(inputTotal[0] == 'v')
        {
            uint32_t wifiVal = MACRO_SPIN_LEFT;
            sendWIFIMsgToQ1(wifiVal);
        }
        else if(inputTotal[0] == 'b')
        {
            uint32_t wifiVal = MACRO_SPIN_RIGHT;
            sendWIFIMsgToQ1(wifiVal);
        }
        else if(inputTotal[0] == 'e')
        {
            errorRoutine(ERROR_TEST);
        }
        else
        {
            //send message to motor thread
            uint32_t motorMSG = 0;
            char input1 [4];
            char input2 [4];
            char input3 [4];
            uint8_t speed1;
            uint8_t speed2;
            uint8_t speed3;

            input1[0] = inputTotal[0];
            input1[1] = inputTotal[1];
            input1[2] = inputTotal[2];
            input1[3] = '\0';

            input2[0] = inputTotal[3];
            input2[1] = inputTotal[4];
            input2[2] = inputTotal[5];
            input2[3] = '\0';

            input3[0] = inputTotal[6];
            input3[1] = inputTotal[7];
            input3[2] = inputTotal[8];
            input3[3] = '\0';

            speed1 = atoi(input1);
            speed2 = atoi(input2);
            speed3 = atoi(input3);
            motorMSG = motorMSG + speed1;
            motorMSG = (motorMSG << 8) + speed2;
            motorMSG = (motorMSG << 8) + speed3;
            sendMotorMsgToQ2(motorMSG);
        }
    }
    return (NULL);
}
*/

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    pthread_t           thread0, thread1, thread2, thread3, thread4;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 detachState;
    uint32_t count = 0;
    pthread_t spawn_thread = (pthread_t) NULL;
    pthread_attr_t pAttrs_spawn;
    int32_t retc = 0;
    UART_Handle tUartHndl;

    GPIO_init();
    GPIO_write(Board_GPIO0, 0);
    GPIO_write(Board_GPIO1, 0);
    GPIO_write(Board_GPIO2, 0);
    GPIO_write(Board_GPIO3, 0);
    GPIO_write(Board_GPIO4, 0);
    GPIO_write(Board_GPIO5, 0);
    GPIO_write(Board_GPIO6, 0);
    GPIO_write(Board_GPIO7, 0);
    sensor_queue_init();



    /* Create application threads */
    //pthread_attr_init(&attrs);

    //detachState = PTHREAD_CREATE_DETACHED;
    /* Set priority and stack size attributes */
    //retc = pthread_attr_setdetachstate(&attrs, detachState);
    //if (retc != 0) {
        /* pthread_attr_setdetachstate() failed */
       // while (1);
    //}

    //retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    //if (retc != 0) {
        /* pthread_attr_setstacksize() failed */
       // while (1);
    //}

    priParam.sched_priority = 1;
    pthread_attr_setschedparam(&attrs, &priParam);


            /*Initialize SlNetSock layer with CC31xx/CC32xx interface */
            SlNetIf_init(0);
            SlNetIf_add(SLNETIF_ID_1, "CC32xx",
                        (const SlNetIf_Config_t *)&SlNetIfConfigWifi,
                        SLNET_IF_WIFI_PRIO);

            SlNetSock_init(0);
            SlNetUtil_init(0);

            GPIO_init();
            SPI_init();

            /*Configure the UART                                                     */
            tUartHndl = InitTerm();
            /*remove uart receive from LPDS dependency                               */
            UART_control(tUartHndl, UART_CMD_RXDISABLE, NULL);

            /*Create the sl_Task                                                     */
            pthread_attr_init(&pAttrs_spawn);
            priParam.sched_priority = SPAWN_TASK_PRIORITY;
            retc = pthread_attr_setschedparam(&pAttrs_spawn, &priParam);
            retc |= pthread_attr_setstacksize(&pAttrs_spawn, TASKSTACKSIZE);
            retc |= pthread_attr_setdetachstate
                                            (&pAttrs_spawn, PTHREAD_CREATE_DETACHED);

            retc = pthread_create(&spawn_thread, &pAttrs_spawn, sl_Task, NULL);

            if(retc != 0)
            {
                UART_PRINT("could not create simplelink task\n\r");
                //throw error
            }

            retc = sl_Start(0, 0, 0);
            if(retc < 0)
            {
                /*Handle Error */
                UART_PRINT("\n sl_Start failed\n");
                //throw error
            }

            /*Output device information to the UART terminal */
            retc = DisplayAppBanner(APPLICATION_NAME, APPLICATION_VERSION);
            /*Set the ClientId with its own mac address */
            retc |= SetClientIdNamefromMacAddress();


            retc = sl_Stop(SL_STOP_TIMEOUT);
            if(retc < 0)
            {
                /*Handle Error */
                UART_PRINT("\n sl_Stop failed\n");
                //throw error
            }

            if(retc < 0)
            {
                /*Handle Error */
                UART_PRINT("mqtt_client - Unable to retrieve device information \n");
                //throw error
            }
            //timer_pub_init_custom();
            timer_stat_init_custom();


            topic[0] = SUBSCRIPTION_TOPIC0;
            topic[1] = SUBSCRIPTION_TOPIC1;
            topic[2] = SUBSCRIPTION_TOPIC2;
            topic[3] = SUBSCRIPTION_TOPIC3;

            //Connect to AP
            gApConnectionState = Mqtt_IF_Connect();

            //Run MQTT Main Thread (it will open the Client and Server)
            Mqtt_start();

            //while(1)
            //{

            //}


    pthread_attr_init(&attrs);

    detachState = PTHREAD_CREATE_DETACHED;
    /* Set priority and stack size attributes */
    retc = pthread_attr_setdetachstate(&attrs, detachState);
    if (retc != 0) {
        /* pthread_attr_setdetachstate() failed */
        while (1);
    }

    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0) {
        /* pthread_attr_setstacksize() failed */
        while (1);
    }

    priParam.sched_priority = 1;
    pthread_attr_setschedparam(&attrs, &priParam);
    retc = pthread_create(&thread0, &attrs, threadFxn0, NULL);
    if (retc != 0) {
        while (1);
    }

    retc = pthread_create(&thread1, &attrs, threadFxn1, (void* )0);
    if (retc != 0) {
        while (1);
    }
    retc = pthread_create(&thread2, &attrs, threadFxn2, (void* )0);
    if (retc != 0) {
       while (1);
    }
    /*
    retc = pthread_create(&thread3, &attrs, threadFxn3, (void* )0);
    if (retc != 0) {
       while (1);
    }
    */

    /* Create threadFxn4 thread */
    /*
    retc = pthread_create(&thread4, &attrs, threadFxn4, (void* )0);
    if (retc != 0) {
       while (1);
    }
    */

    while(1)
    {

    }

    return (NULL);
}
