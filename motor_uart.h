/*
 * motor_uart.h
 *
 *  Created on: Oct 8, 2019
 *      Author: Brandon
 */

#ifndef MOTOR_UART_H_
#define MOTOR_UART_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/devices/cc32xx/inc/hw_types.h>
#include <ti/devices/cc32xx/driverlib/gpio.h>
#include <ti/devices/cc32xx/inc/hw_memmap.h>

/* Example/Board Header files */
#include "Board.h"
UART_Handle UART0;
UART_Handle UART1;

void Uartinit0();
void motorCommand(uint8_t address, uint8_t command, uint8_t speed);
void allMotorOperations(uint8_t speed1, uint8_t speed2, uint8_t speed3);



#endif /* MOTOR_UART_H_ */
