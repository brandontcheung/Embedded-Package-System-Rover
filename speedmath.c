/*
 * speedmath.c
 *
 *  Created on: Nov 10, 2019
 *      Author: Brandon
 */


int speedtomotorconv(int speed)
{
    int motorVal = 0;
    float motorValf = 0.0;
    motorValf = ((0.2518)*(speed) + 4.1126);
    motorVal = (int) motorValf;

    return motorVal;
}


int motortospeedconv(int motor)
{
    int speedVal = 0;
    if(motor == 127)
    {
        speedVal = 0;
    }
    else if(motor == 187)
    {
        speedVal = 348;
    }
    else if(motor == 67)
    {
        speedVal = -348;
    }


    return speedVal;
}
