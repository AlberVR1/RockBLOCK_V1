/*
 * RockBlock9602.h
 *
 *  Created on: 5 feb 2026
 *      Author: Control1
 */

#ifndef INCLUDE_ROCKBLOCK9602_H_
#define INCLUDE_ROCKBLOCK9602_H_

/* Includes --------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pll.h"
#include "gpio.h"
#include "UART.h"
#include "SysTick.h"
/* Definitions -------------------------------------------------------------------------------------*/
#define RB_BUFFER_SIZE 50
//#define AT "AT\r"
//#define ATE0 "ATE0\r"
//#define AT_K0 "AT&K0\r"

/* Public Enums ------------------------------------------------------------------------------------*/

typedef enum {
    AT_COMMAND_AT = 0,
    AT_COMMAND_ATE0,
    AT_COMMAND_AT_K0,
    AT_COMMAND_SEND,
    AT_COMMAND_NO_SEND
}AT_Commands_t;


typedef enum {
    RB_NO_AT_COMMAND_SENT = 0,
    RB_OK_RESPONSE_RECEIVED_SUCCESS,
    RB_ERROR_RESPONSE_RECEIVED,

    //Pin status
    RB_WAKEUP_ERROR,
    RB_WAKEUP_OK,
    RB_SLEEP_ERROR,
    RB_SLEEP_OK,
    RB_RI_ERROR,
    RB_NETAV_OK,
    RB_NO_NETAV,
    RB_NETAV_ERROR,


    // AT command responses for configuration
    RB_AT_COMMAND_RESPONSE_RECEIVED_SUCCESS,
    RB_AT_COMMAND_RESPONSE_RECEIVED_ERROR,
    RB_AT_COMMAND_RESPONSE_TIMEOUT,
    RB_AT_K0_COMMAND_RESPONSE_RECEIVED_SUCCESS,
    RB_AT_K0_COMMAND_RESPONSE_RECEIVED_ERROR,
    RB_AT_K0_COMMAND_RESPONSE_TIMEOUT,
    RB_ATE0_COMMAND_RESPONSE_RECEIVED_SUCCESS,
    RB_ATE0_COMMAND_RESPONSE_RECEIVED_ERROR,
    RB_ATE0_COMMAND_RESPONSE_TIMEOUT,
    RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_SUCCESS,
    RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_ERROR,
    RB_ATSBDMTA_COMMAND_RESPONSE_TIMEOUT,
    RB_ATW0_COMMAND_RESPONSE_RECEIVED_SUCCESS,
    RB_ATW0_COMMAND_RESPONSE_RECEIVED_ERROR,
    RB_ATW0_COMMAND_RESPONSE_TIMEOUT,
    RB_ATY0_COMMAND_RESPONSE_RECEIVED_SUCCESS,
    RB_ATY0_COMMAND_RESPONSE_RECEIVED_ERROR,
    RB_ATY0_COMMAND_RESPONSE_TIMEOUT,
    RB_AT_COMMANDS_FOR_CONFIGURATION_RECEIVED_SUCCESS,


    // AT+CSQ command responses
    RB_AT_CSQ_COMMAND_TIMEOUT,
    RB_AT_CSQ_COMMAND_ERROR,
    RB_AT_CSQ_NO_SIGNAL,
    RB_AT_CSQ_WEAK_SIGNAL,
    RB_AT_CSQ_MODERATE_SIGNAL,
    RB_AT_CSQ_GOOD_SIGNAL,
    RB_AT_CSQ_EXCELLENT_SIGNAL,
    RB_AT_CSQ_VERY_GOOD_SIGNAL,

    // AT+SBDD0 command responses
    RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_SUCCESS,
    RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_ERROR,
    RB_AT_SBDD0_COMMAND_RESPONSE_TIMEOUT,

    // AT+SBDWT command responses
    RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_SUCCESS,
    RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_ERROR,
    RB_AT_SBDWT_COMMAND_RESPONSE_TIMEOUT,

    // AT+SBDRT command responses
    RB_AT_SBDRT_COMMAND_RESPONSE_RECEIVED_SUCCESS,
    RB_AT_SBDRT_COMMAND_RESPONSE_RECEIVED_ERROR,
    RB_AT_SBDRT_COMMAND_RESPONSE_TIMEOUT,

    // AT+SBDIX command responses
    RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_SUCCESS,
    RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_ERROR,
    RB_AT_SBDIX_COMMAND_RESPONSE_TIMEOUT,
    RB_RESPONSE_TIMEOUT
}RB_Response_t;

typedef enum {
    CSQ_LEVEL_ERROR = 0,
    CSQ_LEVEL_0,
    CSQ_LEVEL_1,
    CSQ_LEVEL_2,
    CSQ_LEVEL_3,
    CSQ_LEVEL_4,
    CSQ_LEVEL_5,
    CSQ_TIMEOUT
}CSQ_Level_t;

typedef enum {
    RB_PROBE_NONE = 0,
    RB_WAKEUP,
    RB_SLEEP,
    RB_SEND_AT_COMMAND,
    RB_SEND_ATK0_COMMAND,
    RB_SEND_ATE0_COMMAND,
    RB_SEND_ATSBDMTA1_COMMAND,
    RB_SEND_ATSBDMTA0_COMMAND,
    RB_SEND_ATCSQ_COMMAND,
    RB_SEND_ATSBDD0_COMMAND,
    RB_SEND_ATSBDWT_COMMAND,
    RB_SEND_AT_SBDIX_COMMAND
}RB_Commands_t;

typedef enum {
    WAIT_ACTION = 0,
    SEND_MESSAGE,
    RECEIVE_MESSAGE,
    SEND_ERROR_TO_MCU
}RB_System_t;

typedef enum {
    RB_STATUS_OK = 0,      // Success
    RB_STATUS_WAKEUP,
    RB_STATUS_WAKEUP_ERROR,
    RB_STATUS_SLEEP,
    RB_STATUS_ERROR,       //Command error
    RB_STATUS_BUSY,
    RB_STATUS_TIMEOUT,
    RB_STATUS_HW_ERROR,
    RB_STATUS_GETTING_SIGNAL,
    RB_STATUS_SIGNAL_OK,
    RB_STATUS_NO_SIGNAL,
    RB_STATUS_INQUIRING_MESSAGE,
    RB_STATUS_MESSAGE_SENT,
    RB_STATUS_MESSAGE_NO_SENT,
    RB_STATUS_SENDING_MESSAGE,
    RB_STATUS_MESSAGE_RECEIVED,
    RB_STATUS_MESSAGE_NO_RECEIVED,
    RB_STATUS_MESSAGE_NO_EXIST,
    RB_STATUS_SBD_SESSION_FAILURE,
    RB_STATUS_MESSAGE_IN_QUEUE,
    RB_STATUS_NO_MESSAGE_IN_QUEUE,
    RB_STATUS_INVALID_PARAM,
    RB_STATUS_INITIALIZING
}RB_Status_t;

/* Public Structures -------------------------------------------------------------------------------*/
typedef struct {
    PLL_Frequency_t mcu_frequency;
    UART_BaudRate_t mastermcu_baudRate_communication;       ///< Baud rate for set communication with master mcu
    //bool is_initialized; // Init flag
}RB_Config_t;

typedef struct {
    uint8_t RBDataRaw[RB_BUFFER_SIZE];  // Circular buffer for raw data
    uint16_t head;  // Index for writing new data
    uint16_t tail;  // Index for reading data
    uint16_t count; // Number of bytes currently in buffer
    RB_Response_t at_response_received;
}ROCKBLOCK_Device_t;

typedef struct {
    uint8_t mo_status;  // Mobile Originated status for AT+SBDIX response
    uint16_t momsn;     // Counter for MO (messages that you send), counter from 0 to 65,535 and returns to 0
    uint8_t mt_status;  // Mobile Terminated status for AT+SBDIX response
    uint16_t mtmsn;     // Counter for MT (messages that is received), counter from 0 to 65,535 and returns to 0
    uint16_t mt_length; // Is the lenght, in bytes, of message terminated received from GSS.
    uint8_t mt_queued;  // Is the message terminated quantity that are in queue in the GSS to be send to ISU.
    uint8_t signal_quality; // Signal quality
    RB_Status_t RockBLOCK_Status;   // Status of the RockBLOCK9602 module
}RB_Data_t;

/* Public Function Pointers Structure --------------------------------------------------------------*/
/**
 * @brief RockBLOCK Interface structure containing all API functions
 */
typedef struct {
    RB_Status_t (*init)( const RB_Config_t *config);
    RB_Status_t (*get_signal_strength)(uint8_t *level);
    RB_Status_t (*send_message)(uint8_t *mo_status,
                                uint16_t *momsn,
                                uint8_t *mt_status,
                                uint16_t *mtmsn,
                                uint16_t *mt_length,
                                uint8_t *mt_queued,
                                const char *msg);
    RB_Status_t (*send_long_message)(uint8_t *mo_status,
                                uint16_t *momsn,
                                uint8_t *mt_status,
                                uint16_t *mtmsn,
                                uint16_t *mt_length,
                                uint8_t *mt_queued,
                                const char *msg);
    RB_Status_t (*receive_check)(uint8_t *mo_status,
                                uint16_t *momsn,
                                uint8_t *mt_status,
                                uint16_t *mtmsn,
                                uint16_t *mt_length,
                                uint8_t *mt_queued,
                                char *msg);
    RB_Status_t (*waiting_message)(void);
    RB_Status_t (*wakeup)(void);
    RB_Status_t (*sleep)(void);
}RockBLOCK_Interface_t;

/* Public API Instance -----------------------------------------------------------------------------*/
/**
 * @brief GPIO Public API Instance
 */
extern const RockBLOCK_Interface_t RockBLOCK_API;

#endif /* INCLUDE_ROCKBLOCK9602_H_ */
