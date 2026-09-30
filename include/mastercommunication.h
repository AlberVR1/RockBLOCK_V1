/*
 * mastercommunication.h
 *
 *  Created on: 7 ago 2026
 *      Author: Control1
 */

#ifndef INCLUDE_MASTERCOMMUNICATION_H_
#define INCLUDE_MASTERCOMMUNICATION_H_

#include "UART.h"


#define MASTER_FRAME_SIZE 340U
#define MASTER_MAX_FRAME_SIZE 200U  // Includes the '$' start and '#' end markers.


typedef enum
{
    MASTER_NEW_MISSION = 0,
    MASTER_CONTINUE_MISSION  =1,
    MASTER_HOLD_COMMUNICATION_WITH_GCS = 2,
    RETREIVE_GLIDER_WAIT = 3,
    WITOUT_ANSWER_GLIDER = 4,
    COMMAND_UNKNOWED_GLIDER = 5,
    WAIT_FOR_RBMESSAGE = 6,
    DOWNLOAD_RB_MESSAGE = 7,
    WAKEUP_NOW = 8,
    GET_SIGNAL_QUALITY_NOW = 9,
    NO_ACTION = 10
}main_commands_t;

/**
 *  @brief Master Commands header file - Contains definitions and structures for master commands
 *
 * This enum is used to identify the frame received by master MCU
 *
**/
typedef enum
{
    SYNC_0 = 0,
    SYNC_1,
    RX_PAYLOAD,
    RX_DISCARD
}u1_rx_state_t;

/**
 * @brief
 *
 */
typedef enum
{
    NO_MESSAGE_NEW = 0,
    NEW_MISSION = 1,
    CONTINUE_MISSION = 2,
    HOLD_COMMUNICATION = 3,
    RETREIVE_GLIDER = 4,
    WITOUT_ANSWER = 5,
    COMMAND_UNKNOWED = 6,
    TRANSFER_MESSAGE = 7,
    WAKEUP = 8,
    DOWNLOAD_MESSAGE = 9,
    CHECK_QUEUE_MESSAGES = 10,
    GET_SIGNAL_QUALITY = 11
}Master_Status_t;

/**
 * @brief structure used to store the context of the reception of a command from master MCU
 *
 * This structure is used to store the state of the reception of a command from master MCU,
 * the index of the received byte and the payload of the command
 *
 **/
typedef struct {
    u1_rx_state_t state;
    uint16_t index;
    uint16_t frame_length;
} u1_rx_context_master_t;

/**
 * @brief structure to store the frame received from master MCU
 *
 *
 *
 **/
typedef struct
{
    uint8_t frame[MASTER_FRAME_SIZE];
    volatile uint8_t complete;
}u1_frame_buffer_t;

/**
 * @brief structure to manage the reception of frames from master MCU using a double buffer
 *
 *
 *
 *
 **/
typedef struct
{
    u1_frame_buffer_t buffer_a;
    u1_frame_buffer_t buffer_b;
    u1_frame_buffer_t * volatile p_write;
    u1_frame_buffer_t * volatile p_read;
    volatile uint8_t frame_ready;
    volatile uint32_t frame_errors;
}UART1_frame_manager_t;

/**
 * @brief
 *
 */
typedef struct
{
    uint8_t buffer_to_send[MASTER_FRAME_SIZE];
    uint8_t buffer_to_receive[MASTER_FRAME_SIZE];
    uint8_t buffer_to_send_master[MASTER_FRAME_SIZE];
    Master_Status_t new_message;
}UART1_frame_to_send_t;



/**
 * @brief
 *
 */
typedef struct
{
    UART_Status_t (*mainconf)(void);
    void (*SendStringMaster)(const char *str, uint32_t str_len);
    void (*SendByteMaster)(uint8_t data);
    Master_Status_t (*ReadStatus)(uint8_t *buffer);
    uint32_t (*getstringsize)(const uint8_t *str1);
}u1_master_t;

/**
 * @brief
 *
 */
extern const u1_master_t master_TM4;
#endif /* INCLUDE_MASTERCOMMUNICATION_H_ */
