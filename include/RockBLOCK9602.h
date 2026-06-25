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
    RB_AT_COMMANDS_FOR_CONFIGURATION_RECEIVED_SUCCESS,

    // AT+CSQ command responses
    RB_AT_CSQ_COMMAND_TIMEOUT,
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
    CSQ_LEVEL_5
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

/* Public Structures -------------------------------------------------------------------------------*/
typedef struct RB
{
    uint32_t int_status;    // Temporary variable to hold interrupt status
    uint8_t data;   // Temporary variable to hold incoming byte
    uint8_t RBDataRaw[RB_BUFFER_SIZE];  // Circular buffer for raw data
    uint16_t head;  // Index for writing new data
    uint16_t tail;  // Index for reading data
    uint16_t count; // Number of bytes currently in buffer
    RB_Response_t at_response_received;
    uint8_t tries;

    uint8_t mo_status;  // Mobile Originated status for AT+SBDIX response
    uint16_t momsn;     // Counter for MO (messages that you send), counter from 0 to 65,535 and returns to 0
    uint8_t mt_status;  // Mobile Terminated status for AT+SBDIX response
    uint16_t mtmsn;     // Counter for MT (messages that is received), counter from 0 to 65,535 and returns to 0
    uint16_t mt_length; // Is the lenght, in bytes, of message terminated received from GSS.
    uint8_t mt_queued;  // Is the message terminated quantity that are in queue in the GSS to be send to ISU.
}RB;


/********************************************************************************
 *  Function Prototypes
 ********************************************************************************/
void Configure_RB_GPIO(void);
void Configure_RB_UART(void);
RB_Response_t RB_Configuring_Commands(void);
int at_response_contains(const uint8_t *buff, const char *expected);
CSQ_Level_t atcsq_response_contains(const uint8_t *buff);
//RB_Response_t RB_Send_AT_Command(const uint8_t *buff, const char *expected, uint32_t timeout_ms);
RB_Response_t RB_Send_AT_Command(uint32_t timeout_ms);
RB_Response_t RB_Send_ATK0_Command(uint32_t timeout_ms);
RB_Response_t RB_Send_ATE0_Command(uint32_t timeout_ms);
RB_Response_t RB_RingIndicator_Pin(bool activate,uint32_t timeout_ms);
RB_Response_t RB_Send_ATCSQ_Command(uint32_t timeout_ms);
RB_Response_t RB_Send_AT_SBDD0_Command(uint32_t timeout_ms);
RB_Response_t RB_Send_AT_SBDWT_Command(const char *buff, uint32_t timeout_ms);
RB_Response_t RB_Send_AT_SBDIX_Command(uint8_t *mo_status, uint16_t *momsn, uint8_t *mt_status, uint16_t *mtmsn, uint16_t *mt_length, uint8_t *mt_queued, uint32_t timeout_ms);
void RockBLOCKWakeUp(void);
void RockBLOCKSleep(void);


// Shared global variable
extern RB rockblock;

// Strings de comandos AT
extern const char AT[];
extern const char ATE0[];
extern const char AT_K0[];
extern const char AT_CSQ[];
extern const char AT_SBDIX[];

#endif /* INCLUDE_ROCKBLOCK9602_H_ */
