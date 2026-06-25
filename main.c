/**
 * @file main.c
 * @author Alberto Vazquez
 *
 * @brief source file main, C Interface for the RockBLOCK9602 module, which provides functions to configure the module,
 * send AT commands and handle responses.
 *
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
 * |        APLICATION (main.c)      |  <- Your api code
 * ├─────────────────────────────────┤
 * |       PUBLIC API  (PLL_API,     |
 * |                    UART_API,    |
 * |                    GPIO_API,    |
 * |                    SYSTICK_API) |  <- Simple and clean interface
 * ├─────────────────────────────────┤
 * |    IMPLEMENTATION (static       |  
 * |    functions in pll.c,          |
 * |                 gpio.c          |
 * |                 uart.c,         |
 * |                 systick.c)      |  <- hidden inner logic
 * ├─────────────────────────────────┤
 * |        HARDWARE (TM4C123)       |  <- MCU's registers
 * └─────────────────────────────────┘
 *
 * @version 1.00
 * @date 2026-03-02
 */

/**
 * @addtogroup main
 * @{
 */


#include <include/RockBLOCK9602.h>
#include "include/pll.h"
#include "include/gpio.h"
#include "include/UART.h"
#include "include/SysTick.h"
#include "include/tm4c123gh6pm.h"



/* Private variables--------------------------------------------------------------------------------*/
static PLL_Handle_t pll_handle;
static UART_Handle_t uart2_handle;
static UART_Handle_t uart1_handle;
// GPIO handles for RockBLOCK9602 control and debugging
static GPIO_Handle_t input_pf2;
static GPIO_Handle_t input_pf3;
static GPIO_Handle_t output_pf4;
// GPIO handle for debugging probe (PC4)
//static GPIO_Handle_t output_pc4;

/* Private functions prototypes---------------------------------------------------------------------*/
static void SystemConfig(void);
static void Configure_PLL(void);
static void Configure_UART_1(void);
static void Configure_UART_2(void);
/* Structure ---------------------------------------------------------------------------------------*/
RB rockblock;

/* Private global variables-------------------------------------------------------------------------*/
static bool send_message = false;
/* Char AT Command strings -------------------------------------------------------------------------*/
const char AT[] = "AT\r";   // Command to check if the module is responsive
const char ATE0[] = "ATE0\r";   // Command to disable echo
const char AT_K0[] = "AT&K0\r"; // Command to set flow control to none
const char AT_SBDMTA1[] = "AT+SBDMTA=1\r"; // Command to enable Mobile Terminated Alert (MTA) notifications
const char AT_SBDMTA0[] = "AT+SBDMTA=0\r"; // Command to disable Mobile Terminated Alert (MTA) notifications

const char AT_CSQ[] = "AT+CSQ\r";   // Command to check signal quality

const char AT_SBDD0[] = "AT+SBDD0\r";   // Command to clear the SBD buffer

                                        // -> M0 (Mobile Originated) buffer for outgoing messages
const char AT_SBDWT[] = "AT+SBDWT=";    // Command to write a message to the SBD buffer
                                        // -> M0 (Mobile Originated) buffer for outgoing messages
const char AT_SBDIX[] = "AT+SBDIX\r";   // Command to send messages from ISU to GSS and receive messages from GSS to ISU

/* Char AT Command responses------------------------------------------------------------------------*/
const char AT_RESPONSE_OK[] = "OK\r\n";
const char AT_RESPONSE_ERROR[] = "ERROR\r\n";
const char AT_RESPONSE_NO_SIGNAL[] = "\0\0\0\0\0";

//AT+CSQ response format: +CSQ: <rssi>,<ber>
const char AT_RESPONSE_CSQ_0[] = "+CSQ:0\r\n"; // Example response for no signal, where rssi is 0 and ber is 0
const char AT_RESPONSE_CSQ_1[] = "+CSQ:1\r\n"; // Example response for weak signal, where rssi is 1 and ber is 0
const char AT_RESPONSE_CSQ_2[] = "+CSQ:2\r\n"; // Example response for moderate signal, where rssi is 2 and ber is 0
const char AT_RESPONSE_CSQ_3[] = "+CSQ:3\r\n"; // Example response for good signal, where rssi is 3 and ber is 0
const char AT_RESPONSE_CSQ_4[] = "+CSQ:4\r\n"; // Example response for excellent signal, where rssi is 4 and ber is 0
const char AT_RESPONSE_CSQ_5[] = "+CSQ:5\r\n"; // Example response for very good signal, where rssi is 5 and ber is 0

//AT+SBDIX response format: +SBDIX: <MO status>,<MT status>,<SBD message number>,<MT message number>,<SBD message waiting indicator>
const char AT_RESPONSE_SBDIX_ECHO[] = "AT+SBDIX\r\r\n"; // Identify echo of the command to ignore it and wait for the real response
const char AT_RESPONSE_SBDIX[] = "+SBDIX:"; // Identify "+SBDIX:" in response to parse the rest of the response for MO status, MT status, SBD message number, MT message number and SBD message waiting indicator


/* Enums -------------------------------------------------------------------------------------------*/
UART_Status_t statusUART2 = UART_STATUS_SUCCESS;
AT_Commands_t AT_Commands = AT_COMMAND_NO_SEND;

RB_Commands_t RB_Commands = RB_PROBE_NONE;

RB_System_t RockBLOCKSystem = WAIT_ACTION;


/**
 * @brief 
 *
 *
 */
int main(void)
{
    SystemConfig();
    rockblock.tries = 0; //Variable to get count for tries to each command
    rockblock.at_response_received = RB_NO_AT_COMMAND_SENT;

    // Configure RockBLOCK9602 module
    rockblock.at_response_received = RB_Configuring_Commands();
    if (rockblock.at_response_received != RB_AT_COMMANDS_FOR_CONFIGURATION_RECEIVED_SUCCESS) {
        // Handle error (e.g., log it, retry, etc.)
    }

    while(true)
    {
        switch(RockBLOCKSystem) {
            case WAIT_ACTION:
                if(send_message) {
                    send_message = false;
                    rockblock.tries = 0;    // Reset command tries
                    RB_Commands = RB_WAKEUP; // change to RB_WAKEUP
                    RockBLOCKSystem = SEND_MESSAGE; //Send message
                }
            break;
            case SEND_MESSAGE:
                switch(RB_Commands) {
                    case RB_WAKEUP:
                        RockBLOCKWakeUp();
                        RB_Commands = RB_SEND_AT_COMMAND;
                        PLL_API.delayMs(15000);
                    break;
                    case RB_SEND_AT_COMMAND:    // Check if RockBLOCK is responding.
                        rockblock.at_response_received = RB_Send_AT_Command(900); // Wait for response with a timeout of 900 ms
                        if(rockblock.at_response_received == RB_AT_COMMAND_RESPONSE_RECEIVED_SUCCESS) { //Check if AT command was send succesfully
                            rockblock.tries = 0;
                            RB_Commands = RB_SEND_ATK0_COMMAND;
                        }
                        else {  //count tries to send command
                            rockblock.tries++;
                        }

                        if(rockblock.tries>5) { //Send error if command got a error
                            rockblock.tries = 0;
                            RockBLOCKSystem = SEND_ERROR_TO_MCU;
                        }
                        PLL_API.delayMs(500);
                    break;
                    case RB_SEND_ATK0_COMMAND:  //Disable Control flow
                        rockblock.at_response_received = RB_Send_ATK0_Command(900); // Wait for response with a timeout of 900 ms
                        if(rockblock.at_response_received == RB_AT_K0_COMMAND_RESPONSE_RECEIVED_SUCCESS) { //Check if ATK0 command was send succesfully
                            rockblock.tries = 0;
                            RB_Commands = RB_SEND_ATE0_COMMAND;
                        }
                        else {  //count tries to send command
                            rockblock.tries++;
                        }

                        if(rockblock.tries>5) { //Send error if command got a error
                            rockblock.tries = 0;
                            RockBLOCKSystem = SEND_ERROR_TO_MCU;
                        }
                        PLL_API.delayMs(500);
                    break;
                    case RB_SEND_ATE0_COMMAND:  // Disable echo
                        rockblock.at_response_received = RB_Send_ATE0_Command(900); // Wait for response with a timeout of 900 ms
                        if(rockblock.at_response_received == RB_ATE0_COMMAND_RESPONSE_RECEIVED_SUCCESS) { //Check if ATE0 command was send succesfully
                            rockblock.tries = 0;
                            RB_Commands = RB_SEND_ATCSQ_COMMAND;
                        }
                        else {  //count tries to send command
                            rockblock.tries++;
                        }

                        if(rockblock.tries>5) { //Send error if command got a error
                            rockblock.tries = 0;
                            RockBLOCKSystem = SEND_ERROR_TO_MCU;
                        }
                        PLL_API.delayMs(500);
                    break;
                    case RB_SEND_ATCSQ_COMMAND: // Check Quality signal
                        rockblock.at_response_received = RB_Send_ATCSQ_Command(20000); // Wait for response with a timeout of 20000 ms
                        if((rockblock.at_response_received >= RB_AT_CSQ_WEAK_SIGNAL) && (rockblock.at_response_received <= RB_AT_CSQ_VERY_GOOD_SIGNAL)) {   // Cheack if signal quality is good
                            rockblock.tries = 0;
                            RB_Commands = RB_SEND_ATSBDD0_COMMAND;
                        }
                        else {  //count tries to send command
                            rockblock.tries++;
                        }

                        if(rockblock.tries>5) { //Send error if command got a error
                            rockblock.tries = 0;
                            RockBLOCKSystem = SEND_ERROR_TO_MCU;
                        }
                        PLL_API.delayMs(500);
                    break;
                    case RB_SEND_ATSBDD0_COMMAND:   // Clear ISU buffer
                        rockblock.at_response_received = RB_Send_AT_SBDD0_Command(900); // Wait for response with a timeout of 900 ms
                        if(rockblock.at_response_received == RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_SUCCESS) { //Check if AT+SBDD0 command was send succesfully
                            rockblock.tries = 0;
                            RB_Commands = RB_SEND_ATSBDWT_COMMAND;
                        }
                        else {  //count tries to send command
                            rockblock.tries++;
                        }

                        if(rockblock.tries>5) { //Send error if command got a error
                            rockblock.tries = 0;
                            RockBLOCKSystem = SEND_ERROR_TO_MCU;
                        }
                        PLL_API.delayMs(500);
                    break;
                    case RB_SEND_ATSBDWT_COMMAND:   // Write on ISU buffer
                        rockblock.at_response_received = RB_Send_AT_SBDWT_Command("Test message: Hello World!", 900); // Wait for response with a timeout of 900 ms
                        if(rockblock.at_response_received == RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_SUCCESS) { //Check if AT+SBDWT command was send succesfully
                            rockblock.tries = 0;
                            RB_Commands = RB_SEND_AT_SBDIX_COMMAND;
                        }
                        else {  //count tries to send command
                            rockblock.tries++;
                        }

                        if(rockblock.tries>5) { //Send error if command got a error
                            rockblock.tries = 0;
                            RockBLOCKSystem = SEND_ERROR_TO_MCU;
                        }
                        PLL_API.delayMs(500);
                    break;
                    case RB_SEND_AT_SBDIX_COMMAND:  // Send message from ISU to GSS
                        rockblock.at_response_received = RB_Send_AT_SBDIX_Command(&rockblock.mo_status, &rockblock.momsn, &rockblock.mt_status,
                                                                            &rockblock.mtmsn, &rockblock.mt_length, &rockblock.mt_queued,
                                                                        40000); // Wait for response with a timeout of 40s
                        if(rockblock.at_response_received == RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_SUCCESS) { //Check if AT+SBDWT command was send succesfully
                            rockblock.tries = 0;
                            RB_Commands = RB_SEND_AT_COMMAND;
                            RockBLOCKSystem = WAIT_ACTION;
                        }
                        else {  //count tries to send command
                            rockblock.tries++;
                        }

                        if(rockblock.tries>5) { //Send error if command got a error
                            rockblock.tries = 0;
                            RockBLOCKSystem = SEND_ERROR_TO_MCU;
                        }
                        PLL_API.delayMs(500);
                    break;
                }
            break;
            case RECEIVE_MESSAGE:

            break;
            case SEND_ERROR_TO_MCU:
                if(RB_Commands == RB_SEND_ATCSQ_COMMAND && rockblock.at_response_received == RB_AT_CSQ_NO_SIGNAL) //Check if Signal Quality is low
                {
                    // Notify to MCU to adjust the antenna position
                }
                RockBLOCKSystem = WAIT_ACTION;
            break;
            default:
                RockBLOCKSystem = WAIT_ACTION;
            break;
        }
        /*switch(RB_Probe)
        {
            case RB_PROBE_NONE:
                break;
            case RB_WAKEUP:
                RockBLOCKWakeUp();
                RB_Probe = RB_PROBE_NONE;
                break;
            case RB_SLEEP:
                RockBLOCKSleep();
                RB_Probe = RB_PROBE_NONE;
                break;
            case RB_SEND_AT_COMMAND:
                rockblock.at_response_received = RB_Send_AT_Command(900); // Wait for response with a timeout of 900 ms
                RB_Probe = RB_PROBE_NONE;
                break;
            case RB_SEND_ATK0_COMMAND:
                rockblock.at_response_received = RB_Send_ATK0_Command(900); // Wait for response with a timeout of 900 ms
                RB_Probe = RB_PROBE_NONE;
                break;
            case RB_SEND_ATE0_COMMAND:
                rockblock.at_response_received = RB_Send_ATE0_Command(900); // Wait for response with a timeout of 900 ms
                RB_Probe = RB_PROBE_NONE;
                break;
            case RB_SEND_ATCSQ_COMMAND:
                rockblock.at_response_received = RB_Send_ATCSQ_Command(20000); // Wait for response with a timeout of 20000 ms
                RB_Probe = RB_PROBE_NONE;
                break;
            case RB_SEND_ATSBDD0_COMMAND:
                rockblock.at_response_received = RB_Send_AT_SBDD0_Command(900); // Wait for response with a timeout of 900 ms
                RB_Probe = RB_PROBE_NONE;
                break;
            case RB_SEND_ATSBDWT_COMMAND:
                rockblock.at_response_received = RB_Send_AT_SBDWT_Command("Test message", 900); // Wait for response with a timeout of 900 ms
                RB_Probe = RB_PROBE_NONE;
                break;
            case RB_SEND_ATSBDMTA1_COMMAND:
                rockblock.at_response_received = RB_RingIndicator_Pin(true, 900); // Wait for response with a timeout of 900 ms
                RB_Probe = RB_PROBE_NONE;
                break;
            case RB_SEND_ATSBDMTA0_COMMAND:
                rockblock.at_response_received = RB_RingIndicator_Pin(false, 900); // Wait for response with a timeout of 900 ms
                RB_Probe = RB_PROBE_NONE;
                break;
            case RB_SEND_AT_SBDIX_COMMAND:
            {
                rockblock.at_response_received = RB_Send_AT_SBDIX_Command(&rockblock.mo_status, &rockblock.momsn, &rockblock.mt_status,
                                                                    &rockblock.mtmsn, &rockblock.mt_length, &rockblock.mt_queued,
                                                                    20000); // Wait for response with a timeout of 20s
                RB_Probe = RB_PROBE_NONE;
            }
            default:
                RB_Probe = RB_PROBE_NONE;
                break;
        }*/
    }
}

/**
 * @brief configure the system peripherals for RockBLOCK9602 communication
 *
 * This function calls the functions to configures the PLL to 40 MHz,
 * UART 1 for communication with Master MCU,
 * UART 2 for communication with the RockBLOCK9602 module,
 * configures the SysTick timer for timing operations and
 * GPIO pins for RocBLOCK9602 control and debugging.
 */
static void SystemConfig(void)
{
    ///////////////////////////////////////////////////// Configure PLL to 40 MHz
    Configure_PLL();

    ///////////////////////////////////////////////////// Configure UART 1 module
    Configure_UART_1();

    ///////////////////////////////////////////////////// Configure UART 2 module
    Configure_UART_2();

    ////////////////////////////////////////////////////// Configure SysTick to generate an interrupt every 1ms
    SYSTICK_Status_t statussystick = SYSTICK_API.init(); // The SysTick module configured correctly
    
    if(statussystick != SYSTICK_STATUS_SUCCESS) {
        while(true) {};
    }

    Configure_RB_GPIO(); // Configure GPIO pins for RockBLOCK9602 control and debugging
}

/**
 * @brief configure PLL to the desired frequency
 *
 * This function initializes the PLL to the
 * specified frequency using the PLL API.
 */
static void Configure_PLL(void)
{
    PLL_Status_t statuspll = PLL_API.init(&pll_handle,
                                        MHz40);
    // The PLL module initialized correctly
    if(statuspll != PLL_STATUS_SUCCESS) {
        while(true) {};
    }

    PLL_API.delayMs(200); // Short delay to ensure PLL is stable before proceeding
}

/**
 * @brief configure UART 1 module
 * 
 * This function initializes UART 1 for
 * communication with the Master MCU
 */
static void Configure_UART_1(void)
{
    UART_Config_t uart1_config = {
       .module = UART_MODULE_1,
       .baudRate = UART_BAUD_115200,
       .clockFreqMHz = PLL_API.getPLLFrequency(),
       .enableTx = true,
       .enableRx = true,
       .enableFIFO = true,
       .fifoLevel = UART_FIFO_LEVEL_1_8
    };
    // Initialize UART 1 module
    UART_Status_t statusuart1 = UART_API.init(&uart1_handle,
                                           &uart1_config);
   // The UART module initialized correctly
   if(statusuart1 != UART_STATUS_SUCCESS) {
        while(true) {};
   }
   UART_API.enableInterrupt(&uart1_handle,UART_FIFO_LEVEL_1_8);
    PLL_API.delayMs(200); // Short delay to ensure UART is stable before proceeding
}

/**
 * @brief configure UART 2 module
 * 
 * This function initializes UART 2 for
 * communication with the RockBLOCK9602 module
 */
static void Configure_UART_2(void)
{
    UART_Config_t uart2_config = {
       .module = UART_MODULE_2,
       .baudRate = UART_BAUD_19200,
       .clockFreqMHz = PLL_API.getPLLFrequency(),
       .enableTx = true,
       .enableRx = true,
       .enableFIFO = false,
       .fifoLevel = UART_FIFO_LEVEL_1_8
    };
    // Initialize UART 2 module
    UART_Status_t statusuart2 = UART_API.init(&uart2_handle,
                                           &uart2_config);
    // The UART module initialized correctly
    if(statusuart2 != UART_STATUS_SUCCESS) {
        while(true) {};
    }
    UART_API.enableInterrupt(&uart2_handle,UART_FIFO_LEVEL_1_8);
    
    PLL_API.delayMs(200); // Short delay to ensure UART is stable before proceeding

}

/**
 * @brief configure GPIO pins for RockBLOCK9602 control and debugging
 *
 * GPIO_PF2 is configured as input for read the Ring Indicator (RI) signal from the RockBLOCK9602 module,
 * GPIO_PF3 is configured as input for read the Network Availability (NetAv) signal from the RockBLOCK9602 module and
 * GPIO_PF4 is configured as output for control the power of the RockBLOCK9602 module.
 */
void Configure_RB_GPIO(void)
{
    ////////////////////////////////////////////////////// Configure GPIO PF2 RI (Ring Indicator) pin from the RockBLOCK9602 module
    GPIO_Config_t pf2_config = {
        .mode = GPIO_MODE_INPUT,            // Input mode
        .pull = GPIO_PULL_NONE,             // None pull-up/down
        .drive_strength = GPIO_DRIVE_2MA,   // 2mA drive strength
        .interrupt_type = GPIO_INT_NONE,    // None interrupt
        .initial_state = GPIO_STATE_LOW,    // Initial state: low (not used for input mode)
        .slew_rate_control = false,         // None slew rate control
        .open_drain = false,                // Push-pull (no open drain)
        .alternate_function = 0             // No alternate function
    };

    GPIO_Status_t status_gpiopf2 = GPIO_API.Init(&input_pf2,
                                              GPIO_PORT_F,
                                              GPIO_PIN_2,
                                              &pf2_config);
    if(status_gpiopf2 != GPIO_STATUS_SUCCESS) {
        while(true);
    }
    
    PLL_API.delayMs(200); // Short delay to ensure GPIO is stable before proceeding
    ////////////////////////////////////////////////////// Configure GPIO PF3 Network Availability pin from the RockBLOCK9602 module
    GPIO_Config_t pf3_config = {
        .mode = GPIO_MODE_INPUT,            // Input mode
        .pull = GPIO_PULL_NONE,             // None pull-up/down
        .drive_strength = GPIO_DRIVE_2MA,   // 2mA drive strength
        .interrupt_type = GPIO_INT_NONE,    // None interrupt
        .initial_state = GPIO_STATE_LOW,    // Initial state: low (not used for input mode)
        .slew_rate_control = false,         // None slew rate control
        .open_drain = false,                // Push-pull (no open drain)
        .alternate_function = 0             // No alternate function
    };

    GPIO_Status_t status_gpiopf3 = GPIO_API.Init(&input_pf3,
                                              GPIO_PORT_F,
                                              GPIO_PIN_3,
                                              &pf3_config);
    if(status_gpiopf3 != GPIO_STATUS_SUCCESS) {
        while(true);
    }
    PLL_API.delayMs(200); // Short delay to ensure GPIO is stable before proceeding
    ////////////////////////////////////////////////////// Configure GPIO PF4 On/Off pin from the RockBLOCK9602 module
    GPIO_Config_t pf4_config = {
        .mode = GPIO_MODE_OUTPUT,           // Output mode
        .pull = GPIO_PULL_NONE,             // None pull-up/down
        .drive_strength = GPIO_DRIVE_2MA,   // 2mA drive strength
        .interrupt_type = GPIO_INT_NONE,    // None interrupt
        .initial_state = GPIO_STATE_LOW,   // Initial state: high (turn on the module)
        .slew_rate_control = false,         // None slew rate control
        .open_drain = false,                // Push-pull (no open drain)
        .alternate_function = 0             // No alternate function
    };

    GPIO_Status_t status_gpiopf4 = GPIO_API.Init(&output_pf4,
                                              GPIO_PORT_F,
                                              GPIO_PIN_4,
                                              &pf4_config);
    if(status_gpiopf4 != GPIO_STATUS_SUCCESS) {
        while(true);
    }
    PLL_API.delayMs(200); // Short delay to ensure GPIO is stable before proceeding
}

/**
 * @brief ISR UART1 function
 *
 * @note This function is used to get commands from MCU Master to send or read messages from RockBLOCK
 *
 */
void IntHandlerUART1(void)
{
    char c[10];
    int indx=0;
    if(UART1_MIS_R & UART_MIS_RXMIS) // Verifica si hay datos recibidos
    {
         c[indx] = UART1_DR_R; // Lee el dato recibido
         indx++;
        UART1_ICR_R = UART_ICR_RXIC; // Limpia la interrupciÃ³n de recepciÃ³n
    }
}

/**
 * @brief ISR UART2 function
 *
 * @note This function is used to get RockBLOCK9602 responses and messages to be proccessed
 *
 */
void IntHandlerUART2(void)
{
    rockblock.int_status = UART2_MIS_R;

    if(rockblock.int_status & UART_MIS_RXMIS) // Verifica si hay datos recibidos
    {
        while(!(UART2_FR_R & UART_FR_RXFE))
        {
            rockblock.data = UART2_DR_R;/// & 0xFF; // Lee el dato recibido

            if(rockblock.count < RB_BUFFER_SIZE) { //Circular buffer logic
                rockblock.RBDataRaw[rockblock.head] = rockblock.data;
                rockblock.head = (rockblock.head + 1) % RB_BUFFER_SIZE;
                rockblock.count++;
            }
        }
    }
    
    UART2_ICR_R = rockblock.int_status; // Limpia la interrupciÃ³n de recepciÃ³n
}


/*
 * @brief Configure RockBLOCK9602
 *
 * This function is responsible for configuring the RockBLOCK9602 module,
 * Send AT command to know if the module is working,
 * send ATE0 command to disable echo
 * and send AT&K0 to set the flow control to none.
 *
 * @return RB_Response_t indicating success or error
 */
RB_Response_t RB_Configuring_Commands(void)
{
    RB_Response_t response;

    // Send AT command to check if module is responsive
    response = RB_Send_AT_Command(900); // Wait for response with a timeout of 5000 ms
    
    if(response != RB_AT_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
        return RB_AT_COMMAND_RESPONSE_RECEIVED_ERROR; // Return error if expected response is not received
    }
    PLL_API.delayMs(1000); // Short delay before sending the next command

    // Send AT&K0 command to set flow control to none
    response = RB_Send_ATK0_Command(900); // Wait for response with a timeout of 5000 ms
    if(response != RB_AT_K0_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
        return RB_AT_K0_COMMAND_RESPONSE_RECEIVED_ERROR; // Return error if expected response is not received
    }
    PLL_API.delayMs(1000); // Short delay before sending the next command
    // Send ATE0 command to disable echo
    response = RB_Send_ATE0_Command(900); // Wait for response with a timeout of 5000 ms
    if(response != RB_ATE0_COMMAND_RESPONSE_RECEIVED_SUCCESS) {
        return RB_ATE0_COMMAND_RESPONSE_RECEIVED_ERROR; // Return error if expected response is not received
    }
    PLL_API.delayMs(1000); // Short delay before sending the next command
    return RB_AT_COMMANDS_FOR_CONFIGURATION_RECEIVED_SUCCESS; // Return success if all commands were sent without issues
}

/*
 * @brief compare the received response and get the expected response just for AT command
 *
 * This function compares the received response in the buffer with the expected response string.
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_AT_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_AT_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_AT_COMMAND_RESPONSE_TIMEOUT
 *
 */
RB_Response_t RB_Send_AT_Command(uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    int i = 0;
    // Send AT command to check if module is responsive
    for(i=0; i<rockblock.count; i++) {
        rockblock.RBDataRaw[i] = '\0'; // Clear buffer data
    }
    rockblock.head = rockblock.tail = rockblock.count = 0; // Reset circular buffer
    UART_API.sendString(&uart2_handle, AT,3);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_AT_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}

/*
 * @brief compare the received response and get the expected response just for AT&K0 command
 *
 * This function send AT&K0 command to set flow control to none and
 * compares the received response in the buffer with the expected response string.
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_AT_K0_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_AT_K0_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_AT_K0_COMMAND_RESPONSE_TIMEOUT
 *
 */

RB_Response_t RB_Send_ATK0_Command(uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    int i = 0;
    // Send AT command to check if module is responsive
    for(i=0; i<rockblock.count; i++) {
        rockblock.RBDataRaw[i] = '\0'; // Clear buffer data
    }
    rockblock.head = rockblock.tail = rockblock.count = 0; // Reset circular buffer
    UART_API.sendString(&uart2_handle, AT_K0,6);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_AT_K0_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_K0_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_K0_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}

/*
 * @brief compare the received response and get the expected response just for ATE0 command
 *
 * This function send ATE0 command to disable echo and compares the received response
 * in the buffer with the expected response string.
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_ATE0_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_ATE0_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_ATE0_COMMAND_RESPONSE_TIMEOUT
 *
 */
RB_Response_t RB_Send_ATE0_Command(uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    int i = 0;
    // Send AT command to check if module is responsive
    for(i=0; i<rockblock.count; i++) {
        rockblock.RBDataRaw[i] = '\0'; // Clear buffer data
    }
    rockblock.head = rockblock.tail = rockblock.count = 0; // Reset circular buffer
    UART_API.sendString(&uart2_handle, ATE0,5);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_ATE0_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_ATE0_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_ATE0_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}

/*
 * @brief compare the received response and get the expected response just for AT+SBDMTA=1 or AT+SBDMTA=0 commands
 *
 * This function sends AT+SBDMTA=1 if you want to use RI pin or AT+SBDMTA=0 if you want don't use RI pin
 * and compares the received response in the buffer with the expected response string.
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_ATSBDMTA_COMMAND_RESPONSE_TIMEOUT
 *
 */
RB_Response_t RB_RingIndicator_Pin(bool activate,uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    int i = 0;
    // Send AT command to check if module is responsive
    for(i=0; i<rockblock.count; i++) {
        rockblock.RBDataRaw[i] = '\0'; // Clear buffer data
    }
    rockblock.head = rockblock.tail = rockblock.count = 0; // Reset circular buffer
    if(activate) {
        UART_API.sendString(&uart2_handle, AT_SBDMTA1,12);
    }
    else {
        UART_API.sendString(&uart2_handle, AT_SBDMTA0,12);
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_ATSBDMTA_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_ATSBDMTA_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}

/*
 * @brief compare the received response and get the expected response just for AT+CSQ command
 *
 * This function send AT+CSQ command to check the signal quality and compares the received response
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_AT_CSQ_NO_SIGNAL, RB_AT_CSQ_WEAK_SIGNAL, RB_AT_CSQ_MODERATE_SIGNAL,
 * RB_AT_CSQ_GOOD_SIGNAL, RB_AT_CSQ_EXCELLENT_SIGNAL or RB_AT_CSQ_VERY_GOOD_SIGNAL
 *
 */
RB_Response_t RB_Send_ATCSQ_Command(uint32_t timeout_ms)
{
    CSQ_Level_t csq_level;
    uint32_t getmilis = 0;
    int i = 0;
    // Send AT command to check if module is responsive
    for(i=0; i<rockblock.count; i++) {
        rockblock.RBDataRaw[i] = '\0'; // Clear buffer data
    }
    rockblock.head = rockblock.tail = rockblock.count = 0; // Reset circular buffer
    UART_API.sendString(&uart2_handle, AT_CSQ,7);

    // Start a short timeout to wait for the response to be received and stored in the buffer
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < 1000) { // Short timeout to wait for the response to be received and stored in the buffer
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting

    // Start timeout counter
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms) {
        csq_level = atcsq_response_contains(rockblock.RBDataRaw);
        if(csq_level != CSQ_LEVEL_ERROR) {
            break;
        }
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting

    csq_level = atcsq_response_contains(rockblock.RBDataRaw);
    switch(csq_level) {
        case CSQ_LEVEL_0:
            return RB_AT_CSQ_NO_SIGNAL;
        case CSQ_LEVEL_1:
            return RB_AT_CSQ_WEAK_SIGNAL;
        case CSQ_LEVEL_2:
            return RB_AT_CSQ_MODERATE_SIGNAL;
        case CSQ_LEVEL_3:
            return RB_AT_CSQ_GOOD_SIGNAL;
        case CSQ_LEVEL_4:
            return RB_AT_CSQ_EXCELLENT_SIGNAL;
        case CSQ_LEVEL_5:
            return RB_AT_CSQ_VERY_GOOD_SIGNAL;
    }
    return RB_AT_CSQ_COMMAND_TIMEOUT; // Return timeout if expected response is not received within the specified time    
}

/*
 * @brief compare the received response and get the expected response just for AT+SBDD0 command
 *
 * This function send AT+SBDD0 command to clear the SBD buffer-> MO (Mobile Originated) buffer
 * for outgoing messages and compares the received response
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_AT_SBDD0_COMMAND_RESPONSE_TIMEOUT
 *
 */
RB_Response_t RB_Send_AT_SBDD0_Command(uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    int i = 0;
    // Send AT command to check if module is responsive
    for(i=0; i<rockblock.count; i++) {
        rockblock.RBDataRaw[i] = '\0'; // Clear buffer data
    }
    rockblock.head = rockblock.tail = rockblock.count = 0; // Reset circular buffer
    UART_API.sendString(&uart2_handle, AT_SBDD0, 9);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_SBDD0_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_SBDD0_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}

/*
 * @brief compare the received response and get the expected response just for AT+SBDWT command
 *
 * This function send AT+SBDWT command to write a message to the SBD buffer
 * -> MO (Mobile Originated) buffer
 *
 * @param timeout_ms Time in milliseconds to wait for the expected response
 *
 * @return RB_Response_t Returns RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_SUCCESS if the expected response is found in the buffer,
 * Returns RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_ERROR if an error response is found in the buffer,
 * otherwise returns RB_AT_SBDWT_COMMAND_RESPONSE_TIMEOUT
 *
 */
RB_Response_t RB_Send_AT_SBDWT_Command(const char *buff, uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    int i = 0;
    // Send AT command to check if module is responsive
    for(i=0; i<rockblock.count; i++) {
        rockblock.RBDataRaw[i] = '\0'; // Clear buffer data
    }
    rockblock.head = rockblock.tail = rockblock.count = 0; // Reset circular buffer
    UART_API.sendString(&uart2_handle, AT_SBDWT, 9);
    UART_API.sendString(&uart2_handle, buff, strlen((const char*)buff));
    UART_API.sendString(&uart2_handle, "\r", 1);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        getmilis = SYSTICK_API.milis();
    }
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_OK)) {
        return RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_SBDWT_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_SBDWT_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
}


RB_Response_t RB_Send_AT_SBDIX_Command(uint8_t *mo_status, uint16_t *momsn, uint8_t *mt_status, uint16_t *mtmsn, uint16_t *mt_length, uint8_t *mt_queued, uint32_t timeout_ms)
{
    uint32_t getmilis = 0;
    int i = 0;
    // Send AT command to check if module is responsive
    for(i=0; i<rockblock.count; i++) {
        rockblock.RBDataRaw[i] = '\0'; // Clear buffer data
    }
    rockblock.head = rockblock.tail = rockblock.count = 0; // Reset circular buffer
    UART_API.sendString(&uart2_handle, AT_SBDIX, 9);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    getmilis  = SYSTICK_API.milis();
    SYSTICK_API.Start_Count(); // Start SysTick to wait for response
    while(getmilis < timeout_ms)
    {
        if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_SBDIX)) {
            break;
        }
        getmilis = SYSTICK_API.milis();
    }
    PLL_API.delayMs(100);
    SYSTICK_API.Stop_Count(); // Stop SysTick after waiting
    if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_SBDIX))
    {
        // Parse the response to extract MO and MT status
        char *str = (char*)rockblock.RBDataRaw;
        char *pos = strtok(str, ":,");
        int pos_index = 0;
        while(pos != NULL) {
            if(pos_index == 1) {    // MO status
                *mo_status = (uint8_t)atoi(pos);
            }
            else if(pos_index == 2) {    //MO MSN
                *momsn = (uint16_t)atoi(pos);
            }
            else if(pos_index == 3) {   // MT status
                *mt_status = (uint8_t)atoi(pos);
            }
            else if(pos_index == 4) {   // MT MSN
                *mtmsn = (uint16_t)atoi(pos);
            }
            else if(pos_index == 5) {   // MT_Length
                *mt_length = (uint16_t)atoi(pos);
            }
            else if(pos_index == 6) {   // MT_Queued
                *mt_queued = (uint8_t)atoi(pos);
            }
            pos = strtok(NULL, ":,");
            pos_index++;
        }
        return RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_SBDIX_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }
    /*if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_OK)) {
        // Parse the response to extract MO and MT status
        char *response_str = (char*)rockblock.RBDataRaw;
        char *token = strtok(response_str, ":,");
        int token_index = 0;
        while(token != NULL) {
            if(token_index == 1) { // MO status
                *mo_status = (uint8_t)atoi(token);
            }
            else if(token_index == 2) { // MT status
                *mt_status = (uint8_t)atoi(token);
            }
            token = strtok(NULL, ":,");
            token_index++;
        }
        return RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_SUCCESS;
    }
    else if(at_response_contains(rockblock.RBDataRaw, AT_RESPONSE_ERROR)) {
        return RB_AT_SBDIX_COMMAND_RESPONSE_RECEIVED_ERROR;
    }
    else {
        return RB_AT_SBDIX_COMMAND_RESPONSE_TIMEOUT; // Return timeout if expected response is not received within the specified time
    }*/
}

/*
 * @brief compare the received response with the expected response
 *
 * This function compares the received response in the buffer with the expected response string.
 *
 * @param buff Pointer to the buffer containing the received response
 * @param expected Pointer to the expected response string
 * @return int Returns 1 if the expected response is found in the buffer, otherwise returns 0
 *
 */
int at_response_contains(const uint8_t *buff, const char *expected)
{
    return strstr((const char*)buff, expected) != NULL;
}


CSQ_Level_t atcsq_response_contains(const uint8_t *buff)
{
    if(strstr((const char*)buff, AT_RESPONSE_CSQ_0) != NULL) {
        return CSQ_LEVEL_0;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_1) != NULL) {
        return CSQ_LEVEL_1;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_2) != NULL) {
        return CSQ_LEVEL_2;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_3) != NULL) {
        return CSQ_LEVEL_3;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_4) != NULL) {
        return CSQ_LEVEL_4;
    }
    else if(strstr((const char*)buff, AT_RESPONSE_CSQ_5) != NULL) {
        return CSQ_LEVEL_5;
    }
    else {
        return CSQ_LEVEL_ERROR; // Return error if expected response is not found in the buffer
    }
}

/*
 * @brief Turn on the RockBLOCK9602 module
 *
 * This function sets the GPIO pin connected to the RockBLOCK9602
 * power control to high, turning on the module.
 *
 */
void RockBLOCKWakeUp(void)
{
    // Set PF4 high to wake up the RockBLOCK9602 module<
    GPIO_API.SetPin(&output_pf4);
}

/*
 * @brief Turn on the RockBLOCK9602 module
 *
 * This function sets the GPIO pin connected to the RockBLOCK9602
 * power control to high, turning on the module.
 *
 */
void RockBLOCKSleep(void)
{
    // Set PF4 low to put the RockBLOCK9602 module to sleep
    GPIO_API.ClearPin(&output_pf4);
}










