/*
 * timer_spi.c
 *
 *  Created on: Nov 6, 2019
 *      Author: Brandon
 */
#include "timer_spi.h"
//#include "debug.h"

void timerSPICallback(Timer_Handle myHandle);

void timer_spi_init_custom()
{

    Timer_Params params;

    Timer_init();

    Timer_Params_init(&params);
       //this is what we have to change
       params.period = timerSPIPeriod;
       params.periodUnits = Timer_PERIOD_US;
       params.timerMode = Timer_CONTINUOUS_CALLBACK;
       params.timerCallback = timerSPICallback;

       timerSPI = Timer_open(Board_TIMER3, &params);

       if (timerSPI == NULL) {
               /* Failed to initialized timer */
           //errorRoutine(Error_TIMER_ONE_INI);
           }

       if (Timer_start(timerSPI) == Timer_STATUS_ERROR) {
               /* Failed to start timer */
           //errorRoutine(Error_TIMER_ONE_START);
           }
}

/*
 * This callback is called every 250,000 microseconds, or 250 milliseconds. Because
*/
void timerSPICallback(Timer_Handle myHandle)
{
    sendTimeMsgToQ1(100);
}




