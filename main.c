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
#include <RockBLOCK9602.h>
#include "mastercommands.h"

/* Private functions prototypes---------------------------------------------------------------------*/
void configuration(void);

/* Private global variables-------------------------------------------------------------------------*/
uint32_t freqqq = 0;

static bool send_message = false;
static bool receive_message = false;
static bool signal_quality = false;
static RB_Data_t rb_data;



main_commands_t main_command = WAIT_FOR_RBMESSAGE;
UART1_frame_to_send_t u1_frame_send = {
    .buffer_to_send = NULL,
    //.buffer_to_receive = "This is a mesage number 1, Date: 1238 18 June 2026"
    .buffer_to_receive = "Tres Anillos para los Reyes Elfos bajo el cielo Siete para los Senores Enanos en palacios de piedra. Nueve para los Hombres Mortales condenados a morir. Uno para el Senor Oscuro, sobre el trono oscuro en la Tierra de Mordor donde se extienden las Sombras. Un Anillo para gobernarlos a todos. Un Anillo para encontrarlos,  un Anillo para"
};

/**
 * @brief main function for the RockBLOCK9602 module application
 *
 *
 */
int main(void)
{
    rb_data.RockBLOCK_Status = RB_STATUS_INITIALIZING;
    configuration();
    freqqq = PLL_API.getPLLFrequency();
    while(true)
    {
// Main actions
        switch(main_command) 
        {
        case NO_ACTION:
            break;
        case MASTER_START_MISSION:
            RockBLOCK_API.sleep();
            main_command = NO_ACTION;
            break;
        case WAIT_FOR_RBMESSAGE:
            rb_data.RockBLOCK_Status = RockBLOCK_API.waiting_message();
            SYSTICK_API.delay_ms(5000);
            if(rb_data.RockBLOCK_Status == RB_STATUS_MESSAGE_IN_QUEUE)
            {
                receive_message = true;
                main_command = NO_ACTION;
            }
            break;
        case MASTER_SEND_MESSAGE:
            send_message = true;
            main_command = NO_ACTION;
            break;
        case MASTER_RECEIVE_MESSAGE:
            receive_message = true;
            main_command = NO_ACTION;
            break;
        }
// Send Message to GSS
        if(send_message) {
            rb_data.RockBLOCK_Status = RB_STATUS_SENDING_MESSAGE;
            // THis function try send a message three times
            rb_data.RockBLOCK_Status = RockBLOCK_API.send_long_message(&rb_data.mo_status,
                                                          &rb_data.momsn,
                                                          &rb_data.mt_status,
                                                          &rb_data.mtmsn,
                                                          &rb_data.mt_length,
                                                          &rb_data.mt_queued,
                                                          (char*)u1_frame_send.buffer_to_receive);
            switch(rb_data.RockBLOCK_Status) // Check message sent
            {
            case RB_STATUS_MESSAGE_SENT:
                // Wait MCU main actions
                break;
            case RB_STATUS_MESSAGE_NO_SENT:
                // no session, try move antenna other position
                break;
            default:
                //Status AT Commands error, try again?
                break;
            }
            send_message = false;
            SYSTICK_API.delay_ms(2000);
            send_message = false;
            main_command = WAIT_FOR_RBMESSAGE;
        }
// Receive message from GSS
        if(receive_message) {
            rb_data.RockBLOCK_Status = RB_STATUS_INQUIRING_MESSAGE;
        // THis function try check for message three times
            rb_data.RockBLOCK_Status = RockBLOCK_API.receive_check(&rb_data.mo_status,
                                                          &rb_data.momsn,
                                                          &rb_data.mt_status,
                                                          &rb_data.mtmsn,
                                                          &rb_data.mt_length,
                                                          &rb_data.mt_queued,
                                                          (char*)u1_frame_send.buffer_to_send);
            switch(rb_data.RockBLOCK_Status)    // Check status message incoming
            {
            case RB_STATUS_MESSAGE_RECEIVED:
                //Send_message_to_MCU();
                break;
            case RB_STATUS_MESSAGE_RECEIVED_WITH_QUEUE:
                // Get newest message
                break;
            case RB_STATUS_MESSAGE_NO_EXIST:
                // No message exist
                break;
            case RB_STATUS_SBD_SESSION_FAILURE:
                // Try depending MCU main
                break;
            case RB_STATUS_MESSAGE_NO_RECEIVED:
                // Status AT Commands error, try again?
                break;
            default:
                //Status AT Commands error, try again?
                break;
            }
            receive_message = false;
        }
        if(signal_quality) {
            rb_data.RockBLOCK_Status = RB_STATUS_GETTING_SIGNAL;
            rb_data.RockBLOCK_Status = RockBLOCK_API.get_signal_strength(&rb_data.signal_quality);
            signal_quality = false;
        }
    }

}


void configuration(void)
{
    RB_Config_t rb_config = {
        .mastermcu_baudRate_communication = UART_BAUD_115200,
        .mcu_frequency = MHz40
    };
    rb_data.RockBLOCK_Status = RockBLOCK_API.init(&rb_config);
    if(rb_data.RockBLOCK_Status != RB_STATUS_OK) {

    }
}







