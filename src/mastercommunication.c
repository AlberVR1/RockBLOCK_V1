/*
 * mastercommunication.c
 *
 *  Created on: 7 ago 2026
 *      Author: Control1
 */

#include "tm4c123gh6pm.h"
#include "functions.h"
#include "UART.h"
#include "mastercommunication.h"



static UART_Status_t Configure_UART_1(void);

void master_callback(uint8_t data);

static UART_Handle_t uart1_handle;

/* RockBlock and Master variables -------------------------------------------------------------------------------*/
u1_rx_context_master_t rx_cntxt_mstr = {
    .state = SYNC_0,
    .index = 0
};
UART1_frame_manager_t frame_mgr_mstr = {
    .buffer_a.complete = 0,
    .buffer_b.complete = 0,
    .p_write = &frame_mgr_mstr.buffer_a,
    .p_read = &frame_mgr_mstr.buffer_b,
    .frame_ready = 0,
    .frame_errors = 0
};



void uart1_main_configure(void)
{
    uint32_t i;
    for(i=0;i<MASTER_FRAME_SIZE;i++)
    {
        frame_mgr_mstr.buffer_a.frame[i] = '\0';
        frame_mgr_mstr.buffer_b.frame[i] = '\0';
    }

    // Configure UART 1
    UART_Status_t status_uart = Configure_UART_1();
    if(status_uart != UART_STATUS_SUCCESS) {
    }
}

/**
 * @brief configure UART 1 module
 *
 * This function initializes UART 1 for
 * communication with the Master MCU
 *
 * @param mcu_communication_master configures baudrate for UART
 */
static UART_Status_t Configure_UART_1(void)
{
    UART_Config_t uart1_config = {
       .module = UART_MODULE_1,
       .baudRate = UART_BAUD_115200,
       .clockFreqMHz = 40,
       .enableTx = true,
       .enableRx = true,
       .enableFIFO = false,
       .fifoLevel = UART_FIFO_LEVEL_1_8
    };
    // Initialize UART 1 module
    UART_Status_t statusuart1 = UART_API.init(&uart1_handle,
                                           &uart1_config);
    // The UART module initialized correctly
    if(statusuart1 != UART_STATUS_SUCCESS) {
        return statusuart1;
    }
    UART_API.enableInterrupt(&uart1_handle,UART_FIFO_LEVEL_1_8,master_callback);
    return UART_STATUS_SUCCESS;
}

/**
 * @brief
 *
 */
Master_Status_t Master_ReadStatus(void)
{
    Master_Status_t status;
    uint32_t i;
    uint8_t data;
    if(frame_mgr_mstr.p_read->frame[0] == 0x24)
    {
        data = frame_mgr_mstr.p_read->frame[1]; // 0x24 = $
        switch(data)
        {
        case 0x52:  // 0x52 = R
            status  = NEW_MESSAGE;
            break;
        case 0x54:  // 0x54 = T
            status = TRANSFER_MESSAGE;
            break;
        default:    // Command unauthorized
            break;
        }
    }
    else    // 0x00 = NULL
    {
        status = NO_MESSAGE_NEW;
    }
    for(i=0;i<MASTER_FRAME_SIZE;i++)
    {
        frame_mgr_mstr.buffer_a.frame[i] = '\0';
        frame_mgr_mstr.buffer_b.frame[i] = '\0';
    }

    return status;
}

/**
 * @brief Send string to mcu master
 *
 */
void SendtoMaster(const uint8_t *str, uint32_t str_len)
{
    UART_API.sendString(&uart1_handle, (const char*)str, str_len);
}

/**
 * @brief UART1 Callback function to get data from main mcu
 *
 * @note This function is used to get commands from MCU Master to send or read messages from RockBLOCK
 *
 */
void master_callback(uint8_t data)
{
    switch(rx_cntxt_mstr.state) {
        case SYNC_0:
            if(data == 0x24) // '$'
            {
                frame_mgr_mstr.p_write->frame[0] = data; // Store the first byte of the frame
                rx_cntxt_mstr.index = 1; // Move to the next index for the next byte
                rx_cntxt_mstr.state = SYNC_1; // Transition to the next state
            }
            break;
        case SYNC_1:
            // Handle SYNC_1 state
            if(data == 0x52)    // 'R'
            {
                frame_mgr_mstr.p_write->frame[1] = data; // Store the first byte of the frame
                rx_cntxt_mstr.index = 2; // Move to the next index for the next byte
                rx_cntxt_mstr.state = RX_PAYLOAD; // Transition to the next state
            } else {
                rx_cntxt_mstr.state = SYNC_0;
                if(data == 0x24) // '$'
                {
                    frame_mgr_mstr.p_write->frame[0] = data; // Store the first byte of the frame
                    rx_cntxt_mstr.index = 1; // Move to the next index for the next byte
                    rx_cntxt_mstr.state = SYNC_1; // Transition to the next state
                }
            }
            break;
        case RX_PAYLOAD:
            // Handle RX_PAYLOAD state
            frame_mgr_mstr.p_write->frame[rx_cntxt_mstr.index++] = data; // Store next bytes
            if(data == 0x23) // '#'
            {
                frame_mgr_mstr.p_write->complete = 1;   // Mark frame as complete
                frame_mgr_mstr.frame_ready = 1; //Set frame ready

                // Interchange buffers
                if(frame_mgr_mstr.p_write == &frame_mgr_mstr.buffer_a)
                {
                    frame_mgr_mstr.p_write = &frame_mgr_mstr.buffer_b;
                    frame_mgr_mstr.p_read = &frame_mgr_mstr.buffer_a;
                } else {
                    frame_mgr_mstr.p_write = &frame_mgr_mstr.buffer_a;
                    frame_mgr_mstr.p_read = &frame_mgr_mstr.buffer_b;
                }

                // Reset
                rx_cntxt_mstr.state = SYNC_0; // Transition to the next state
                rx_cntxt_mstr.index = 0; // Move to the next index for the next byte
                rx_cntxt_mstr.timeout_counter = 0;
            } else {
                rx_cntxt_mstr.timeout_counter++;
                if(rx_cntxt_mstr.timeout_counter>200)
                {
                    frame_mgr_mstr.frame_errors++;
                    rx_cntxt_mstr.state = SYNC_0; // Transition to the next state
                    rx_cntxt_mstr.index = 0; // Move to the next index for the next byte
                    rx_cntxt_mstr.timeout_counter = 0;

                }
            }
            break;
        default:
            rx_cntxt_mstr.state = SYNC_0; // Should never reach here, reset state just in case
            break;
    }
}

extern const u1_master_t master_TM4 = {
                          .mainconf = uart1_main_configure,
                          .SendMaster = SendtoMaster,
                          .ReadStatus = Master_ReadStatus,
};



