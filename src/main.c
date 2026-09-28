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
#include <RockBLOCK9602.h>
#include "mastercommunication.h"

/* Private functions prototypes---------------------------------------------------------------------*/
void configuration(void);
void RB_SendMessage(void);
void RB_RecieveMessage(void);
void ClearmainBuffer(void);

/* Private global variables-------------------------------------------------------------------------*/
uint32_t freqqq = 0;
uint32_t count = 0;

static bool receive_message = false;
static bool signal_quality = false;
static RB_Data_t rb_data;


const char MESSAGE_NO_SENT[] = "$RB_I,MESSAGENOSENT#CS";
const char MESSAGE_SENT[] = "$RB_I,MESSAGESENT#CS";
const char MESSAGE_SENT_AND_MESSAGE_RECEIVED[] = "$RB_I,MESSAGESENT_W_NM#CS";    // Message Sent and Message Received
const char MESSAGE_SENT_AND_MESSAGE_IN_QUEUE_PART1[] = "$RB_I,MESSAGESENT_W_";   // Message Sent With New Message In Queue Part1
const char MESSAGE_SENT_AND_MESSAGE_IN_QUEUE_PART2[] = "_MIQ#CS";   // Message Sent With New Message In Queue Part2

const char RB_TIMEOUT[] = "$RB_TIMEOUT#CS";
const char RB_COMMUNICATION_OK[] = "$RB_OK#CS";

const char MESSAGE_RECEIVED[] = "$RB_I,NEW_MESSAGE_RECEIVED#CS";    // When a Message was received
const char MESSAGE_RECEIVED_WITH_QUEUE[] = "$RB_I,NEW_MESSAGE_RECEIVED_W_Q#CS";    // When a Message was received and a message in queue exist
const char SESSION_FAILURE[] = "$RB_I,SESSION_FAILURE#CS";    // When a Message was received
const char NO_MESSAGES[] = "$RB_I,NO_MESSAGE_EXIST#CS";    // When a Message was received

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
    rb_data.RockBLOCK_Status = RB_STATUS_INITIALIZING;
    rb_data.MESSAGE_SENT = RB_STATUS_OK;
    rb_data.MESSAGE_RECEIVED = RB_STATUS_OK;
    configuration();
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
                break;
            case RETREIVE_GLIDER:
                break;
            case WITOUT_ANSWER:
                break;
            case COMMAND_UNKNOWED:
                break;
            case TRANSFER_MESSAGE:
                master_TM4.SendStringMaster(u1_frame_send.buffer_to_receive, master_TM4.getstringsize(u1_frame_send.buffer_to_receive));
                break;
            case WAKEUP:
                main_command = WAIT_FOR_RBMESSAGE;
                break;
            case DOWNLOAD_MESSAGE:
                main_command = DOWNLOAD_RB_MESSAGE;
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
        }
        if(receive_message)
        {
            receive_message = false;
            RB_RecieveMessage();
        }
// Send Message to GSS
        if(signal_quality) {
            rb_data.RockBLOCK_Status = RB_STATUS_GETTING_SIGNAL;
            rb_data.RockBLOCK_Status = RockBLOCK_API.get_signal_strength(&rb_data.signal_quality);
            signal_quality = false;
        }

        //master_TM4.SendMaster(u1_frame_send.buffer_to_receive, 47);
        SYSTICK_API.delay_ms(5000);
    }
}


void configuration(void)
{

    PLL_Status_t statuspll = PLL_API.init(MHz40);
    // The PLL module initialized correctly
    if(statuspll != PLL_STATUS_SUCCESS) {
    }
    PLL_API.delayMs(200); // Short delay to ensure PLL is stable before proceeding


    SYSTICK_Status_t statussystick = SYSTICK_API.init(); // The SysTick module configured correctly

    if(statussystick != SYSTICK_STATUS_SUCCESS) {
    }
    // Configure UART1 for MCU main communication
    master_TM4.mainconf();

    // Configure UART2, GPIOS for RockBLOCK
    rb_data.RockBLOCK_Status = RockBLOCK_API.init();
    if(rb_data.RockBLOCK_Status != RB_STATUS_OK) {
        rb_data.MESSAGE_SENT = rb_data.RockBLOCK_Status;
        rb_data.MESSAGE_RECEIVED = rb_data.RockBLOCK_Status;
        master_TM4.SendStringMaster(RB_TIMEOUT, 14);
    } else {
        master_TM4.SendStringMaster(RB_COMMUNICATION_OK, 9);
    }
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
        master_TM4.SendStringMaster(MESSAGE_SENT_AND_MESSAGE_RECEIVED, 24);
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

void ClearmainBuffer(void)
{

}

