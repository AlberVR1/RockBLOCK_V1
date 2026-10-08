/**
 * @file main.c
 * @author Alberto Vazquez
 *
 * @brief source file main, C Interface for the RockBLOCK9602 module, which provides functions to configure the module,
 * send AT commands and handle responses.
 *
 * @note RockBLOCK_V2
 * @note This source file contains the main system initialization for Main Clock, UART modules for communication with
 * the RockBLOCK9602 module and the Master MCU, GPIO pins for control and debugging, and the main loop to handle
 * the communication with the RockBLOCK9602 module.
 *
 * @note To send a message from ISU to GSS,
 *                                          the maximum size to MO (send): 340 bytes per message maximum
 *                                          the maximum size to MT (received): 270 bytes per message maximum
 *
 *
 * Alt+217 -> ┘    Alt+218 -> ┌    Alt+191 -> ┐    Alt+192-> └    Alt+196 -> ─    Alt+124 -> |    Alt+195-> ├    Alt+180 -> ┤
 *
 * ┌─────────────────────────────────┐
 * |        RockBLOCK9602 module     |  <- Your hardware
 * ├─────────────────────────────────┤
 * |        APLICATION (main.c)      |  <- Your api code
 * ├─────────────────────────────────┤
 * |      PUBLIC API (RockBLOCK_API) |  <- Simple and clean interface
 * ├─────────────────────────────────┤
 * |    IMPLEMENTATION (static       |
 * |   functions in RockBLOCK9602.c) |  <- hidden inner logic
 * ├─────────────────────────────────┤
 * |        HARDWARE (TM4C123)       |  <- MCU's registers
 * └─────────────────────────────────┘
 *
 *
 * MCU PINOUT
 * ┌─────────────────────────────────┐
 * |                                 |
 * |  TM4C123GXL LaunchPad           |
 * ├─────────────────────────────────┤
 * |                                 |
 * |                          PF2───>|  Ring Alert
 * |                          PF3───>|  Network Available
 * |                          PF4───>|  On/Off
 * |                                 |
 * |                          PD6───>|
 * |                          PD7───>|
 * └─────────────────────────────────┘
 *
 *
 *
 *
 *
 *
 *

 * @version 2.00
 * @date 2026-03-02
 */

/**
 * @addtogroup main
 * @{
 */


#include "pll.h"
#include "SysTick.h"
#include "power.h"
#include <RockBLOCK9602.h>
#include "mastercommunication.h"

/* Private functions prototypes---------------------------------------------------------------------*/
typedef enum {
    CONFIGURATION_STATUS_SUCCESS = 0,
    CONFIGURATION_STATUS_PLL_ERROR,
    CONFIGURATION_STATUS_SYSTICK_ERROR,
    CONFIGURATION_STATUS_UART1_ERROR,
    CONFIGURATION_STATUS_POWER_ERROR,
    CONFIGURATION_STATUS_ROCKBLOCK_ERROR
} Configuration_Status_t;

Configuration_Status_t configuration(void);
void RB_SendMessage(void);
void RB_RecieveMessage(void);
void RB_GetSignalQuality(void);
void ClearmainBuffer(void);

/* Private global variables-------------------------------------------------------------------------*/
uint32_t freqqq = 0;
uint32_t count = 0;

static bool receive_message = false;
static RB_Data_t rb_data;


const char MESSAGE_NO_SENT[] = "$RB_I,MESSAGENOSENT#CS";
const char MESSAGE_SENT[] = "$RB_I,MESSAGESENT#CS";
const char MESSAGE_SENT_AND_MESSAGE_RECEIVED[] = "$RB_I,MESSAGESENT_W_NM#CS";    // Message Sent and Message Received
const char MESSAGE_SENT_AND_MESSAGE_IN_QUEUE_PART1[] = "$RB_I,MESSAGESENT_W_";   // Message Sent With New Message In Queue Part1
const char MESSAGE_SENT_AND_MESSAGE_IN_QUEUE_PART2[] = "_MIQ#CS";   // Message Sent With New Message In Queue Part2

const char RB_TIMEOUT[] = "$RB_TIMEOUT#CS";
const char RB_COMMUNICATION_OK[] = "$RB_I_OK#CS";

const char MESSAGE_RECEIVED[] = "$RB_I,NEW_MESSAGE_RECEIVED#CS";    // When a Message was received
const char MESSAGE_RECEIVED_WITH_QUEUE[] = "$RB_I,NEW_MESSAGE_RECEIVED_W_Q#CS";    // When a Message was received and a message in queue exist
const char SESSION_FAILURE[] = "$RB_I,SESSION_FAILURE#CS";    // When a Message was received
const char NO_MESSAGES[] = "$RB_I,NO_MESSAGE_EXIST#CS";    // When a Message was received
const char SIGNAL_STATUS_OK[] = "$RB_I,STATUS_SIGNAL_OK#CS";
const char SIGNAL_STATUS_NO_SIGNAL[] = "$RB_I,STATUS_NO_SIGNAL#CS";
const char SIGNAL_STATUS_TIMEOUT[] = "$RB_I,RB_TIMEOUT#CS";
const char SIGNAL_STATUS_ERROR[] = "$RB_I,STATUS_ERROR#CS";
const char MCU_SLEEPING[] = "$RB_I,SLEEPING#CS";
const char MCU_READY[] = "$RB_I,READY#CS";

const char COMMAND_DOESNT_EXIST[] = "$RB_I,COMMAND UNKNOWED#CS";    // Command doesn't exist

main_commands_t main_command = WAIT_FOR_RBMESSAGE;
UART1_frame_to_send_t u1_frame_send = {
    .buffer_to_send = "$RB_I,WA,40.623663,-60.405777,91,1.04,25,45,844.5554,SIGNALOK#CS",
    //.buffer_to_receive = "This is a mesage number 1, Date: 1238 18 June 2026"
    .buffer_to_receive = NULL,
    .new_message = NO_MESSAGE_NEW
};

/**
 * @brief main function for the RockBLOCK9602 module application
 *
 *
 */
int main(void)
{
    Configuration_Status_t configuration_status;

    rb_data.RockBLOCK_Status = RB_STATUS_INITIALIZING;
    rb_data.MESSAGE_SENT = RB_STATUS_OK;
    rb_data.MESSAGE_RECEIVED = RB_STATUS_OK;
    rb_data.signal_qualiity = RB_STATUS_OK;
    configuration_status = configuration();
    if(configuration_status != CONFIGURATION_STATUS_SUCCESS &&
       configuration_status != CONFIGURATION_STATUS_ROCKBLOCK_ERROR) {
        rb_data.RockBLOCK_Status = RB_STATUS_HW_ERROR;
        while(true) {
            // Essential MCU or master communication setup failed.
        }
    }
    while(true)
    {
        if(count<1000)
        {
            count++;
        }
        // Check Main commands
        u1_frame_send.new_message = master_TM4.ReadStatus(&u1_frame_send.buffer_to_send);
        switch(u1_frame_send.new_message)
        {
            case NEW_MISSION:
                main_command = MASTER_NEW_MISSION;
                break;
            case CONTINUE_MISSION:
                main_command = MASTER_CONTINUE_MISSION;
                break;
            case HOLD_COMMUNICATION:
                main_command = MASTER_HOLD_COMMUNICATION_WITH_GCS;
                break;
            case RETREIVE_GLIDER:
                main_command = RETREIVE_GLIDER_WAIT;
                break;
            case WITOUT_ANSWER:
                main_command = WITOUT_ANSWER_GLIDER;
                break;
            case COMMAND_UNKNOWED:
                main_command = COMMAND_UNKNOWED_GLIDER;
                break;
            case TRANSFER_MESSAGE:
                master_TM4.SendStringMaster(u1_frame_send.buffer_to_receive, master_TM4.getstringsize(u1_frame_send.buffer_to_receive));
                break;
            case WAKEUP:
                main_command = WAKEUP_NOW;
                break;
            case DOWNLOAD_MESSAGE:
                main_command = DOWNLOAD_RB_MESSAGE;
                break;
            case GET_SIGNAL_QUALITY:
                main_command = GET_SIGNAL_QUALITY_NOW;
                break;
            case SLEEP_REQUEST:
                main_command = ENTER_SLEEP;
                break;
            default:
                main_command = WAIT_FOR_RBMESSAGE;
                break;
        }
// Main actions
        switch(main_command) 
        {
        case NO_ACTION:
            break;
        case MASTER_NEW_MISSION:
            RB_SendMessage();
            RockBLOCK_API.sleep();
            main_command = NO_ACTION;
            break;
        case MASTER_CONTINUE_MISSION:
            RB_SendMessage();
            RockBLOCK_API.sleep();
            main_command = NO_ACTION;
            break;
        case MASTER_HOLD_COMMUNICATION_WITH_GCS: // GCS (Ground Control Station) is the Interface
            // Do something to hold communication with GCS
            RB_SendMessage();
            break;
        case RETREIVE_GLIDER_WAIT:
            RB_SendMessage();
            RockBLOCK_API.sleep();
            break;
        case WITOUT_ANSWER_GLIDER:
            RB_SendMessage();
            break;
        case COMMAND_UNKNOWED_GLIDER:
            master_TM4.SendStringMaster(COMMAND_DOESNT_EXIST, 25);
            main_command = WAIT_FOR_RBMESSAGE;
            break;
        case WAIT_FOR_RBMESSAGE:
            rb_data.RockBLOCK_Status = RockBLOCK_API.waiting_message();
            if(rb_data.RockBLOCK_Status == RB_STATUS_MESSAGE_IN_QUEUE)
            {
                receive_message = true;
                main_command = NO_ACTION;
            }
            break;
        case DOWNLOAD_RB_MESSAGE:
            receive_message = true;
            break;
        case WAKEUP_NOW:
            RockBLOCK_API.wakeup();
            PLL_API.delayMs(20000);
            break;
        case GET_SIGNAL_QUALITY_NOW:
            RB_GetSignalQuality();
            break;
        case ENTER_SLEEP:
            /* Acknowledge the request before sleeping; UART1 RX wakes the core. */
            master_TM4.setSleepMode(true);
            master_TM4.SendStringMaster(MCU_SLEEPING, sizeof(MCU_SLEEPING) - 1U);
            Power_EnterSleep();
            if(master_TM4.consumeWakeEvent()) {
                master_TM4.SendStringMaster(MCU_READY, sizeof(MCU_READY) - 1U);
            }
            master_TM4.setSleepMode(false);
            main_command = WAIT_FOR_RBMESSAGE;
            break;
        }
        if(receive_message)
        {
            receive_message = false;
            RB_RecieveMessage();
        }
        //master_TM4.SendMaster(u1_frame_send.buffer_to_receive, 47);
        SYSTICK_API.delay_ms(1000);
    }
}


Configuration_Status_t configuration(void)
{

    PLL_Status_t statuspll = PLL_API.init(MHz40);
    // The PLL module initialized correctly
    if(statuspll != PLL_STATUS_SUCCESS) {
        return CONFIGURATION_STATUS_PLL_ERROR;
    }
    PLL_API.delayMs(200); // Short delay to ensure PLL is stable before proceeding


    SYSTICK_Status_t statussystick = SYSTICK_API.init(); // The SysTick module configured correctly

    if(statussystick != SYSTICK_STATUS_SUCCESS) {
        return CONFIGURATION_STATUS_SYSTICK_ERROR;
    }
    // Configure UART1 for MCU main communication
    UART_Status_t status_uart1 = master_TM4.mainconf();
    if(status_uart1 != UART_STATUS_SUCCESS) {
        return CONFIGURATION_STATUS_UART1_ERROR;
    }

    if(Power_ConfigureSleepWakeSources() != POWER_STATUS_SUCCESS) {
        return CONFIGURATION_STATUS_POWER_ERROR;
    }

    // Configure UART2, GPIOS for RockBLOCK
    rb_data.RockBLOCK_Status = RockBLOCK_API.init();
    if(rb_data.RockBLOCK_Status != RB_STATUS_OK) {
        rb_data.MESSAGE_SENT = rb_data.RockBLOCK_Status;
        rb_data.MESSAGE_RECEIVED = rb_data.RockBLOCK_Status;
        master_TM4.SendStringMaster(RB_TIMEOUT, 14);
        return CONFIGURATION_STATUS_ROCKBLOCK_ERROR;
    } else {
        master_TM4.SendStringMaster(RB_COMMUNICATION_OK, 9);
    }

    return CONFIGURATION_STATUS_SUCCESS;
}

void RB_SendMessage(void)
{
    // Send Message to GSS
    rb_data.RockBLOCK_Status = RB_STATUS_SENDING_MESSAGE;
    rb_data.MESSAGE_SENT = rb_data.RockBLOCK_Status;
    // THis function try send a message three times
    rb_data.RockBLOCK_Status = RockBLOCK_API.send_long_message(&rb_data,
                                                  (char*)u1_frame_send.buffer_to_send,
                                                  u1_frame_send.buffer_to_receive);
    switch(rb_data.RockBLOCK_Status) // Check message sent
    {
    case RB_STATUS_MESSAGE_SENT:
        // Wait MCU main actions
        master_TM4.SendStringMaster(MESSAGE_SENT, 20);
        break;
    case RB_STATUS_MESSAGE_NO_SENT:
        // no session, try move antenna other position
        master_TM4.SendStringMaster(MESSAGE_NO_SENT, 22);
        break;
    case RB_STATUS_MESSAGE_SENT_AND_MESSAGE_RECEIVED:
        master_TM4.SendStringMaster(MESSAGE_SENT_AND_MESSAGE_RECEIVED, 25);
        break;
        // Transmit message to MCU main
    case RB_STATUS_MESSAGE_SENT_AND_MESSAGE_RECEIVED_WITH_QUEUE:
        master_TM4.SendStringMaster(MESSAGE_SENT_AND_MESSAGE_IN_QUEUE_PART1, 20);
        master_TM4.SendByteMaster(rb_data.mt_queued+48);
        master_TM4.SendStringMaster(MESSAGE_SENT_AND_MESSAGE_IN_QUEUE_PART2, 7);
        // Transmit message to MCU main or check last message
        break;
    default:
        //Status AT Commands error, try again?
        master_TM4.SendStringMaster(RB_TIMEOUT, 14);
        break;
    }
    rb_data.MESSAGE_SENT = rb_data.RockBLOCK_Status;
    SYSTICK_API.delay_ms(2000);
    main_command = WAIT_FOR_RBMESSAGE;
}

void RB_RecieveMessage(void)
{
    // Receive message from GSS
    rb_data.RockBLOCK_Status = RB_STATUS_INQUIRING_MESSAGE;
    rb_data.MESSAGE_RECEIVED = rb_data.RockBLOCK_Status;
// THis function try check for message three times
    rb_data.RockBLOCK_Status = RockBLOCK_API.receive_check(&rb_data,
                                                  u1_frame_send.buffer_to_receive);
    switch(rb_data.RockBLOCK_Status)    // Check status message incoming
    {
    case RB_STATUS_MESSAGE_RECEIVED:
        master_TM4.SendStringMaster(MESSAGE_RECEIVED, 29);

        //Send_message_to_MCU();
        break;
    case RB_STATUS_MESSAGE_RECEIVED_WITH_QUEUE:
    master_TM4.SendStringMaster(MESSAGE_RECEIVED_WITH_QUEUE, 33);
        // Get newest message
        break;
    case RB_STATUS_MESSAGE_NO_EXIST:
        // No message exist
        master_TM4.SendStringMaster(NO_MESSAGES, 25);
        break;
    case RB_STATUS_SBD_SESSION_FAILURE:
        // Try depending MCU main
        master_TM4.SendStringMaster(SESSION_FAILURE, 24);
        break;
    case RB_STATUS_MESSAGE_NO_RECEIVED:
        // Status AT Commands error, try again?
        break;
    default:
        //Status AT Commands error, try again?
        break;
    }
    rb_data.MESSAGE_RECEIVED = rb_data.RockBLOCK_Status;
    SYSTICK_API.delay_ms(2000);
    main_command = WAIT_FOR_RBMESSAGE;
}

void RB_GetSignalQuality(void)
{
    rb_data.RockBLOCK_Status = RB_STATUS_GETTING_SIGNAL;
    rb_data.RockBLOCK_Status = RockBLOCK_API.get_signal_strength(&rb_data.signal_quality);

    switch(rb_data.RockBLOCK_Status)
    {
    case RB_STATUS_SIGNAL_OK:
        master_TM4.SendStringMaster(SIGNAL_STATUS_OK, sizeof(SIGNAL_STATUS_OK) - 1U);
        break;
    case RB_STATUS_NO_SIGNAL:
        master_TM4.SendStringMaster(SIGNAL_STATUS_NO_SIGNAL, sizeof(SIGNAL_STATUS_NO_SIGNAL) - 1U);
        break;
    case RB_STATUS_TIMEOUT:
        master_TM4.SendStringMaster(SIGNAL_STATUS_TIMEOUT, sizeof(SIGNAL_STATUS_TIMEOUT) - 1U);
        break;
    case RB_STATUS_ERROR:
    case RB_STATUS_WAKEUP_ERROR:
    default:
        master_TM4.SendStringMaster(SIGNAL_STATUS_ERROR, sizeof(SIGNAL_STATUS_ERROR) - 1U);
        break;
    }

    main_command = WAIT_FOR_RBMESSAGE;
}

void ClearmainBuffer(void)
{

}

