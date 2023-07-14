/*
 * motor_uart.c
 *
 *  Created on: Oct 8, 2019
 *      Author: Brandon
 */

#include "motor_uart.h"

void Uartinit0()
{
    UART_Params uartParams;

    /* Call driver init functions */
    UART_init();


    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 9600;

    UART0 = UART_open(Board_UART0, &uartParams);

    if (UART0 == NULL) {
        /* UART_open() failed */
        while (1);
    }
    uint8_t baudrate = 0xAA;
    UART_write(UART0, &baudrate, sizeof(baudrate));
}

void motorCommand(uint8_t address, uint8_t command, uint8_t speed)
{
    uint8_t checksum = ((address + command + speed) & 0x7F);
    UART_write(UART0, &address, sizeof(address));
    UART_write(UART0, &command, sizeof(command));
    UART_write(UART0, &speed, sizeof(speed));
    UART_write(UART0, &checksum, sizeof(checksum));
}

void allMotorOperations(uint8_t speed1, uint8_t speed2, uint8_t speed3)
{
    uint8_t address1 = 128;
    uint8_t address2 = 129;
    uint8_t address3 = 130;
    uint8_t command1;
    uint8_t command2;
    uint8_t command3;

    if(speed1 > 127)
    {
        speed1 = speed1 - 128;
        command1 = 1;
    }
    else if(speed1 < 127)
    {
        command1 = 0;
        speed1 = abs(speed1-127);
    }
    else
    {
        command1 = 0;
        speed1 = 0;
    }

    if(speed2 > 127)
    {
        speed2 = speed2 - 128;
        command2 = 1;
    }
    else if(speed2 < 127)
    {
        command2 = 0;
        speed2 = abs(speed2-127);
    }
    else
    {
        command2 = 0;
        speed2 = 0;
    }

    if(speed3 > 127)
    {
        speed3 = speed3 - 128;
        command3 = 1;
    }
    else if(speed3 < 127)
    {
        command3 = 0;
        speed3 = abs(speed3-127);
    }
    else
    {
        command3 = 0;
        speed3 = 0;
    }
    motorCommand(address1, command1, speed1);
    motorCommand(address2, command2, speed2);
    motorCommand(address3, command3, speed3);
}
