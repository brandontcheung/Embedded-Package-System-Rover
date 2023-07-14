/*
 * timer_stats.c
 *
 *  Created on: Oct 22, 2019
 *      Author: Brandon
 */

#include "timer_stats.h"
//#include "debug.h"

void timerStatsCallback(Timer_Handle myHandle);

void timer_stat_init_custom()
{

    Timer_Params params;

    Timer_init();

    Timer_Params_init(&params);
       //this is what we have to change
       params.period = timerSPeriod;
       params.periodUnits = Timer_PERIOD_US;
       params.timerMode = Timer_CONTINUOUS_CALLBACK;
       params.timerCallback = timerStatsCallback;

       timerS = Timer_open(Board_TIMER1, &params);

       if (timerS == NULL) {
               /* Failed to initialized timer */
           //errorRoutine(Error_TIMER_ONE_INI);
           }

       if (Timer_start(timerS) == Timer_STATUS_ERROR) {
               /* Failed to start timer */
           //errorRoutine(Error_TIMER_ONE_START);
           }
}

/*
 * This callback is called every 250,000 microseconds, or 250 milliseconds. Because
*/
void timerStatsCallback(Timer_Handle myHandle)
{
    unsigned int type = 1;
    unsigned int WifiVal = 0;
    if (sendWIFIMsgToQ6(WifiVal, type) == 0)
    {
        //errorRoutine(Error_QUEUE_TIME_SEND);
    }
}



