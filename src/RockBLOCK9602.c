/*
 * RockBLOCK9602.c
 *
 *  Created on: 12 feb 2026
 *      Author: Control1
 */

/* Includes ---------------------------------------------------------------------------------------*/
#include "include/RockBLOCK9602.h"
#include "include/gpio.h"
#include "include/UART.h"



/* Private Implementation -------------------------------------------------------------------------*/

/*
 * @brief Configure RockBLOCK9602
 *
 * This function is responsible for configuring the RockBLOCK9602 module,
 * Send AT command to know if the module is working,
 * send ATE0 command to disable echo
 * and send AT&K0 to set the flow control to none.
 *
 * @return RB_Response_t indicating success or error
 */
 /*
RB_Response_t RB_Configure_Commands_and_responses(void)
{
    UART_Status_t status;
    // Send AT command to check if module is responsive
    
    for(i=0; i<rockblock.count; i++) {
        rockblock.RBDataRaw[i] = '\0'; // Clear buffer data
    }
    rockblock.head = rockblock.tail = rockblock.count = 0; // Reset circular buffer
    status = UART_API.sendString(&uart2_handle, AT, 3);
    if (status != UART_STATUS_SUCCESS) {
        return RB_AT_RESPONSE_RECEIVED_ERROR;
    }

    // Send ATE0 command to disable echo
    status = UART_API.sendString(&uart2_handle, ATE0, 5);
    if (status != UART_STATUS_SUCCESS) {
        return RB_ATE0_RESPONSE_RECEIVED_ERROR;
    }

    // Send AT&K0 command to set flow control to none
    status = UART_API.sendString(&uart2_handle, AT_K0, 6);
    if (status != UART_STATUS_SUCCESS) {
        return RB_AT_K0_RESPONSE_RECEIVED_ERROR;
    }

    return RB_AT_COMMANDS_RECEIVED_SUCCESS;
}
*/

