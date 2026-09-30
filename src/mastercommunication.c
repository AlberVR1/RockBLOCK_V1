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
#include <string.h>


const char NEWMISSION[] = "$RB_I,NM";
const char CONTINUEMISSION[] = "$RB_I,CM";
const char HOLDCOMMUNICATION[] = "$RB_I,HC";
const char RETRIEVEGLIDER[] = "$RB_I,RG";
const char WITOUTANSWER[] = "$RB_I,WA";
const char TRANSFERMESSAGE[] = "$TRANSFER_MESSAGE";
const char WAKEUP_RB[] = "$WAKEUP_RB";
const char DOWNLOAD_MESSAGE_RB[] = "$DOWNLOAD_MESSAGE";
const char GET_SIGNAL_COMMAND[] = "$GET_SIGNAL";

void master_callback(uint8_t data);

static UART_Handle_t uart1_handle;

/* RockBlock and Master variables -------------------------------------------------------------------------------*/
u1_rx_context_master_t rx_cntxt_mstr = {
    .state = SYNC_0,
    .index = 0,
    .frame_length = 0
};
UART1_frame_manager_t frame_mgr_mstr = {
    .buffer_a.complete = 0,
    .buffer_b.complete = 0,
    .p_write = &frame_mgr_mstr.buffer_a,
    .p_read = &frame_mgr_mstr.buffer_b,
    .frame_ready = 0,
    .frame_errors = 0
};


static int frame_starts_with_command(const uint8_t *buff, const char *expected);
static void master_start_frame(void);
static UART_Status_t Configure_UART_1(void);



UART_Status_t uart1_main_configure(void)
{
    uint32_t i;
    for(i=0;i<MASTER_FRAME_SIZE;i++)
    {
        frame_mgr_mstr.buffer_a. frame[i] = '\0';
        frame_mgr_mstr.buffer_b.frame[i] = '\0';
    }

    // Configure UART 1
    UART_Status_t status_uart = Configure_UART_1();
    return status_uart;
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
    statusuart1 = UART_API.enableInterrupt(&uart1_handle, UART_FIFO_LEVEL_1_8, master_callback);
    return statusuart1;
}

/**
 * @brief
 *
 */
Master_Status_t Master_ReadStatus(uint8_t *buffer)
{
    Master_Status_t status;
    uint32_t i;
    u1_frame_buffer_t *read_buffer = frame_mgr_mstr.p_read;
    if(read_buffer->frame[0] == '$')
    {
        if(frame_starts_with_command(read_buffer->frame, NEWMISSION))
        {
            status  = NEW_MISSION;
        }
        else if(frame_starts_with_command(read_buffer->frame, CONTINUEMISSION))
        {
            status = CONTINUE_MISSION;
        }
        else if(frame_starts_with_command(read_buffer->frame, HOLDCOMMUNICATION))
        {
            status = HOLD_COMMUNICATION;
        }
        else if(frame_starts_with_command(read_buffer->frame, RETRIEVEGLIDER))
        {
            status = RETREIVE_GLIDER;
        }
        else if(frame_starts_with_command(read_buffer->frame, WITOUTANSWER))
        {
            status = WITOUT_ANSWER;
        }
        else if(frame_starts_with_command(read_buffer->frame, TRANSFERMESSAGE))
        {
            status = TRANSFER_MESSAGE;
        }
        else if(frame_starts_with_command(read_buffer->frame, WAKEUP_RB))
        {
            status = WAKEUP;
        }
        else if(frame_starts_with_command(read_buffer->frame, DOWNLOAD_MESSAGE_RB))
        {
            status = DOWNLOAD_MESSAGE;
        }
        else if(frame_starts_with_command(read_buffer->frame, GET_SIGNAL_COMMAND))
        {
            status = GET_SIGNAL_QUALITY;
        }
        else
        {
            status = COMMAND_UNKNOWED;
        }
        memcpy(buffer, read_buffer->frame, MASTER_FRAME_SIZE);

        /*data = frame_mgr_mstr.p_read->frame[1]; // 0x24 = $
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
        }*/
    }
    else    // 0x00 = NULL
    {
        status = NO_MESSAGE_NEW;
    }
    for(i=0;i<MASTER_FRAME_SIZE;i++)
    {
        read_buffer->frame[i] = '\0';
    }
    read_buffer->complete = 0;

    return status;
}

/**
 * @brief Send string to mcu master
 *
 */
void SendStringtoMaster(const char *str, uint32_t str_len)
{
    UART_API.sendString(&uart1_handle, str, str_len);
}
/**
 * @brief
 *
 */
void SendByteToMaster(uint8_t data)
{
    UART_API.sendByte(&uart1_handle, data);
}

uint32_t getsizeofstring(const uint8_t *str1)
{
    uint32_t i;
    for(i=0;str1[i] != 0x00;i++);
    return i;
}

/*
 * @brief
 *
 */
static int frame_starts_with_command(const uint8_t *buff, const char *expected)
{
    size_t command_length = strlen(expected);
    if(strncmp((const char*)buff, expected, command_length) != 0) {
        return 0;
    }

    return buff[command_length] == ',' || buff[command_length] == '#';
}

static void master_start_frame(void)
{
    frame_mgr_mstr.p_write->frame[0] = '$';
    frame_mgr_mstr.p_write->complete = 0;
    rx_cntxt_mstr.index = 1;
    rx_cntxt_mstr.frame_length = 1;
    rx_cntxt_mstr.state = SYNC_1;
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
            if(data == '$')
            {
                master_start_frame();
            }
            break;
        case SYNC_1:
            if(data == 'D' || data == 'G' || data == 'R' || data == 'T' || data == 'W')
            {
                frame_mgr_mstr.p_write->frame[rx_cntxt_mstr.index++] = data;
                rx_cntxt_mstr.frame_length++;
                rx_cntxt_mstr.state = RX_PAYLOAD;
            } else {
                frame_mgr_mstr.frame_errors++;
                rx_cntxt_mstr.state = SYNC_0;
                rx_cntxt_mstr.index = 0;
                rx_cntxt_mstr.frame_length = 0;
                if(data == '$')
                {
                    master_start_frame();
                }
            }
            break;
        case RX_PAYLOAD:
            // Treat a new start marker as recovery from a truncated frame.
            if(data == '$') {
                frame_mgr_mstr.frame_errors++;
                master_start_frame();
                break;
            }

            if(rx_cntxt_mstr.frame_length >= MASTER_MAX_FRAME_SIZE) {
                frame_mgr_mstr.frame_errors++;
                rx_cntxt_mstr.index = 0;
                rx_cntxt_mstr.frame_length = 0;
                rx_cntxt_mstr.state = RX_DISCARD;
                break;
            }

            frame_mgr_mstr.p_write->frame[rx_cntxt_mstr.index++] = data;
            rx_cntxt_mstr.frame_length++;
            if(data == '#')
            {
                frame_mgr_mstr.p_write->frame[rx_cntxt_mstr.index] = '\0';
                frame_mgr_mstr.p_write->complete = 1;
                frame_mgr_mstr.frame_ready = 1;

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
                rx_cntxt_mstr.frame_length = 0;
            }
            break;
        case RX_DISCARD:
            if(data == '$') {
                master_start_frame();
            }
            break;
        default:
            rx_cntxt_mstr.state = SYNC_0; // Should never reach here, reset state just in case
            rx_cntxt_mstr.index = 0;
            rx_cntxt_mstr.frame_length = 0;
            break;
    }
}

extern const u1_master_t master_TM4 = {
                          .mainconf = uart1_main_configure,
                          .SendStringMaster = SendStringtoMaster,
                          .SendByteMaster = SendByteToMaster,
                          .ReadStatus = Master_ReadStatus,
                          .getstringsize = getsizeofstring,
};



