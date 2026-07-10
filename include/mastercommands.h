/*
 * mastercommands.h
 *
 *  Created on: 17 jun 2026
 *      Author: Control1
 */

#ifndef INCLUDE_MASTERCOMMANDS_H_
#define INCLUDE_MASTERCOMMANDS_H_

#include <stdint.h>

#define MASTER_FRAME_SIZE 340U

typedef enum
{
    MASTER_START_MISSION = 0,
    WAIT_FOR_RBMESSAGE,
    MASTER_SEND_MESSAGE,
    MASTER_RECEIVE_MESSAGE,
    NO_ACTION
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
    RX_PAYLOAD
}u1_rx_state_t;

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
    u1_frame_buffer_t *p_write;
    u1_frame_buffer_t *p_read;
    volatile uint8_t frame_ready;
    volatile uint32_t frame_errors;
}UART1_frame_manager_t;

typedef struct
{
    uint8_t buffer_to_send[MASTER_FRAME_SIZE];
    uint8_t buffer_to_receive[MASTER_FRAME_SIZE];
}UART1_frame_to_send_t;



#endif /* INCLUDE_MASTERCOMMANDS_H_ */
