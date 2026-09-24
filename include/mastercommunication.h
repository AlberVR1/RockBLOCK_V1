/*
 * mastercommunication.h
 *
 *  Created on: 7 ago 2026
 *      Author: Control1
 */

#ifndef INCLUDE_MASTERCOMMUNICATION_H_
#define INCLUDE_MASTERCOMMUNICATION_H_



#define MASTER_FRAME_SIZE 340U


typedef enum
{
    MASTER_NEW_MISSION = 0,
    MASTER_CONTINUE_MISSION  =1,
    MASTER_HOLD_COMMUNICATION_WITH_GCS = 2,
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
 * @brief
 *
 */
typedef enum
{
    NO_MESSAGE_NEW = 0,
    NEW_MESSAGE = 1,
    TRANSFER_MESSAGE = 2,
    CHECK_QUEUE_MESSAGES = 3
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
    uint32_t timeout_counter;
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

/**
 * @brief
 *
 */
typedef struct
{
    uint8_t buffer_to_send[MASTER_FRAME_SIZE];
    uint8_t buffer_to_receive[MASTER_FRAME_SIZE];
    Master_Status_t new_message;
}UART1_frame_to_send_t;



/**
 * @brief
 *
 */
typedef struct
{
    void (*mainconf)(void);
    void (*SendMaster)(const uint8_t *str, uint32_t str_len);
    Master_Status_t (*ReadStatus)(void);
}u1_master_t;

/**
 * @brief
 *
 */
extern const u1_master_t master_TM4;
#endif /* INCLUDE_MASTERCOMMUNICATION_H_ */
