/*
 * timer_pub.c
 *
 *  Created on: Oct 21, 2019
 *      Author: Brandon
 */

#include "timer_pub.h"
//#include "debug.h"

void timerQuarterCallback(Timer_Handle myHandle);

void timer_pub_init_custom()
{

    Timer_Params params;

    Timer_init();

    Timer_Params_init(&params);
       //this is what we have to change
       params.period = timerQPeriod;
       params.periodUnits = Timer_PERIOD_US;
       params.timerMode = Timer_CONTINUOUS_CALLBACK;
       params.timerCallback = timerQuarterCallback;

       timerQ = Timer_open(Board_TIMER0, &params);

       if (timerQ == NULL) {
               /* Failed to initialized timer */
           //errorRoutine(Error_TIMER_ONE_INI);
           }

       if (Timer_start(timerQ) == Timer_STATUS_ERROR) {
               /* Failed to start timer */
           //errorRoutine(Error_TIMER_ONE_START);
           }
}

/*
 * This callback is called every 250,000 microseconds, or 250 milliseconds. Because
*/
void timerQuarterCallback(Timer_Handle myHandle)
{
    //dbgOutputLoc(OLOC_INTIMERQUARTERISR);
    //dbgOutputLoc(OLOC_BEFOREQUEUEINTIMERQUARTERISR);
    //uint32_t timevl= Timer_getCount(myHandle);
    unsigned int type = 0;
    unsigned int WifiVal = 0;
    if (sendWIFIMsgToQ6(WifiVal, type) == 0)
    {
        //errorRoutine(Error_QUEUE_TIME_SEND);
    }
    //dbgOutputLoc(OLOC_AFTERQUEUEINTIMERQUARTERISR);
    //dbgOutputLoc(OLOC_LEAVINGTIMERQUARTERISR);
}


