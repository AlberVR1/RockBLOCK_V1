/*
 * RockBLOCK9602.c
 *
 *  Created on: 12 feb 2026
 *      Author: Control1
 */

/* Includes ---------------------------------------------------------------------------------------*/
#include "RockBLOCK9602.h"
#include "tm4c123gh6pm.h"
//#include "mastercommands.h"

#include "gpio.h"
#include "UART.h"
#include "SysTick.h"



/* Private variables--------------------------------------------------------------------------------*/
static UART_Handle_t uart2_handle;
// GPIO handles for RockBLOCK9602 control and debugging
static GPIO_Handle_t input_pf2;
static GPIO_Handle_t input_pf3;
static GPIO_Handle_t output_pf4;

// Global structure
static ROCKBLOCK_Device_t rb_dev;



/* Char AT Command strings -------------------------------------------------------------------------*/
const char AT[] = "AT\r";   // Command to check if the module is responsive
const char ATE0[] = "ATE0\r";   // Command to disable echo
const char AT_K0[] = "AT&K0\r"; // Command to set flow control to none
const char AT_W0[] = "AT&W0\r"; // Command to write in user 0 profile
const char AT_Y0[] = "AT&Y0\r"; // Command to set the module with the user 0 profile at startup
const char AT_SBDMTA1[] = "AT+SBDMTA=1\r"; // Command to enable Mobile Terminated Alert (MTA) notifications
const char AT_SBDMTA0[] = "AT+SBDMTA=0\r"; // Command to disable Mobile Terminated Alert (MTA) notifications

const char AT_CSQ[] = "AT+CSQ\r";   // Command to check signal quality

const char AT_SBDD0[] = "AT+SBDD0\r";   // Command to clear the SBD buffer

                                        // -> M0 (Mobile Originated) buffer for outgoing messages
const char AT_SBDWT[] = "AT+SBDWT=";    // Command to write a message to the SBD buffer
                                        // -> M0 (Mobile Originated) buffer for outgoing messages
const char AT_SBDRT[] = "AT+SBDRT\r";    // Command to read the message in the SBD buffer
const char AT_SBDIX[] = "AT+SBDIX\r";   // Command to send messages from ISU to GSS and receive messages from GSS to ISU

/* Char AT Command responses------------------------------------------------------------------------*/
const char AT_RESPONSE_OK[] = "OK\r\n";
const char AT_RESPONSE_ERROR[] = "ERROR\r\n";
const char AT_RESPONSE_NO_SIGNAL[] = "\0\0\0\0\0";

// AT+CSQ response format: +CSQ: <rssi>,<ber>
const char AT_RESPONSE_CSQ_0[] = "+CSQ:0\r\n"; // Example response for no signal, where rssi is 0 and ber is 0
const char AT_RESPONSE_CSQ_1[] = "+CSQ:1\r\n"; // Example response for weak signal, where rssi is 1 and ber is 0
const char AT_RESPONSE_CSQ_2[] = "+CSQ:2\r\n"; // Example response for moderate signal, where rssi is 2 and ber is 0
const char AT_RESPONSE_CSQ_3[] = "+CSQ:3\r\n"; // Example response for good signal, where rssi is 3 and ber is 0
const char AT_RESPONSE_CSQ_4[] = "+CSQ:4\r\n"; // Example response for excellent signal, where rssi is 4 and ber is 0
const char AT_RESPONSE_CSQ_5[] = "+CSQ:5\r\n"; // Example response for very good signal, where rssi is 5 and ber is 0
const char AT_RESPONSE_CSQ_ERROR[] = "ERROR\r\n"; // Return error if expected response is not found in the buffer

// AT+SBDRT response format: +SBDRT: <SBD message>, where SBD message is the message in the SBD buffer that can be read with this command
const char AT_RESPONSE_SBDRT[] = "+SBDRT:"; // Identify "+SBDRT:" in response to parse the rest of the response for SBD message
// AT+SBDIX response format: +SBDIX: <MO status>,<MT status>,<SBD message number>,<MT message number>,<SBD message waiting indicator>
const char AT_RESPONSE_SBDIX_ECHO[] = "AT+SBDIX\r\r\n"; // Identify echo of the command to ignore it and wait for the real response
const char AT_RESPONSE_SBDIX[] = "+SBDIX:"; // Identify "+SBDIX:" in response to parse the rest of the response for MO status, MT status, SBD message number, MT message number and SBD message waiting indicator
// SBDRING response format: +SBDRING: <ring indicator>
const char AT_RESPONSE_SBDRING[] = "SBDRING"; // Identify "SBDRING" in response to parse the rest of the response for ring indicator
// READY response format: READY
const char AT_RESPONSE_READY[] = "READY\r\n";


/* Private Functions ------------------------------------------------------------------------------*/
// Static inline bool RB_IsValidHandle(const ROCKBLOCK_Handle_t *handle);
static GPIO_Status_t Configure_RB_GPIO(void);
static UART_Status_t Configure_UART_2(void);
static void clearBuffer(void);
static RB_Response_t RockBLOCKWakeUp(void);
static RB_Response_t RockBLOCKSleep(void);
static RB_Response_t RB_Configuring_Commands(void);
static RB_Response_t RB_Send_AT_Command(uint32_t timeout_ms);
//static RB_Response_t RB_Send_ATK0_Command(uint32_t timeout_ms);
//static RB_Response_t RB_Send_ATE0_Command(uint32_t timeout_ms);
static RB_Response_t RB_Send_ATCSQ_Command(uint32_t timeout_ms);
static RB_Response_t RB_Send_AT_SBDWT_Command(const bool long_message, const char *buff, uint32_t timeout_ms);
//static RB_Response_t RB_RingIndicator_Pin(bool activate,uint32_t timeout_ms);
static RB_Response_t RockBlockNetworkAvailability(void);
//static RB_Response_t RB_Send_ATW0_Command(uint32_t timeout_ms);
//static RB_Response_t RB_Send_ATY0_Command(uint32_t timeout_ms);
static RB_Response_t RB_Send_AT_SBDD0_Command(uint32_t timeout_ms);
static RB_Response_t RB_Send_AT_SBDRT_Command(uint8_t *buff, uint32_t timeout_ms);
static RB_Response_t RB_Send_AT_SBDIX_Command(uint8_t *mo_status, uint16_t *momsn, uint8_t *mt_status,
                                                uint16_t *mtmsn, uint16_t *mt_length,
                                                uint8_t *mt_queued, uint32_t timeout_ms);
static CSQ_Level_t atcsq_response_contains(const uint8_t *buff);
static int at_response_contains(const uint8_t *buff, const char *expected);
static void getString(uint8_t *buff, const uint8_t start_delimiter, const uint8_t end_delimiter);

void rb_callback(uint8_t data);
/* Private Implementation -------------------------------------------------------------------------*/
/**
 * @brief configure the system peripherals for RockBLOCK9602 communication
 *
 * This function calls the functions to configures the PLL to any MHz desired by user,
 * UART 1 for communication with Master MCU,
 * UART 2 for communication with the RockBLOCK9602 module,
 * configures the SysTick timer for timing operations and
 * GPIO pins for RocBLOCK9602 control and debugging.
 */
RB_Status_t RB_init(void)
{

    UART_Status_t statusuart2 = Configure_UART_2();

    // Configure GPIO 
    GPIO_Status_t status_gpio = Configure_RB_GPIO();
    if(status_gpio != GPIO_STATUS_SUCCESS) {
        return RB_STATUS_HW_ERROR;
    }

    // Start configuration 
    RB_Response_t at_status = RB_Configuring_Commands();
    switch(at_status) {
    case RB_WAKEUP_ERROR:
        return RB_STATUS_WAKEUP_ERROR;
    case RB_AT_COMMAND_RESPONSE_RECEIVED_ERROR:
    case RB_AT_K0_COMMAND_RESPONSE_RECEIVED_ERROR:
    case RB_ATE0_COMMAND_RESPONSE_RECEIVED_ERROR:
        return RB_STATUS_ERROR;
    case RB_AT_COMMAND_RESPONSE_TIMEOUT:
    case RB_AT_K0_COMMAND_RESPONSE_TIMEOUT:
    case RB_ATE0_COMMAND_RESPONSE_TIMEOUT:
        return RB_STATUS_TIMEOUT;
    }

    return RB_STATUS_OK;
}



/**
 * @brief Get the signal strength of the RockBLOCK9602 module
 *
 * This function sends the AT+CSQ command to the RockBLOCK9602 module and returns the signal strength level.
 *
 * @param level Pointer to the buffer where the signal strength will be stored
 *
 * @return RB_Status_t Returns RB_STATUS_NO_SIGNAL if no signal is detected, RB_STATUS_SIGNAL_OK if signal is detected
 * RB_STATUS_ERROR if any error occurs, RB_STATUS_TIMEOUT if the command times out
 *
 */
RB_Status_t RB_get_signal_strength(uint8_t *level)
{
    uint8_t tries = 0;
    *level = 0;
    rb_dev.at_response_received = RB_NO_AT_COMMAND_SENT;

    // Set PF4 high to wake up the RockBLOCK9602 module
    if(output_pf4.current_state != GPIO_STATE_HIGH) {
        RB_Response_t pinstatus = RockBLOCKWakeUp();
        if(pinstatus != RB_WAKEUP_OK) {
            return RB_STATUS_WAKEUP;
        }
    }
    do {
        rb_dev.at_response_received = RB_Send_ATCSQ_Command(20000); // Wait for response with a timeout of 20000 ms
        if(rb_dev.at_response_received >= RB_AT_CSQ_NO_SIGNAL && rb_dev.at_response_received <= RB_AT_CSQ_VERY_GOOD_SIGNAL) {
            break;
        }
        tries++;
    }while(tries<=3);

    // It's neccesary clear main buffer to wait a mmesage in queue
    clearBuffer();

    switch (rb_dev.at_response_received) {
        case RB_AT_CSQ_WEAK_SIGNAL:
            *level = 1;
            break;
        case RB_AT_CSQ_MODERATE_SIGNAL:
            *level = 2;
            break;
        case RB_AT_CSQ_GOOD_SIGNAL:
            *level = 3;
            break;
        case RB_AT_CSQ_EXCELLENT_SIGNAL:
            *level = 4;
            break;
        case RB_AT_CSQ_VERY_GOOD_SIGNAL:
            *level = 5;
            break;
    }
    switch (rb_dev.at_response_received) {
        case RB_AT_CSQ_NO_SIGNAL:
        case RB_AT_CSQ_WEAK_SIGNAL:
            return RB_STATUS_NO_SIGNAL;
        case RB_AT_CSQ_COMMAND_TIMEOUT:
            return RB_STATUS_TIMEOUT;
        case RB_AT_CSQ_COMMAND_ERROR:
            return RB_STATUS_ERROR;
        default:
            return RB_STATUS_SIGNAL_OK;
    }
}

/**
 * @brief This function sends all the necessary AT commands to send
 * a message to the ISU and post send message from ISU to GSS.
 *
 * @note You can send 121 characters as max, by security is better use 115 characters
 *
 * This function check the next functions using AT commands:
 * -> Checks if the RockBLOCK9602 module is awake
 * -> send AT+SBBD0 command to clear the ISU buffer
 * -> send AT+SBDWT command to write a message to the ISU buffer
 * -> send AT+SBDIX command to send messages from ISU to GSS and receive code responses from GSS to ISU
 * 
 * @param mo_status Pointer to the buffer where the MO status will be stored
 * @param momsn Pointer to the buffer where the MO MSN will be stored
 * @param mt_status Pointer to the buffer where the MT status will be stored
 * @param mtmsn Pointer to the buffer where the MT MSN will be stored
 * @param mt_length Pointer to the buffer where the MT length will be stored
 * @param mt_queued Pointer to the buffer where the MT queued will be stored
 * @param msg Pointer to the message to be sent
 *
 * @return RB_Status_t Returns RB_STATUS_OK if the message was sent successfully, 
 * Returns RB_STATUS_ERROR if any error occurs.
 * 
 */
RB_Status_t RB_send_message(RB_Data_t *data,
                            const char *msg)
{
    uint8_t tries = 0;
    rb_dev.at_response_received = RB_NO_AT_COMMAND_SENT;
    
    // Set PF4 high to wake up the RockBLOCK9602 module
    if(output_pf4.current_state != GPIO_STATE_HIGH) {
        RB_Response_t pinstatus = RockBLOCKWakeUp();
        if(pinstatus != RB_WAKEUP_OK) {
            return RB_STATUS_WAKEUP_ERROR;
        }
    }

    // clear buffer and send AT+SBDD0 command to clear the SBD buffer-> MO (Mobile Originated) buffer
    tries = 0;
    do {
        rb_dev.at_response_received = RB_Send_AT_SBDD0_Command(900); // Wait for response with a timeout of 900 ms
        if(rb_dev.at_response_received == RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
            break;
        }
        tries++;
    }while(tries<=3);
    switch(rb_dev.at_response_received) {
        case RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_ERROR:
            return RB_STATUS_ERROR;
        case RB_AT_SBDD0_COMMAND_RESPONSE_TIMEOUT:
            return RB_STATUS_TIMEOUT;
    }

    // Send message to ISU -> MO (Mobile Originated) buffer
    tries = 0;
    do {
        rb_dev.at_response_received = RB_Send_AT_SBDWT_Command(0,msg, 900); // Wait for response with a timeout of 900 ms
        if(rb_dev.at_response_received == RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
            break;
        }
        tries++;
    }while(tries<=3);
    switch(rb_dev.at_response_received) {
        case RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_ERROR:
            return RB_STATUS_ERROR;
        case RB_AT_SBDWT_COMMAND_RESPONSE_TIMEOUT:
            return RB_STATUS_TIMEOUT;
    }

    // Send messate from ISU -> GSS (GSM Originated) buffer
    tries = 0;
    do {
        rb_dev.at_response_received = RB_Send_AT_SBDIX_Command(&data->mo_status,    // Mobile Originated status for AT+SBDIX response
                                                               &data->momsn,       // Counter for MO (messages that you send), counter from 0 to 65,535 and returns to 0
                                                               &data->mt_status,   // Mobile Terminated status for AT+SBDIX response
                                                               &data->mtmsn,       // Counter for MT (messages that is received), counter from 0 to 65,535 and returns to 0
                                                               &data->mt_length,   // Is the lenght, in bytes, of message terminated received from GSS.
                                                               &data->mt_queued,   // Is the message terminated quantity that are in queue in the GSS to be send to ISU.
                                                                40000); // Wait for response with a timeout of 900 ms
        if(rb_dev.at_response_received == RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
            break;
        }
        tries++;
    }while(tries<=3);
    
    // It's neccesary clear main buffer to wait a mmesage in queue
    clearBuffer();

    switch(rb_dev.at_response_received) {
        case RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_ERROR:
            return RB_STATUS_ERROR;
        case RB_AT_SBDIX_COMMAND_RESPONSE_TIMEOUT:
            return RB_STATUS_TIMEOUT;
    }

    if(data->mo_status<=2) {
        return RB_STATUS_MESSAGE_SENT;
    } else {
        return RB_STATUS_MESSAGE_NO_SENT;
    }
}

/**
 * @brief This function uses all the necessary AT commands to transfer
 * a message to the ISU and post send a long message from ISU to GSS.
 *
 * @note You can send 340 characters as max, by security is better use 335 characters
 *
 * This function check the next functions using AT commands:
 * -> Checks if the RockBLOCK9602 module is awake
 * -> send AT+SBBD0 command to clear the ISU buffer
 * -> send AT+SBDWT command to write a message to the ISU buffer
 * -> send AT+SBDIX command to send messages from ISU to GSS and receive code responses from GSS to ISU
 *
 * @param mo_status Pointer to the buffer where the MO status will be stored
 * @param momsn Pointer to the buffer where the MO MSN will be stored
 * @param mt_status Pointer to the buffer where the MT status will be stored
 * @param mtmsn Pointer to the buffer where the MT MSN will be stored
 * @param mt_length Pointer to the buffer where the MT length will be stored
 * @param mt_queued Pointer to the buffer where the MT queued will be stored
 * @param msg Pointer to the message to be sent
 *
 * @return RB_Status_t Returns RB_STATUS_OK if the message was sent successfully,
 * Returns RB_STATUS_ERROR if any error occurs.
 *
 */
RB_Status_t RB_send__long_message(RB_Data_t *data,
                                    const char *msg1,
                                    uint8_t *msg2)
{
    RB_Response_t responsenet = RB_NO_AT_COMMAND_SENT;
    uint8_t tries = 0;
    rb_dev.at_response_received = RB_NO_AT_COMMAND_SENT;

    // Set PF4 high to wake up the RockBLOCK9602 module
    if(output_pf4.current_state != GPIO_STATE_HIGH) {
        RB_Response_t pinstatus = RockBLOCKWakeUp();
        if(pinstatus != RB_WAKEUP_OK) {
            //return RB_STATUS_WAKEUP_ERROR;
        }
    }

    // Flag to check if a command is not send (sending)
    rb_dev.isatcommandsent = ATCOMMAND_SENDING;

    // clear buffer and send AT+SBDD0 command to clear the SBD buffer-> MO (Mobile Originated) buffer
    tries = 0;
    do {
        rb_dev.at_response_received = RB_Send_AT_SBDD0_Command(5000); // Wait for response with a timeout of 900 ms
        if(rb_dev.at_response_received == RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
            break;
        }
        tries++;
    }while(tries<=0);
    switch(rb_dev.at_response_received) {
    case RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_ERROR:
        return RB_SBDD0_STATUS_ERROR;
    case RB_AT_SBDD0_COMMAND_RESPONSE_TIMEOUT:
        return RB_SBDD0_STATUS_TIMEOUT;
    }
    // Check Network Availability by PF3 three times
    // It's necessary to get a stability
    tries = 0;
    do {
        responsenet = RockBlockNetworkAvailability();
        if(responsenet == RB_NETAV_OK) {
            break;
        }
        SYSTICK_API.delay_ms(1000);
        tries++;
    }while(tries<=3);

    // Send message to ISU -> MO (Mobile Originated) buffer
    tries = 0;
    do {
        rb_dev.at_response_received = RB_Send_AT_SBDWT_Command(1,msg1, 5000); // Wait for response with a timeout of 900 ms
        if(rb_dev.at_response_received == RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
            break;
        }
        tries++;
    }while(tries<=3);
    switch(rb_dev.at_response_received) {
        case RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_ERROR:
            return RB_SBDWT_STATUS_ERROR;
        case RB_AT_SBDWT_COMMAND_RESPONSE_TIMEOUT:
            return RB_SBDWT_STATUS_TIMEOUT;
    }

    // Send message from ISU -> GSS (GSM Originated) buffer
    tries = 0;
    do {
        rb_dev.at_response_received = RB_Send_AT_SBDIX_Command(&data->mo_status,    // Mobile Originated status for AT+SBDIX response
                                                               &data->momsn,       // Counter for MO (messages that you send), counter from 0 to 65,535 and returns to 0
                                                               &data->mt_status,   // Mobile Terminated status for AT+SBDIX response
                                                               &data->mtmsn,       // Counter for MT (messages that is received), counter from 0 to 65,535 and returns to 0
                                                               &data->mt_length,   // Is the length, in bytes, of message terminated received from GSS.
                                                               &data->mt_queued,   // Is the message terminated quantity that are in queue in the GSS to be send to ISU.
                                                                40000); // Wait for response with a timeout of 900 ms
        if(rb_dev.at_response_received == RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
            break;
        }
        tries++;
    }while(tries<=3);
    // It's neccesary clear main buffer to wait a mmesage in queue
    clearBuffer();
    // Flag to check if a command is not send
    rb_dev.isatcommandsent = ATCOMMAND_NOSENT;
    switch(rb_dev.at_response_received) {
        case RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_ERROR:
            return RB_STATUS_ERROR;
        case RB_AT_SBDIX_COMMAND_RESPONSE_TIMEOUT:
            return RB_STATUS_TIMEOUT;
    }

    if(data->mo_status<=2) {
        if(data->mt_status == 1) {

            rb_dev.at_response_received = RB_Send_AT_SBDRT_Command(msg2, 5000); // Wait for response with a timeout of 900 ms

            // It's neccesary clear main buffer to wait a mmesage in queue
            clearBuffer();

            switch(rb_dev.at_response_received) {
                case RB_AT_SBDRT_COMMAND_RESPONSE_RECEIVED_ERROR:
                    return RB_SBDRT_STATUS_ERROR;
                case RB_AT_SBDRT_COMMAND_RESPONSE_TIMEOUT:
                    return RB_SBDRT_STATUS_TIMEOUT;
            }
            if(data->mt_queued > 0) {
                return RB_STATUS_MESSAGE_SENT_AND_MESSAGE_RECEIVED_WITH_QUEUE;
            }
            return RB_STATUS_MESSAGE_SENT_AND_MESSAGE_RECEIVED;
        }
        return RB_STATUS_MESSAGE_SENT;
    } else {
        return RB_STATUS_MESSAGE_NO_SENT;
    }
}

/**
 * @brief Receive a message from the RockBLOCK9602 module
 *
 * @param mo_status Pointer to the buffer where the MO status will be stored
 * @param momsn Pointer to the buffer where the MO MSN will be stored
 * @param mt_status Pointer to the buffer where the MT status will be stored
 * @param mtmsn Pointer to the buffer where the MT MSN will be stored
 * @param mt_length Pointer to the buffer where the MT length will be stored
 * @param mt_queued Pointer to the buffer where the MT queued will be stored
 * @param msg Pointer to the buffer where the message will be stored
 *
 * @return RB_Status_t Returns RB_STATUS_OK if the message was received successfully,
 *
 **/
RB_Status_t RB_receive_check(RB_Data_t *data,
                            uint8_t *msg)
{
    RB_Response_t responsenet = RB_NO_AT_COMMAND_SENT;
    uint8_t tries = 0;
    rb_dev.at_response_received = RB_NO_AT_COMMAND_SENT;
    // Set PF4 high to wake up the RockBLOCK9602 module
    if(output_pf4.current_state != GPIO_STATE_HIGH) {
        RB_Response_t pinstatus = RockBLOCKWakeUp();
        if(pinstatus != RB_WAKEUP_OK) {
            return RB_STATUS_WAKEUP_ERROR;
        }
    }

    // Flag to check if a command is not send (sending)
    rb_dev.isatcommandsent = ATCOMMAND_SENDING;
    // clear buffer and send AT+SBDD0 command to clear the SBD buffer-> MO (Mobile Originated) buffer
    tries = 0;
    do {
        rb_dev.at_response_received = RB_Send_AT_SBDD0_Command(900); // Wait for response with a timeout of 900 ms
        if(rb_dev.at_response_received == RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
            break;
        }
        tries++;
    }while(tries<=0);
    switch(rb_dev.at_response_received) {
        case RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_ERROR:
            return RB_SBDD0_STATUS_ERROR;
        case RB_AT_SBDD0_COMMAND_RESPONSE_TIMEOUT:
            return RB_SBDD0_STATUS_TIMEOUT;
    }

    // Check Network Availability by PF3 three times
    // It's necessary to get a stability
    tries = 0;
    do {
        responsenet = RockBlockNetworkAvailability();
        if(responsenet == RB_NETAV_OK) {
            break;
        }
        SYSTICK_API.delay_ms(1000);
        tries++;
    }while(tries<=3);
    // Send message from ISU -> GSS (GSM Originated) buffer
    tries = 0;
    do
    {
        rb_dev.at_response_received = RB_Send_AT_SBDIX_Command(&data->mo_status,    // Mobile Originated status for AT+SBDIX response
                                                               &data->momsn,       // Counter for MO (messages that you send), counter from 0 to 65,535 and returns to 0
                                                               &data->mt_status,   // Mobile Terminated status for AT+SBDIX response
                                                               &data->mtmsn,       // Counter for MT (messages that is received), counter from 0 to 65,535 and returns to 0
                                                               &data->mt_length,   // Is the length, in bytes, of message terminated received from GSS.
                                                               &data->mt_queued,   // Is the message terminated quantity that are in queue in the GSS to be send to ISU.
                                                               40000); // Wait for response with a timeout of 900 ms
        if(rb_dev.at_response_received == RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
            // Make the action when a several messages are in queue
            break;
        }
        tries++;
    }while(tries<= 3);
    // Flag to check if a command is not send (sending)
    rb_dev.isatcommandsent = ATCOMMAND_SENDING;
    // It's neccesary clear main buffer to wait a mmesage in queue
    clearBuffer();
    switch(rb_dev.at_response_received) {
        case RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_ERROR:
            return RB_SBDIX_STATUS_ERROR;
        case RB_AT_SBDIX_COMMAND_RESPONSE_TIMEOUT:
            return RB_SBDIX_STATUS_TIMEOUT;
    }
    //if(*mo_status >= 1)
    //{
    //    return RB_STATUS_SBD_SESSION_FAILURE;
    //}
    if(data->mt_status == 1)
    {
        // Flag to check if a command is not send (sending)
        rb_dev.isatcommandsent = ATCOMMAND_SENDING;
        rb_dev.at_response_received = RB_Send_AT_SBDRT_Command(msg, 900); // Wait for response with a timeout of 900 ms
        
        // It's neccesary clear main buffer to wait a mmesage in queue
        clearBuffer();

        // Flag to check if a command is not send
        rb_dev.isatcommandsent = ATCOMMAND_NOSENT;

        switch(rb_dev.at_response_received) {
            case RB_AT_SBDRT_COMMAND_RESPONSE_RECEIVED_ERROR:
                return RB_SBDRT_STATUS_ERROR;
            case RB_AT_SBDRT_COMMAND_RESPONSE_TIMEOUT:
                return RB_SBDRT_STATUS_TIMEOUT;
        }
        if(data->mt_queued > 0)
        {
                return RB_STATUS_MESSAGE_RECEIVED_WITH_QUEUE;
        }
            return RB_STATUS_MESSAGE_RECEIVED;
    }
    else if(data->mt_status == 0 && data->mo_status == 0)
    {
        return RB_STATUS_MESSAGE_NO_EXIST;
    }
    else if(data->mo_status > 2)
    {
        return RB_STATUS_SBD_SESSION_FAILURE;
    }
    else
    {
        return RB_STATUS_MESSAGE_NO_RECEIVED;
    }

    //// Check page 112 from Iridium ISU AT Command Reference v5.pdf for sbdix command
    //// Check page 107 from Iridium ISU AT Command Reference v5.pdf for sbdd command
}

/**
 *
 *
 *
 *
 *
 **/
RB_Status_t RB_waiting_for_message(void)
{

    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_SBDRING)) {
        return RB_STATUS_MESSAGE_IN_QUEUE;
    }
    return RB_STATUS_NO_MESSAGE_IN_QUEUE; // Return timeout if expected response is not received within the specified time
}
/**
 * @brief This function sends all the necessary AT commands to configure the RockBLOCK9602 module
 *
 * This function sends the AT command to check if the module is responsive, 
 * the AT&K0 command to set flow control to none and the ATE0 command to disable echo.
 * It waits for the expected response from the module for each command and returns
 * an error if the expected response is not received within the specified timeout.
 *
 * @return RB_Response_t Returns RB_AT_COMMANDS_FOR_CONFIGURATION_RECEIVED_SUCCESS if all commands were sent without issues, 
 * Returns RB_WAKEUP_ERROR if the RockBLOCK9602 module does not wake up, 
 * Returns RB_AT_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is received from the module, 
 * Returns RB_AT_COMMAND_RESPONSE_TIMEOUT if the expected response is not received within the specified timeout.
 */
static RB_Response_t RB_Configuring_Commands(void)
{
    rb_dev.at_response_received = RB_NO_AT_COMMAND_SENT;

    // Set PF4 high to wake up the RockBLOCK9602 module
    RB_Response_t pinstatus = RockBLOCKWakeUp();
    if(pinstatus != RB_WAKEUP_OK) {
        return pinstatus;
    }
    // Flag to check if a command is not send (sending)
    rb_dev.isatcommandsent = ATCOMMAND_SENDING;

    // Send AT command to check if module is responsive
    rb_dev.at_response_received = RB_Send_AT_Command(900); // Wait for response with a timeout of 900 ms

    // It's neccesary clear main buffer to wait a mmesage in queue
    clearBuffer();
    if(rb_dev.at_response_received != RB_AT_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
        return rb_dev.at_response_received; // Return error if expected response is not received
    }
    SYSTICK_API.delay_ms(1000);

    // Flag to check if a command is not send (sending)
    rb_dev.isatcommandsent = ATCOMMAND_NOSENT;

    /*// Send AT&K0 command to set flow control to none
    rb_dev.at_response_received = RB_Send_ATK0_Command(900); // Wait for response with a timeout of 5000 ms
    if(rb_dev.at_response_received != RB_AT_K0_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
        return rb_dev.at_response_received; // Return error if expected response is not received
    }
    PLL_API.delayMs(1000); // Short delay before sending the next command
    // Send ATE0 command to disable echo
    rb_dev.at_response_received = RB_Send_ATE0_Command(900); // Wait for response with a timeout of 5000 ms
    if(rb_dev.at_response_received != RB_ATE0_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
        return rb_dev.at_response_received; // Return error if expected response is not received
    }
    PLL_API.delayMs(1000); // Short delay before sending the next command
    rb_dev.at_response_received = RB_RingIndicator_Pin(true, 900); // Wait for response with a timeout of 5000 ms
    if(rb_dev.at_response_received != RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
        return rb_dev.at_response_received; // Return error if expected response is not received
    }
    PLL_API.delayMs(1000); // Short delay before sending the next command
    rb_dev.at_response_received = RB_Send_ATW0_Command(900); // Wait for response with a timeout of 5000 ms
    if(rb_dev.at_response_received != RB_ATW0_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
        return rb_dev.at_response_received; // Return error if expected response is not received
    }
    PLL_API.delayMs(1000); // Short delay before sending the next command
    rb_dev.at_response_received = RB_Send_ATY0_Command(900); // Wait for response with a timeout of 5000 ms
    if(rb_dev.at_response_received != RB_ATY0_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
        return rb_dev.at_response_received; // Return error if expected response is not received
    }*/

    return rb_dev.at_response_received =RB_AT_COMMANDS_FOR_CONFIGURATION_RECEIVED_SUCCESS; // Return success if all commands were sent without issues
}

/*
 * @brief compare the received response and get the expected response just for AT command
 *
 * This function compares the received response in the buffer with the expected response string.
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_AT_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_AT_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_AT_COMMAND_RESPONSE_TIMEOUT
 *
 */
static RB_Response_t RB_Send_AT_Command(uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    UART_API.sendString(&uart2_handle, AT,3);
    //SYSTICK_API.delay_ms(timeout_ms);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK))
        {
            SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
            SYSTICK_API.delay_ms(50);
            break;
        }
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_AT_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}

/*
 * @brief compare the received response and get the expected response just for AT&K0 command
 *
 * This function send AT&K0 command to set flow control to none and
 * compares the received response in the buffer with the expected response string.
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_AT_K0_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_AT_K0_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_AT_K0_COMMAND_RESPONSE_TIMEOUT
 *
 */
/*static RB_Response_t RB_Send_ATK0_Command(uint32_t timeout_ms)
{
    //uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    UART_API.sendString(&uart2_handle, AT_K0,6);
    SYSTICK_API.delay_ms(timeout_ms);
    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_AT_K0_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_K0_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_K0_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}*/

/*
 * @brief compare the received response and get the expected response just for ATE0 command
 *
 * This function send ATE0 command to disable echo and compares the received response
 * in the buffer with the expected response string.
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_ATE0_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_ATE0_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_ATE0_COMMAND_RESPONSE_TIMEOUT
 *
 */
/*static RB_Response_t RB_Send_ATE0_Command(uint32_t timeout_ms)
{
    //uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    UART_API.sendString(&uart2_handle, ATE0,5);
    SYSTICK_API.delay_ms(timeout_ms);
    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_ATE0_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_ATE0_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_ATE0_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}*/

/*
 * @brief compare the received response and get the expected response just for AT+SBDMTA=1 or AT+SBDMTA=0 commands
 *
 * This function sends AT+SBDMTA=1 if you want to use RI pin or AT+SBDMTA=0 if you want don't use RI pin
 * and compares the received response in the buffer with the expected response string.
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_ATSBDMTA_COMMAND_RESPONSE_TIMEOUT
 *
 */
/*static RB_Response_t RB_RingIndicator_Pin(bool activate,uint32_t timeout_ms)
{
    //uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    if(activate) {
        UART_API.sendString(&uart2_handle, AT_SBDMTA1,12);
    }
    else {
        UART_API.sendString(&uart2_handle, AT_SBDMTA0,12);
    }
    SYSTICK_API.delay_ms(timeout_ms);
    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_ATSBDMTA_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}*/

/*static RB_Response_t RB_Send_ATW0_Command(uint32_t timeout_ms)
{
    //uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    UART_API.sendString(&uart2_handle, AT_W0,6);
    SYSTICK_API.delay_ms(timeout_ms);
    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_ATW0_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_ATW0_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_ATW0_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}*/

/*static RB_Response_t RB_Send_ATY0_Command(uint32_t timeout_ms)
{
    //uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    UART_API.sendString(&uart2_handle, AT_Y0,6);
    SYSTICK_API.delay_ms(timeout_ms);
    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_ATY0_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_ATY0_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_ATY0_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}*/


/*
 * @brief compare the received response and get the expected response just for AT+CSQ command
 *
 * This function send AT+CSQ command to check the signal quality and compares the received response
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_AT_CSQ_NO_SIGNAL, RB_AT_CSQ_WEAK_SIGNAL, RB_AT_CSQ_MODERATE_SIGNAL,
 * RB_AT_CSQ_GOOD_SIGNAL, RB_AT_CSQ_EXCELLENT_SIGNAL or RB_AT_CSQ_VERY_GOOD_SIGNAL
 *
 */
static RB_Response_t RB_Send_ATCSQ_Command(uint32_t timeout_ms)
{
    CSQ_Level_t csq_level;
    uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    UART_API.sendString(&uart2_handle, AT_CSQ,7);

    SYSTICK_API.delay_ms(1000);
    // Start timeout counter
    //SYSTICK_API.delay_ms(timeout_ms);
    SYSTICK_API.Stop_Count();
    getmilis = SYSTICK_API.milis();
    SYSTICK_API.Start_Count();
    while(getmilis < timeout_ms)
    {
        csq_level = atcsq_response_contains(rb_dev.RBDataRaw);
        if(csq_level != CSQ_TIMEOUT)
        {
            SYSTICK_API.Stop_Count();
            SYSTICK_API.delay_ms(50);
            break;
        }
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count();



    csq_level = atcsq_response_contains(rb_dev.RBDataRaw);
    switch(csq_level) {
        case CSQ_LEVEL_0:
            return RB_AT_CSQ_NO_SIGNAL;
        case CSQ_LEVEL_1:
            return RB_AT_CSQ_WEAK_SIGNAL;
        case CSQ_LEVEL_2:
            return RB_AT_CSQ_MODERATE_SIGNAL;
        case CSQ_LEVEL_3:
            return RB_AT_CSQ_GOOD_SIGNAL;
        case CSQ_LEVEL_4:
            return RB_AT_CSQ_EXCELLENT_SIGNAL;
        case CSQ_LEVEL_5:
            return RB_AT_CSQ_VERY_GOOD_SIGNAL;
        case CSQ_LEVEL_ERROR:
            return RB_AT_CSQ_COMMAND_ERROR;
    }
    return RB_AT_CSQ_COMMAND_TIMEOUT; // Return timeout if expected response is not received within the specified time
}

/*
 * @brief compare the received response and get the expected response just for AT+SBDD0 command
 *
 * This function send AT+SBDD0 command to clear the SBD buffer-> MO (Mobile Originated) buffer
 * for outgoing messages and compares the received response
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_AT_SBDD0_COMMAND_RESPONSE_TIMEOUT
 *
 */
static RB_Response_t RB_Send_AT_SBDD0_Command(uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    UART_API.sendString(&uart2_handle, AT_SBDD0, 9);
    //SYSTICK_API.delay_ms(timeout_ms);


    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK))
        {
            SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
            SYSTICK_API.delay_ms(50);
            break;
        }
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting


    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_SBDD0_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}

/*
 * @brief compare the received response and get the expected response just for AT+SBDWT command
 *
 * This function send AT+SBDWT command to write a message to the SBD buffer
 * -> MO (Mobile Originated) buffer
 *
 * @param long_message If it's true, the message is long, otherwise it's short
 * @param buff Pointer to send the desired message
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_AT_SBDWT_COMMAND_RESPONSE_TIMEOUT
 *
 */
static RB_Response_t RB_Send_AT_SBDWT_Command(const bool long_message, const char *buff, uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    if(long_message)
    {
        UART_API.sendString(&uart2_handle, AT_SBDWT, 8);
        UART_API.sendString(&uart2_handle, "\r", 1);
        //SYSTICK_API.delay_ms(timeout_ms);

        SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
        getmilis  = SYSTICK_API.milis();
        SYSTICK_API.Start_Count(); // Start SysTick to wait for response
        while(getmilis < timeout_ms)
        {
            if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_READY))
            {
                SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
                SYSTICK_API.delay_ms(50);
                break;
            }
            getmilis = SYSTICK_API.milis();
        }
        SYSTICK_API.Stop_Count(); // Stop SysTick after waiting

        if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR))
        {
            return RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_ERROR;
        }
        else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_READY))
        {
            UART_API.sendString(&uart2_handle, buff, strlen((const char*)buff));
            UART_API.sendString(&uart2_handle, "\r", 1);
        }
        else
        {
            return RB_AT_SBDWT_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
        }
    }
    else
    {
        UART_API.sendString(&uart2_handle, AT_SBDWT, 9);
        UART_API.sendString(&uart2_handle, buff, strlen((const char*)buff));
        UART_API.sendString(&uart2_handle, "\r", 1);
    }
    //SYSTICK_API.delay_ms(timeout_ms);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK))
        {
            SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
            SYSTICK_API.delay_ms(50);
            break;
        }
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    return RB_AT_SBDWT_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time

}

static RB_Response_t RB_Send_AT_SBDRT_Command(uint8_t *buff, uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    UART_API.sendString(&uart2_handle, AT_SBDRT, 9);
    //SYSTICK_API.delay_ms(timeout_ms);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_SBDRT))
        {
            SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
            SYSTICK_API.delay_ms(200);
            break;
        }
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting

    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_SBDRT)) {
        getString(buff, ':', 13); // Get the message from the response, the message is between the LineFeed LF and CarriageReturn CR
        return RB_AT_SBDRT_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_SBDRT_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_SBDRT_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}

static RB_Response_t RB_Send_AT_SBDIX_Command(uint8_t *mo_status, uint16_t *momsn, uint8_t *mt_status, uint16_t *mtmsn, uint16_t *mt_length, uint8_t *mt_queued, uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    // Clear main buffer
    clearBuffer();
    UART_API.sendString(&uart2_handle, AT_SBDIX, 9);
    //SYSTICK_API.delay_ms(timeout_ms);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_SBDIX))
        {
            SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
            SYSTICK_API.delay_ms(50);
            break;
        }
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_SBDIX))
    {
        // Parse the response to extract MO and MT status
        char *str = (char*)rb_dev.RBDataRaw;
        char *pos = strtok(str, ":,");
        int pos_index = 0;
        while(pos != NULL) {
            if(pos_index == 1) {    // MO status
                *mo_status = (uint8_t)atoi(pos);
            }
            else if(pos_index == 2) {    //MO MSN
                *momsn = (uint16_t)atoi(pos);
            }
            else if(pos_index == 3) {   // MT status
                *mt_status = (uint8_t)atoi(pos);
            }
            else if(pos_index == 4) {   // MT MSN
                *mtmsn = (uint16_t)atoi(pos);
            }
            else if(pos_index == 5) {   // MT_Length
                *mt_length = (uint16_t)atoi(pos);
            }
            else if(pos_index == 6) {   // MT_Queued
                *mt_queued = (uint8_t)atoi(pos);
            }
            pos = strtok(NULL, ":,");
            pos_index++;
        }
        return RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rb_dev.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_SBDIX_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}

/*
 * @brief compare the received response with the expected response
 *
 * This function compares the received response in the buffer with the expected response string.
 *
 * @param buff Pointer to the buffer containing the received response
 * @param expected Pointer to the expected response string
 * @return int Returns 1 if the expected response is found in the buffer, otherwise returns 0
 *
 */
static int at_response_contains(const uint8_t *buff, const char *expected)
{
    return strstr((const char*)buff, expected) != NULL;
}


static CSQ_Level_t atcsq_response_contains(const uint8_t *buff)
{
    if(strstr((const char*)buff, AT_RESPONSE_CSQ_0) != NULL) {
        return CSQ_LEVEL_0;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_1) != NULL) {
        return CSQ_LEVEL_1;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_2) != NULL) {
        return CSQ_LEVEL_2;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_3) != NULL) {
        return CSQ_LEVEL_3;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_4) != NULL) {
        return CSQ_LEVEL_4;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_5) != NULL) {
        return CSQ_LEVEL_5;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_ERROR) != NULL) {
        return CSQ_LEVEL_ERROR; // Return error if expected response is not found in the buffer
    }
    else {
        return CSQ_TIMEOUT; // Return timeout if expected response is not found in the buffer
    }
}

static void getString(uint8_t *buff, const uint8_t start_delimiter, const uint8_t end_delimiter)
{
    uint16_t pos = 0;
    uint16_t index = 0;
    char *str = (char*)rb_dev.RBDataRaw;
    while(str[pos] != start_delimiter && str[pos] != '\0') { //Check for the start delimiter
        pos++;
    }
    pos += 3;
    while(str[pos] != end_delimiter && str[pos] != '\0') { //Check for the end delimiter
        buff[index++] = str[pos++];
    }
    buff[index] = '\0'; // Null-terminate the string
}

RB_Status_t RB_WakeUp(void)
{
    RB_Response_t pinstatus = RB_NO_AT_COMMAND_SENT;

    pinstatus = RockBLOCKWakeUp();
    if(pinstatus != RB_WAKEUP_OK) {
        return RB_STATUS_ERROR;
    }
    
    return RB_STATUS_WAKEUP;

}

RB_Status_t RB_Sleep(void)
{
    RB_Response_t pinstatus = RB_NO_AT_COMMAND_SENT;
     pinstatus = RockBLOCKSleep();
    if(pinstatus != RB_SLEEP_OK)
    {
        return RB_STATUS_ERROR;
    }
        return RB_STATUS_SLEEP;
}

/**
 * @brief this pin PF4 actives RockBLOCK to get commands
 *
 * @return if pin is set returns RB_WAKEUP_OK, if an error ocuurs returns RB_WAKEUP_ERROR
 */
static RB_Response_t RockBLOCKWakeUp(void)
{
    // Set PF4 high to wake up the RockBLOCK9602 module
    GPIO_Status_t pinstatus = GPIO_API.SetPin(&output_pf4);
    //PLL_API.delayMs(20000); //Wait until RockBLOCK supercondenser charges
    if(pinstatus != GPIO_STATUS_SUCCESS) {
        return RB_WAKEUP_ERROR;
    }
    return RB_WAKEUP_OK;
}

/**
 * @brief this pin PF4 clears RockBLOCK to deactivate transceptor
 *
 * @return if pin is set returns RB_SLEEP_OK, if an error ocuurs returns RB_SLEEP_ERROR
 */
static RB_Response_t RockBLOCKSleep(void)
{
    // Set PF4 high to wake up the RockBLOCK9602 module
    GPIO_Status_t pinstatus = GPIO_API.ClearPin(&output_pf4);
    if(pinstatus != GPIO_STATUS_SUCCESS) {
        return RB_SLEEP_ERROR;
    }
    return RB_SLEEP_OK;
}


static RB_Response_t RockBlockNetworkAvailability(void)
{
    GPIO_State_t pinvalue = GPIO_STATE_LOW;
    // Read PF3 to check the network availability status from the RockBLOCK9602 module
    GPIO_Status_t pinstatus = GPIO_API.ReadPin(&input_pf3, &pinvalue);
    if(pinstatus != GPIO_STATUS_SUCCESS) {
        return RB_NETAV_ERROR;
    }
    else if(pinvalue == GPIO_STATE_HIGH) {
        return RB_NETAV_OK;
    }
    else {
        return RB_NO_NETAV;
    }
}

/**
 * @brief
 *
 *
 *
 *
 **/
static void clearBuffer(void)
{
    uint32_t i = 0;
    // Clear main buffer
    if((rb_dev.RBDataRaw[0] != '\0') && (rb_dev.RBDataRaw[1] != '\0'))
    {
        for(i=0; i<rb_dev.count; i++) {
            rb_dev.RBDataRaw[i] = '\0'; // Clear buffer data
        }
        rb_dev.head = rb_dev.tail = rb_dev.count = 0; // Reset circular buffer
    }
}

/**
 * @brief configure GPIO pins for RockBLOCK9602 control and debugging
 *
 * GPIO_PF2 is configured as input for read the Ring Indicator (RI) signal from the RockBLOCK9602 module,
 * GPIO_PF3 is configured as input for read the Network Availability (NetAv) signal from the RockBLOCK9602 module and
 * GPIO_PF4 is configured as output for control the power of the RockBLOCK9602 module.
 */
static GPIO_Status_t Configure_RB_GPIO(void)
{
    ////////////////////////////////////////////////////// Configure GPIO PF2 RI (Ring Indicator) pin from the RockBLOCK9602 module
    GPIO_Config_t pf2_config = {
        .mode = GPIO_MODE_INPUT,            // Input mode
        .pull = GPIO_PULL_NONE,             // None pull-up/down
        .drive_strength = GPIO_DRIVE_2MA,   // 2mA drive strength
        .interrupt_type = GPIO_INT_NONE,    // None interrupt
        .initial_state = GPIO_STATE_LOW,    // Initial state: low (not used for input mode)
        .slew_rate_control = false,         // None slew rate control
        .open_drain = false,                // Push-pull (no open drain)
        .alternate_function = 0             // No alternate function
    };

    GPIO_Status_t status_gpiopf2 = GPIO_API.Init(&input_pf2,
                                              GPIO_PORT_F,
                                              GPIO_PIN_2,
                                              &pf2_config);
    if(status_gpiopf2 != GPIO_STATUS_SUCCESS) {
        return status_gpiopf2;
    }

    SYSTICK_API.delay_ms(200);
    ////////////////////////////////////////////////////// Configure GPIO PF3 Network Availability pin from the RockBLOCK9602 module
    GPIO_Config_t pf3_config = {
        .mode = GPIO_MODE_INPUT,            // Input mode
        .pull = GPIO_PULL_NONE,             // None pull-up/down
        .drive_strength = GPIO_DRIVE_2MA,   // 2mA drive strength
        .interrupt_type = GPIO_INT_NONE,    // None interrupt
        .initial_state = GPIO_STATE_LOW,    // Initial state: low (not used for input mode)
        .slew_rate_control = false,         // None slew rate control
        .open_drain = false,                // Push-pull (no open drain)
        .alternate_function = 0             // No alternate function
    };

    GPIO_Status_t status_gpiopf3 = GPIO_API.Init(&input_pf3,
                                              GPIO_PORT_F,
                                              GPIO_PIN_3,
                                              &pf3_config);
    if(status_gpiopf3 != GPIO_STATUS_SUCCESS) {
        return status_gpiopf3;
    }
    SYSTICK_API.delay_ms(200);
    ////////////////////////////////////////////////////// Configure GPIO PF4 On/Off pin from the RockBLOCK9602 module
    GPIO_Config_t pf4_config = {
        .mode = GPIO_MODE_OUTPUT,           // Output mode
        .pull = GPIO_PULL_NONE,             // None pull-up/down
        .drive_strength = GPIO_DRIVE_2MA,   // 2mA drive strength
        .interrupt_type = GPIO_INT_NONE,    // None interrupt
        .initial_state = GPIO_STATE_LOW,   // Initial state: high (turn on the module)
        .slew_rate_control = false,         // None slew rate control
        .open_drain = false,                // Push-pull (no open drain)
        .alternate_function = 0             // No alternate function
    };

    GPIO_Status_t status_gpiopf4 = GPIO_API.Init(&output_pf4,
                                              GPIO_PORT_F,
                                              GPIO_PIN_4,
                                              &pf4_config);
    if(status_gpiopf4 != GPIO_STATUS_SUCCESS) {
        return status_gpiopf4;
    }

    return GPIO_STATUS_SUCCESS;
}



/**
 * @brief configure UART 2 module
 *
 * This function initializes UART 2 for
 * communication with the RockBLOCK9602 module with 119200 baudrate
 */
static UART_Status_t Configure_UART_2(void)
{
    UART_Config_t uart2_config = {
       .module = UART_MODULE_2,
       .baudRate = UART_BAUD_19200,
       .clockFreqMHz = 40,
       .enableTx = true,
       .enableRx = true,
       .enableFIFO = false,
       .fifoLevel = UART_FIFO_LEVEL_1_8
    };
    // Initialize UART 2 module
    UART_Status_t statusuart2 = UART_API.init(&uart2_handle,
                                           &uart2_config);
    // The UART module initialized correctly
    if(statusuart2 != UART_STATUS_SUCCESS) {
        return statusuart2;
    }
    UART_API.enableInterrupt(&uart2_handle,UART_FIFO_LEVEL_1_8, rb_callback);
    return UART_STATUS_SUCCESS;
}


/* Public API Instance ----------------------------------------------------------------------------*/
/*
 * @brief Public RockBLOCK API Instance
 *
 */
const RockBLOCK_Interface_t RockBLOCK_API = {
    .init = RB_init,
    .get_signal_strength = RB_get_signal_strength,
    .send_message = RB_send_message,
    .send_long_message = RB_send__long_message,
    .receive_check = RB_receive_check,
    .waiting_message = RB_waiting_for_message,
    .wakeup = RB_WakeUp,
    .sleep = RB_Sleep
};


/**
 * @brief ISR UART2 function
 *
 * @note This function is used to get RockBLOCK9602 responses and messages to be proccessed
 *
 */
void rb_callback(uint8_t data)
{
    if(rb_dev.count < RB_BUFFER_SIZE) { //Circular buffer logic
        rb_dev.RBDataRaw[rb_dev.head] = data;
        rb_dev.head = (rb_dev.head + 1) % RB_BUFFER_SIZE;
        rb_dev.count++;
    }
    /*if(rb_dev.isatcommandsent == ATCOMMAND_NOSENT && rb_dev.count > 100)
    {
        rb_dev.count = 0;
    }*/
}





