/**
 * @file gpio.c
 * @author Alberto Vazquez
 * @brief source file for managing GPIO peripheral
 *
 * This source file contains an API whitch includes and implementation functions
 * for managing TM4C123 GPIO peripheral.
 *
 * @version 0.11
 * @date 2025-09-17
 */

/* Includes ----------------------------------------------------------------------------------------*/

#include "include/gpio.h"
//#include "include/tm4c123gh6pm.h"

/* Private Defines --------------------------------------------------------------------------------*/
#ifndef NULL
#define NULL 0
#endif


// SYSCTL
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
//*****************************************************************************
//
// GPIO registers (PORTA)
//
//*****************************************************************************
#define GPIO_PORTA_DATA_R       (*((volatile uint32_t *)0x400043FC))
#define GPIO_PORTA_DIR_R        (*((volatile uint32_t *)0x40004400))
#define GPIO_PORTA_IS_R         (*((volatile uint32_t *)0x40004404))
#define GPIO_PORTA_IBE_R        (*((volatile uint32_t *)0x40004408))
#define GPIO_PORTA_IEV_R        (*((volatile uint32_t *)0x4000440C))
#define GPIO_PORTA_IM_R         (*((volatile uint32_t *)0x40004410))
#define GPIO_PORTA_ICR_R        (*((volatile uint32_t *)0x4000441C))
#define GPIO_PORTA_AFSEL_R      (*((volatile uint32_t *)0x40004420))
#define GPIO_PORTA_DR2R_R       (*((volatile uint32_t *)0x40004500))
#define GPIO_PORTA_DR4R_R       (*((volatile uint32_t *)0x40004504))
#define GPIO_PORTA_DR8R_R       (*((volatile uint32_t *)0x40004508))
#define GPIO_PORTA_ODR_R        (*((volatile uint32_t *)0x4000450C))
#define GPIO_PORTA_PUR_R        (*((volatile uint32_t *)0x40004510))
#define GPIO_PORTA_PDR_R        (*((volatile uint32_t *)0x40004514))
#define GPIO_PORTA_SLR_R        (*((volatile uint32_t *)0x40004518))
#define GPIO_PORTA_DEN_R        (*((volatile uint32_t *)0x4000451C))
#define GPIO_PORTA_LOCK_R       (*((volatile uint32_t *)0x40004520))
#define GPIO_PORTA_CR_R         (*((volatile uint32_t *)0x40004524))
#define GPIO_PORTA_AMSEL_R      (*((volatile uint32_t *)0x40004528))
#define GPIO_PORTA_PCTL_R       (*((volatile uint32_t *)0x4000452C))
//*****************************************************************************
//
// GPIO registers (PORTB)
//
//*****************************************************************************
#define GPIO_PORTB_DATA_R       (*((volatile uint32_t *)0x400053FC))
#define GPIO_PORTB_DIR_R        (*((volatile uint32_t *)0x40005400))
#define GPIO_PORTB_IS_R         (*((volatile uint32_t *)0x40005404))
#define GPIO_PORTB_IBE_R        (*((volatile uint32_t *)0x40005408))
#define GPIO_PORTB_IEV_R        (*((volatile uint32_t *)0x4000540C))
#define GPIO_PORTB_IM_R         (*((volatile uint32_t *)0x40005410))
#define GPIO_PORTB_MIS_R        (*((volatile uint32_t *)0x40005418))
#define GPIO_PORTB_ICR_R        (*((volatile uint32_t *)0x4000541C))
#define GPIO_PORTB_AFSEL_R      (*((volatile uint32_t *)0x40005420))
#define GPIO_PORTB_DR2R_R       (*((volatile uint32_t *)0x40005500))
#define GPIO_PORTB_DR4R_R       (*((volatile uint32_t *)0x40005504))
#define GPIO_PORTB_DR8R_R       (*((volatile uint32_t *)0x40005508))
#define GPIO_PORTB_PUR_R        (*((volatile uint32_t *)0x40005510))
#define GPIO_PORTB_PDR_R        (*((volatile uint32_t *)0x40005514))
#define GPIO_PORTB_DEN_R        (*((volatile uint32_t *)0x4000551C))
#define GPIO_PORTB_LOCK_R       (*((volatile uint32_t *)0x40005520))
#define GPIO_PORTB_CR_R         (*((volatile uint32_t *)0x40005524))
#define GPIO_PORTB_AMSEL_R      (*((volatile uint32_t *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile uint32_t *)0x4000552C))
//*****************************************************************************
//
// GPIO registers (PORTC)
//
//*****************************************************************************
#define GPIO_PORTC_DATA_R       (*((volatile uint32_t *)0x400063FC))
#define GPIO_PORTC_DIR_R        (*((volatile uint32_t *)0x40006400))
#define GPIO_PORTC_IS_R         (*((volatile uint32_t *)0x40006404))
#define GPIO_PORTC_IBE_R        (*((volatile uint32_t *)0x40006408))
#define GPIO_PORTC_IEV_R        (*((volatile uint32_t *)0x4000640C))
#define GPIO_PORTC_IM_R         (*((volatile uint32_t *)0x40006410))
#define GPIO_PORTC_ICR_R        (*((volatile uint32_t *)0x4000641C))
#define GPIO_PORTC_AFSEL_R      (*((volatile uint32_t *)0x40006420))
#define GPIO_PORTC_DR2R_R       (*((volatile uint32_t *)0x40006500))
#define GPIO_PORTC_DR4R_R       (*((volatile uint32_t *)0x40006504))
#define GPIO_PORTC_DR8R_R       (*((volatile uint32_t *)0x40006508))
#define GPIO_PORTC_ODR_R        (*((volatile uint32_t *)0x4000650C))
#define GPIO_PORTC_PUR_R        (*((volatile uint32_t *)0x40006510))
#define GPIO_PORTC_PDR_R        (*((volatile uint32_t *)0x40006514))
#define GPIO_PORTC_SLR_R        (*((volatile uint32_t *)0x40006518))
#define GPIO_PORTC_DEN_R        (*((volatile uint32_t *)0x4000651C))
#define GPIO_PORTC_LOCK_R       (*((volatile uint32_t *)0x40006520))
#define GPIO_PORTC_CR_R         (*((volatile uint32_t *)0x40006524))
#define GPIO_PORTC_AMSEL_R      (*((volatile uint32_t *)0x40006528))
#define GPIO_PORTC_PCTL_R       (*((volatile uint32_t *)0x4000652C))
//*****************************************************************************
//
// GPIO registers (PORTD)
//
//*****************************************************************************
#define GPIO_PORTD_DATA_R       (*((volatile uint32_t *)0x400073FC))
#define GPIO_PORTD_DIR_R        (*((volatile uint32_t *)0x40007400))
#define GPIO_PORTD_IS_R         (*((volatile uint32_t *)0x40007404))
#define GPIO_PORTD_IBE_R        (*((volatile uint32_t *)0x40007408))
#define GPIO_PORTD_IEV_R        (*((volatile uint32_t *)0x4000740C))
#define GPIO_PORTD_IM_R         (*((volatile uint32_t *)0x40007410))
#define GPIO_PORTD_ICR_R        (*((volatile uint32_t *)0x4000741C))
#define GPIO_PORTD_AFSEL_R      (*((volatile uint32_t *)0x40007420))
#define GPIO_PORTD_DR2R_R       (*((volatile uint32_t *)0x40007500))
#define GPIO_PORTD_DR4R_R       (*((volatile uint32_t *)0x40007504))
#define GPIO_PORTD_DR8R_R       (*((volatile uint32_t *)0x40007508))
#define GPIO_PORTD_ODR_R        (*((volatile uint32_t *)0x4000750C))
#define GPIO_PORTD_PUR_R        (*((volatile uint32_t *)0x40007510))
#define GPIO_PORTD_PDR_R        (*((volatile uint32_t *)0x40007514))
#define GPIO_PORTD_SLR_R        (*((volatile uint32_t *)0x40007518))
#define GPIO_PORTD_DEN_R        (*((volatile uint32_t *)0x4000751C))
#define GPIO_PORTD_LOCK_R       (*((volatile uint32_t *)0x40007520))
#define GPIO_PORTD_CR_R         (*((volatile uint32_t *)0x40007524))
#define GPIO_PORTD_AMSEL_R      (*((volatile uint32_t *)0x40007528))
#define GPIO_PORTD_PCTL_R       (*((volatile uint32_t *)0x4000752C))
//*****************************************************************************
//
// GPIO registers (PORTE)
//
//*****************************************************************************
#define GPIO_PORTE_DATA_R       (*((volatile uint32_t *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile uint32_t *)0x40024400))
#define GPIO_PORTE_IS_R         (*((volatile uint32_t *)0x40024404))
#define GPIO_PORTE_IBE_R        (*((volatile uint32_t *)0x40024408))
#define GPIO_PORTE_IEV_R        (*((volatile uint32_t *)0x4002440C))
#define GPIO_PORTE_IM_R         (*((volatile uint32_t *)0x40024410))
#define GPIO_PORTE_ICR_R        (*((volatile uint32_t *)0x4002441C))
#define GPIO_PORTE_AFSEL_R      (*((volatile uint32_t *)0x40024420))
#define GPIO_PORTE_DR2R_R       (*((volatile uint32_t *)0x40024500))
#define GPIO_PORTE_DR4R_R       (*((volatile uint32_t *)0x40024504))
#define GPIO_PORTE_DR8R_R       (*((volatile uint32_t *)0x40024508))
#define GPIO_PORTE_ODR_R        (*((volatile uint32_t *)0x4002450C))
#define GPIO_PORTE_PUR_R        (*((volatile uint32_t *)0x40024510))
#define GPIO_PORTE_PDR_R        (*((volatile uint32_t *)0x40024514))
#define GPIO_PORTE_SLR_R        (*((volatile uint32_t *)0x40024518))
#define GPIO_PORTE_DEN_R        (*((volatile uint32_t *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile uint32_t *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile uint32_t *)0x4002452C))
//*****************************************************************************
//
// GPIO registers (PORTF)
//
//*****************************************************************************
#define GPIO_PORTF_DATA_R       (*((volatile uint32_t *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile uint32_t *)0x40025400))
#define GPIO_PORTF_IS_R         (*((volatile uint32_t *)0x40025404))
#define GPIO_PORTF_IBE_R        (*((volatile uint32_t *)0x40025408))
#define GPIO_PORTF_IEV_R        (*((volatile uint32_t *)0x4002540C))
#define GPIO_PORTF_IM_R         (*((volatile uint32_t *)0x40025410))
#define GPIO_PORTF_ICR_R        (*((volatile uint32_t *)0x4002541C))
#define GPIO_PORTF_AFSEL_R      (*((volatile uint32_t *)0x40025420))
#define GPIO_PORTF_DR2R_R       (*((volatile uint32_t *)0x40025500))
#define GPIO_PORTF_DR4R_R       (*((volatile uint32_t *)0x40025504))
#define GPIO_PORTF_DR8R_R       (*((volatile uint32_t *)0x40025508))
#define GPIO_PORTF_ODR_R        (*((volatile uint32_t *)0x4002550C))
#define GPIO_PORTF_PUR_R        (*((volatile uint32_t *)0x40025510))
#define GPIO_PORTF_PDR_R        (*((volatile uint32_t *)0x40025514))
#define GPIO_PORTF_SLR_R        (*((volatile uint32_t *)0x40025518))
#define GPIO_PORTF_DEN_R        (*((volatile uint32_t *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile uint32_t *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile uint32_t *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile uint32_t *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile uint32_t *)0x4002552C))
//*****************************************************************************
//
// NVIC registers (NVIC)
//
//*****************************************************************************
#define NVIC_EN0_R              (*((volatile uint32_t *)0xE000E100))
//*****************************************************************************
//
// The following are defines for the bit fields in the SYSCTL_RCGCGPIO
// register.
//
//*****************************************************************************
#define SYSCTL_RCGCGPIO_R5      0x00000020  // GPIO Port F Run Mode Clock
                                            // Gating Control
#define SYSCTL_RCGCGPIO_R4      0x00000010  // GPIO Port E Run Mode Clock
                                            // Gating Control
#define SYSCTL_RCGCGPIO_R3      0x00000008  // GPIO Port D Run Mode Clock
                                            // Gating Control
#define SYSCTL_RCGCGPIO_R2      0x00000004  // GPIO Port C Run Mode Clock
                                            // Gating Control
#define SYSCTL_RCGCGPIO_R1      0x00000002  // GPIO Port B Run Mode Clock
                                            // Gating Control
#define SYSCTL_RCGCGPIO_R0      0x00000001  // GPIO Port A Run Mode Clock
                                            // Gating Control
//*****************************************************************************
//
// The following are defines for the bit fields in the SYSCTL_PRGPIO register.
//
//*****************************************************************************
#define SYSCTL_PRGPIO_R5        0x00000020  // GPIO Port F Peripheral Ready
#define SYSCTL_PRGPIO_R4        0x00000010  // GPIO Port E Peripheral Ready
#define SYSCTL_PRGPIO_R3        0x00000008  // GPIO Port D Peripheral Ready
#define SYSCTL_PRGPIO_R2        0x00000004  // GPIO Port C Peripheral Ready
#define SYSCTL_PRGPIO_R1        0x00000002  // GPIO Port B Peripheral Ready
#define SYSCTL_PRGPIO_R0        0x00000001  // GPIO Port A Peripheral Ready



// NVIC Interrupt Numbers for GPIO Ports
#define INTT_GPIOA          0    // GPIO Port A
#define INTT_GPIOB          1    // GPIO Port B
#define INTT_GPIOC          2    // GPIO Port C
#define INTT_GPIOD          3    // GPIO Port D
#define INTT_GPIOE          4    // GPIO Port E
#define INTT_GPIOF          30   // GPIO Port F

//GPIO Lock and Commit values
#define GPIO_LOCK_KEY           0x4C4F434B  // Unlocks the GPIO_CR register
#define GPIO_CR_PIN_ALL        0xFF        // Allow changes to all pins


/* Private Functions ------------------------------------------------------------------------------*/
static inline bool GPIO_IsValidHandle(const GPIO_Handle_t* handle);
static inline bool GPIO_IsValidPort(GPIO_Port_t port);
static inline bool GPIO_IsValidPin(GPIO_Pin_t pin);
static inline bool GPIO_IsValidConfig(const GPIO_Config_t* config);
static inline uint32_t GPIO_GetPinMask(GPIO_Pin_t pin);
static GPIO_Status_t GPIO_EnablePortClock(GPIO_Port_t port);
static GPIO_Status_t GPIO_ConfigureHardware(GPIO_Handle_t* handle);
static GPIO_Status_t GPIO_UnlockPin(GPIO_Handle_t* handle);

/* Private Variables ------------------------------------------------------------------------------*/
static GPIO_Handle_t* gpio_interrupt_handles[GPIO_PORT_COUNT][GPIO_PIN_COUNT];

/* Private Implementation -------------------------------------------------------------------------*/

/*
 * @brief Initialize GPIO Pin
 *
 */
static GPIO_Status_t GPIO_Init(GPIO_Handle_t* handle, GPIO_Port_t port, GPIO_Pin_t pin, const GPIO_Config_t* config)
{
    // Validate parameters
    if(!handle || !config) {
        return GPIO_STATUS_INVALID_PARAM;
    }
    if(!GPIO_IsValidPort(port) || !GPIO_IsValidPin(pin) || !GPIO_IsValidConfig(config)) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    // Initialize handle structure
    handle->port = port;
    handle->pin = pin;
    handle->config = *config;
    handle->is_initialized = false;
    handle->interrupt_callback = NULL;
    handle->current_state = config->initial_state;

    // Enable GPIO port clock
    GPIO_Status_t status = GPIO_EnablePortClock(port);
    if(status != GPIO_STATUS_SUCCESS) {
        return status;
    }

    // Unlock pin if necessary (for special pins like PF0)
    status = GPIO_UnlockPin(handle);
    if(status != GPIO_STATUS_SUCCESS) {
        return status;
    }

    // Configure GPIO hardware registers
    status = GPIO_ConfigureHardware(handle);
    if(status != GPIO_STATUS_SUCCESS) {
        return status;
    }

    // Store handle for interrupt handling if needed
    gpio_interrupt_handles[port][pin] = handle;

    // Mark as initialized
    handle->is_initialized = true;

    return GPIO_STATUS_SUCCESS;
}
/*
 * @brief Deinitialize GPIO Pin
 *
 */
static GPIO_Status_t GPIO_DeInit(GPIO_Handle_t* handle)
{
    // Validate parameters
    if(!GPIO_IsValidHandle(handle)) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);

    //Resetpin configuration based on port
    switch(handle->port) {
        case GPIO_PORT_A:
            GPIO_PORTA_DIR_R &= ~pin_mask;    // Set as input
            GPIO_PORTA_DEN_R &= ~pin_mask;    // Disable digital function
            GPIO_PORTA_AFSEL_R &= ~pin_mask;  // Disable alternate function
            GPIO_PORTA_PUR_R &= ~pin_mask;   // Disable pull-up resistor
            GPIO_PORTA_PDR_R &= ~pin_mask;   // Disable pull-down resistor
            GPIO_PORTA_IM_R &= ~pin_mask;    // Disable interrupt
            break;
        case GPIO_PORT_B:
            GPIO_PORTB_DIR_R &= ~pin_mask;    // Set as input
            GPIO_PORTB_DEN_R &= ~pin_mask;    // Disable digital function
            GPIO_PORTB_AFSEL_R &= ~pin_mask;  // Disable alternate function
            GPIO_PORTB_PUR_R &= ~pin_mask;   // Disable pull-up resistor
            GPIO_PORTB_PDR_R &= ~pin_mask;   // Disable pull-down resistor
            GPIO_PORTB_IM_R &= ~pin_mask;    // Disable interrupt
            break;
        default: 
            return GPIO_STATUS_INVALID_PARAM;
    }

    //Remove from interrupt handles
    gpio_interrupt_handles[handle->port][handle->pin] = NULL;

    //Mark as not initialized
    handle->is_initialized = false;

    return GPIO_STATUS_SUCCESS;
}

/*
 * @brief Check if GPIO Handle is valid
 *
 */
static inline bool GPIO_IsValidHandle(const GPIO_Handle_t* handle)
{
    return (handle != NULL && handle->is_initialized);
}

/*
 * @brief Check if GPIO port is valid
 *
 */
static inline bool GPIO_IsValidPort(GPIO_Port_t port)
{
    return (port < GPIO_PORT_COUNT);
}

/*
 * @brief Check if GPIO pin is valid
 *
 */
static inline bool GPIO_IsValidPin(GPIO_Pin_t pin)
{
    return (pin < GPIO_PIN_COUNT);
}

/*
 * @brief Check if GPIO config is valid
 *
 */
static inline bool GPIO_IsValidConfig(const GPIO_Config_t* config)
{
    return (config != NULL &&
            config->mode <= GPIO_MODE_ANALOG &&
            config->pull <= GPIO_PULL_DOWN &&
            config->drive_strength <= GPIO_DRIVE_8MA &&
            config->interrupt_type <= GPIO_INT_HIGH_LEVEL &&
            config->alternate_function <= 15);
}

/*
 * @brief Get pin mask for specific pin
 *
 */
static inline uint32_t GPIO_GetPinMask(GPIO_Pin_t pin)
{
    return (1U << pin);
}

/*
 * @brief Enable clock for specific GPIO port
 *
 */
static GPIO_Status_t GPIO_EnablePortClock(GPIO_Port_t port)
{
    uint32_t clock_mask;
    uint32_t ready_mask;

    switch (port)
    {
        case GPIO_PORT_A:
            clock_mask = SYSCTL_RCGCGPIO_R0;
            ready_mask = SYSCTL_PRGPIO_R0;
            break;
        case GPIO_PORT_B:
            clock_mask = SYSCTL_RCGCGPIO_R1;
            ready_mask = SYSCTL_PRGPIO_R1;
            break;
        case GPIO_PORT_C:
            clock_mask = SYSCTL_RCGCGPIO_R2;
            ready_mask = SYSCTL_PRGPIO_R2;
            break;
        case GPIO_PORT_D:
            clock_mask = SYSCTL_RCGCGPIO_R3;
            ready_mask = SYSCTL_PRGPIO_R3;
            break;
        case GPIO_PORT_E:
            clock_mask = SYSCTL_RCGCGPIO_R4;
            ready_mask = SYSCTL_PRGPIO_R4;
            break;
        case GPIO_PORT_F:
            clock_mask = SYSCTL_RCGCGPIO_R5;
            ready_mask = SYSCTL_PRGPIO_R5;
            break;
        default:
            return GPIO_STATUS_INVALID_PARAM;
    }

    // Enable clock
    SYSCTL_RCGCGPIO_R |= clock_mask;

    // Wait until the port is ready
    while((SYSCTL_PRGPIO_R & ready_mask) == 0);

    return GPIO_STATUS_SUCCESS;
}

/*
 * @brief Unlock Special GPIO Pins (like PF0, PD7,
 *          PC0-PC3, PB2-PB3, PA0-PA5)
 *
 */
static GPIO_Status_t GPIO_UnlockPin(GPIO_Handle_t* handle)
{
    if(handle->port == GPIO_PORT_F && handle->pin == GPIO_PIN_0) {
        GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; // Unlock GPIO Port F
        GPIO_PORTF_CR_R |= GPIO_GetPinMask(handle->pin); // Allow changes to PF0
        GPIO_PORTF_LOCK_R = 0; // Relock GPIO Port F
    }
    else if(handle->port == GPIO_PORT_D && handle->pin == GPIO_PIN_7) {
        GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY; // Unlock GPIO Port D
        GPIO_PORTD_CR_R |= GPIO_GetPinMask(handle->pin); // Allow changes to PD7
        GPIO_PORTD_LOCK_R = 0; // Relock GPIO Port D
    }
    else if(handle->port == GPIO_PORT_C && ( handle->pin <= GPIO_PIN_3)) {
        GPIO_PORTC_LOCK_R = GPIO_LOCK_KEY; // Unlock GPIO Port C
        GPIO_PORTC_CR_R |= GPIO_GetPinMask(handle->pin); // Allow changes to PC0-PC3
        GPIO_PORTC_LOCK_R = 0; // Relock GPIO Port C
    }
    else if(handle->port == GPIO_PORT_B && (handle->pin >= GPIO_PIN_2 && handle->pin <= GPIO_PIN_3)) {
        GPIO_PORTB_LOCK_R = GPIO_LOCK_KEY; // Unlock GPIO Port B
        GPIO_PORTB_CR_R |= GPIO_GetPinMask(handle->pin); // Allow changes to PB2-PB3
        GPIO_PORTB_LOCK_R = 0; // Relock GPIO Port B
    }
    else if(handle->port == GPIO_PORT_A && (handle->pin <= GPIO_PIN_5)) {
        GPIO_PORTA_LOCK_R = GPIO_LOCK_KEY; // Unlock GPIO Port A
        GPIO_PORTA_CR_R |= GPIO_GetPinMask(handle->pin); // Allow changes to PA0-PA5
        GPIO_PORTA_LOCK_R = 0; // Relock GPIO Port A
    }

    return GPIO_STATUS_SUCCESS;
}

/*
 * @brief Configure GPIO hardware registers based on handle config
 *
 */
 static GPIO_Status_t GPIO_ConfigureHardware(GPIO_Handle_t* handle)
 {
    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);

    //COnfigure based on port
    switch(handle->port) {
        case GPIO_PORT_A:
            //Configure direction
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                GPIO_PORTA_DIR_R |= pin_mask; // Set as output
            }
            else {
                GPIO_PORTA_DIR_R &= ~pin_mask; // Set as input
            }

            //Configure pull-up/pull-down resitors
            GPIO_PORTA_PUR_R &= ~pin_mask; // Disable pull-up resistor
            GPIO_PORTA_PDR_R &= ~pin_mask; // Disable pull-down resistor
            if(handle->config.pull == GPIO_PULL_UP) {
                GPIO_PORTA_PUR_R |= pin_mask; // Enable pull-up resistor
            }
            else if(handle->config.pull == GPIO_PULL_DOWN) {
                GPIO_PORTA_PDR_R |= pin_mask; // Enable pull-down resistor
            }

            //Configure drive strength
            GPIO_PORTA_DR2R_R &= ~pin_mask; // Disable 2mA drive
            GPIO_PORTA_DR4R_R &= ~pin_mask; // Disable 4mA drive
            GPIO_PORTA_DR8R_R &= ~pin_mask; // Disable 8mA drive
            switch(handle->config.drive_strength) {
                case GPIO_DRIVE_2MA: GPIO_PORTA_DR2R_R |= pin_mask; break; // Enable 2mA drive
                case GPIO_DRIVE_4MA: GPIO_PORTA_DR4R_R |= pin_mask; break; // Enable 4mA drive
                case GPIO_DRIVE_8MA: GPIO_PORTA_DR8R_R |= pin_mask; break; // Enable 8mA drive
                default: return GPIO_STATUS_INVALID_PARAM;
            }

            // Configure slew rate control
            if(handle->config.slew_rate_control) {
                GPIO_PORTA_SLR_R |= pin_mask; // Enable slew rate control
            } else {
                GPIO_PORTA_SLR_R &= ~pin_mask; // Disable slew rate control
            }

            // Configure open-drain
            if(handle->config.open_drain) {
                GPIO_PORTA_ODR_R |= pin_mask; // Enable open-drain
            } else {
                GPIO_PORTA_ODR_R &= ~pin_mask; // Disable open-drain
            }

            // Configure alternate function
            if(handle->config.alternate_function) {
                GPIO_PORTA_AFSEL_R |= pin_mask; // Enable alternate function
                // Set the appropriate bits in the PCTL register
                GPIO_PORTA_PCTL_R &= ~(0xF << (handle->pin * 4)); // Clear existing bits
                GPIO_PORTA_PCTL_R |= (handle->config.alternate_function << (handle->pin * 4)); // Set new function
            } else {
                GPIO_PORTA_AFSEL_R &= ~pin_mask; // Disable alternate function
                //GPIO_PORTA_PCTL_R &= ~(0xF << (handle->pin * 4)); // Clear PCTL bits
            }

            // Enable digital or analog function
            if(handle->config.mode == GPIO_MODE_ANALOG) {
                GPIO_PORTA_DEN_R &= ~pin_mask; // Disable digital function
                GPIO_PORTA_AMSEL_R |= pin_mask; // Enable analog function
            } else {
                GPIO_PORTA_DEN_R |= pin_mask; // Enable digital function
                GPIO_PORTA_AMSEL_R &= ~pin_mask; // Disable analog function
            }

            // Set initial state for output mode
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                if(handle->config.initial_state == GPIO_STATE_HIGH) {
                    GPIO_PORTA_DATA_R |= pin_mask; // Set pin high
                } else {
                    GPIO_PORTA_DATA_R &= ~pin_mask; // Set pin low
                }
            }
            break;
        case GPIO_PORT_B:
            //Configure direction
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                GPIO_PORTB_DIR_R |= pin_mask; // Set as output
            }
            else {
                GPIO_PORTB_DIR_R &= ~pin_mask; // Set as input
            }

            // Configure pull-up/pull-down resitors
            GPIO_PORTB_PUR_R &= ~pin_mask; // Disable pull-up resistor
            GPIO_PORTB_PDR_R &= ~pin_mask; // Disable pull-down resistor
            if(handle->config.pull == GPIO_PULL_UP) {
                GPIO_PORTB_PUR_R |= pin_mask; // Enable pull-up resistor
            }
            else if(handle->config.pull == GPIO_PULL_DOWN) {
                GPIO_PORTB_PDR_R |= pin_mask; // Enable pull-down resistor
            }

            // Configure drive strength
            GPIO_PORTB_DR2R_R &= ~pin_mask; // Disable 2mA drive
            GPIO_PORTB_DR4R_R &= ~pin_mask; // Disable 4mA drive
            GPIO_PORTB_DR8R_R &= ~pin_mask; // Disable 8mA drive
            switch(handle->config.drive_strength) {
                case GPIO_DRIVE_2MA: GPIO_PORTB_DR2R_R |= pin_mask; break; // Enable 2mA drive
                case GPIO_DRIVE_4MA: GPIO_PORTB_DR4R_R |= pin_mask; break; // Enable 4mA drive
                case GPIO_DRIVE_8MA: GPIO_PORTB_DR8R_R |= pin_mask; break; // Enable 8mA drive
                default: return GPIO_STATUS_INVALID_PARAM;
            }

            
            // Configure analog/digital function
            if(handle->config.mode == GPIO_MODE_ANALOG) {
                GPIO_PORTB_DEN_R &= ~pin_mask; // Disable digital function
                GPIO_PORTB_AMSEL_R |= pin_mask; // Enable analog function
            } else {
                GPIO_PORTB_DEN_R |= pin_mask; // Enable digital function
                GPIO_PORTB_AMSEL_R &= ~pin_mask; // Disable analog function
            }

            // Set initial state for output mode
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                if(handle->config.initial_state == GPIO_STATE_HIGH) {
                    GPIO_PORTB_DATA_R |= pin_mask; // Set pin high
                } else {
                    GPIO_PORTB_DATA_R &= ~pin_mask; // Set pin low
                }
            }
            break;
        case GPIO_PORT_C:
            // Configure direction
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                GPIO_PORTC_DIR_R |= pin_mask; // Set as output
            }
            else {
                GPIO_PORTC_DIR_R &= ~pin_mask; // Set as input
            }

            // Configure pull-up/pull-down resitors
            GPIO_PORTC_PUR_R &= ~pin_mask; // Disable pull-up resistor
            GPIO_PORTC_PDR_R &= ~pin_mask; // Disable pull-down resistor
            if(handle->config.pull == GPIO_PULL_UP) {
                GPIO_PORTC_PUR_R |= pin_mask; // Enable pull-up resistor
            }
            else if(handle->config.pull == GPIO_PULL_DOWN) {
                GPIO_PORTC_PDR_R |= pin_mask; // Enable pull-down resistor
            }

            // Configure drive strength
            GPIO_PORTC_DR2R_R &= ~pin_mask; // Disable 2mA drive
            GPIO_PORTC_DR4R_R &= ~pin_mask; // Disable 4mA drive
            GPIO_PORTC_DR8R_R &= ~pin_mask; // Disable 8mA drive
            switch(handle->config.drive_strength) {
                case GPIO_DRIVE_2MA: GPIO_PORTC_DR2R_R |= pin_mask; break; // Enable 2mA drive
                case GPIO_DRIVE_4MA: GPIO_PORTC_DR4R_R |= pin_mask; break; // Enable 4mA drive
                case GPIO_DRIVE_8MA: GPIO_PORTC_DR8R_R |= pin_mask; break; // Enable 8mA drive
                default: return GPIO_STATUS_INVALID_PARAM;
            }

            // Configure Slave rate control
            if(handle->config.slew_rate_control) {
                GPIO_PORTC_SLR_R |= pin_mask; // Enable slew rate control
            } else {
                GPIO_PORTC_SLR_R &= ~pin_mask; // Disable slew rate control
            }

            // Configure open-drain
            if(handle->config.open_drain) {
                GPIO_PORTC_ODR_R |= pin_mask; // Enable open-drain
            } else {
                GPIO_PORTC_ODR_R &= ~pin_mask; // Disable open-drain
            }

            // Configure alternate function
            if(handle->config.alternate_function) {
                GPIO_PORTC_AFSEL_R |= pin_mask; // Enable alternate function
                // Set the appropriate bits in the PCTL register
                GPIO_PORTC_PCTL_R &= ~(0xF << (handle->pin * 4)); // Clear existing bits
                GPIO_PORTC_PCTL_R |= (handle->config.alternate_function << (handle->pin * 4)); // Set new function
            } else {
                GPIO_PORTC_AFSEL_R &= ~pin_mask; // Disable alternate function
            }

            // Configure analog/digital function
            if(handle->config.mode == GPIO_MODE_ANALOG) {
                GPIO_PORTC_DEN_R &= ~pin_mask; // Disable digital function
                GPIO_PORTC_AMSEL_R |= pin_mask; // Enable analog function
            } else {
                GPIO_PORTC_DEN_R |= pin_mask; // Enable digital function
                GPIO_PORTC_AMSEL_R &= ~pin_mask; // Disable analog function
            }

            // Set initial state for output mode
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                if(handle->config.initial_state == GPIO_STATE_HIGH) {
                    GPIO_PORTC_DATA_R |= pin_mask; // Set pin high
                } else {
                    GPIO_PORTC_DATA_R &= ~pin_mask; // Set pin low
                }
            }
            break;
        case GPIO_PORT_D:
            // Configure direction
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                GPIO_PORTD_DIR_R |= pin_mask; // Set as output
            } else {
                GPIO_PORTD_DIR_R &= ~pin_mask; // Set as input
            }

            //Configure pull-up/pull-down resitors
            GPIO_PORTD_PUR_R &= ~pin_mask; // Disable pull-up resistor
            GPIO_PORTD_PDR_R &= ~pin_mask; // Disable pull-down resistor
            if(handle->config.pull == GPIO_PULL_UP) {
                GPIO_PORTD_PUR_R |= pin_mask; // Enable pull-up resistor
            } else {
                GPIO_PORTD_PDR_R |= pin_mask; // Enable pull-down resistor
            }

            //Configure drive strength
            GPIO_PORTD_DR2R_R &= ~pin_mask; // Disable 2mA drive
            GPIO_PORTD_DR4R_R &= ~pin_mask; // Disable 4mA drive
            GPIO_PORTD_DR8R_R &= ~pin_mask; // Disable 8mA drive
            switch(handle->config.drive_strength) {
                case GPIO_DRIVE_2MA: GPIO_PORTD_DR2R_R |= pin_mask; break; // Enable 2mA drive
                case GPIO_DRIVE_4MA: GPIO_PORTD_DR4R_R |= pin_mask; break; // Enable 4mA drive
                case GPIO_DRIVE_8MA: GPIO_PORTD_DR8R_R |= pin_mask; break; // Enable 8mA drive
                default: return GPIO_STATUS_INVALID_PARAM;
            
            }

            // Configure Slew rate control
            if(handle->config.slew_rate_control) {
                GPIO_PORTD_SLR_R |= pin_mask; // Enable slew rate control
            } else {
                GPIO_PORTD_SLR_R &= ~pin_mask; // Disable slew rate control
            }

            // Configure open-drain
            if(handle->config.open_drain) {
                GPIO_PORTD_ODR_R |= pin_mask; // Enable open-drain
            } else {
                GPIO_PORTD_ODR_R &= ~pin_mask; // Disable open-drain
            }

            // Configure alternate function
            if(handle->config.alternate_function) {
                GPIO_PORTD_AFSEL_R |= pin_mask; // Enable alternate function
                // Set the appropriate bits in the PCTL register
                GPIO_PORTD_PCTL_R &= ~(0xF << (handle->pin * 4)); // Clear existing bits
                GPIO_PORTD_PCTL_R |= (handle->config.alternate_function << (handle->pin * 4)); // Set new function
            } else {
                GPIO_PORTD_AFSEL_R &= ~pin_mask; // Disable alternate function
            }

            // Configure analog/digital function
            if(handle->config.mode == GPIO_MODE_ANALOG) {
                GPIO_PORTD_DEN_R &= ~pin_mask; // Disable digital function
                GPIO_PORTD_AMSEL_R |= pin_mask; // Enable analog function
            } else {
                GPIO_PORTD_DEN_R |= pin_mask; // Enable digital function
                GPIO_PORTD_AMSEL_R &= ~pin_mask; // Disable analog function
            }

            // Set initial state for output mode
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                if(handle->config.initial_state == GPIO_STATE_HIGH) {
                    GPIO_PORTD_DATA_R |= pin_mask; // Set pin high
                } else {
                    GPIO_PORTD_DATA_R &= ~pin_mask; // Set pin low
                }
            }


            break;
        case GPIO_PORT_E:
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                GPIO_PORTE_DIR_R |= pin_mask; // Set as output
            }
            else {
                GPIO_PORTE_DIR_R &= ~pin_mask; // Set as input
            }

            // Configure pull-up/pull-down resitors
            GPIO_PORTE_PUR_R &= ~pin_mask; // Disable pull-up resistor
            GPIO_PORTE_PDR_R &= ~pin_mask; // Disable pull-down resistor
            if(handle->config.pull == GPIO_PULL_UP) {
                GPIO_PORTE_PUR_R |= pin_mask; // Enable pull-up resistor
            }
            else if(handle->config.pull == GPIO_PULL_DOWN) {
                GPIO_PORTE_PDR_R |= pin_mask; // Enable pull-down resistor
            }

            // Configure drive strength
            GPIO_PORTE_DR2R_R &= ~pin_mask; // Disable 2mA drive
            GPIO_PORTE_DR4R_R &= ~pin_mask; // Disable 4mA drive
            GPIO_PORTE_DR8R_R &= ~pin_mask; // Disable 8mA drive
            switch(handle->config.drive_strength) {
                case GPIO_DRIVE_2MA: GPIO_PORTE_DR2R_R |= pin_mask; break; // Enable 2mA drive
                case GPIO_DRIVE_4MA: GPIO_PORTE_DR4R_R |= pin_mask; break; // Enable 4mA drive
                case GPIO_DRIVE_8MA: GPIO_PORTE_DR8R_R |= pin_mask; break; // Enable 8mA drive
                default: return GPIO_STATUS_INVALID_PARAM;
            }

            // Configure slew rate control
            if(handle->config.slew_rate_control) {
                GPIO_PORTE_SLR_R |= pin_mask; // Enable slew rate control
            } else {
                GPIO_PORTE_SLR_R &= ~pin_mask; // Disable slew rate control
            }

            // Configure open-drain
            if(handle->config.open_drain) {
                GPIO_PORTE_ODR_R |= pin_mask; // Enable open-drain
            } else {
                GPIO_PORTE_ODR_R &= ~pin_mask; // Disable open-drain
            }

            // Configure alternate function
            if(handle->config.alternate_function) {
                GPIO_PORTE_AFSEL_R |= pin_mask; // Enable alternate function
                // Set the appropriate bits in the PCTL register
                GPIO_PORTE_PCTL_R &= ~(0xF << (handle->pin * 4)); // Clear existing bits
                GPIO_PORTE_PCTL_R |= (handle->config.alternate_function << (handle->pin * 4)); // Set new function
            } else {
                GPIO_PORTE_AFSEL_R &= ~pin_mask; // Disable alternate function
            }

            // Configure analog/digital function
            if(handle->config.mode == GPIO_MODE_ANALOG) {
                GPIO_PORTE_DEN_R &= ~pin_mask; // Disable digital function
                GPIO_PORTE_AMSEL_R |= pin_mask; // Enable analog function
            } else {
                GPIO_PORTE_DEN_R |= pin_mask; // Enable digital function
                GPIO_PORTE_AMSEL_R &= ~pin_mask; // Disable analog function
            }

            // Set initial state for output mode
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                if(handle->config.initial_state == GPIO_STATE_HIGH) {
                    GPIO_PORTE_DATA_R |= pin_mask; // Set pin high
                } else {
                    GPIO_PORTE_DATA_R &= ~pin_mask; // Set pin low
                }
            }
            break;
        case GPIO_PORT_F:
            // Configure direction
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                GPIO_PORTF_DIR_R |= pin_mask; // Set as output
            }
            else {
                GPIO_PORTF_DIR_R &= ~pin_mask; // Set as input
            }

            // Configure pull-up/pull-down resitors
            GPIO_PORTF_PUR_R &= ~pin_mask; // Disable pull-up resistor
            GPIO_PORTF_PDR_R &= ~pin_mask; // Disable pull-down resistor
            if(handle->config.pull == GPIO_PULL_UP) {
                GPIO_PORTF_PUR_R |= pin_mask; // Enable pull-up resistor
            }
            else if(handle->config.pull == GPIO_PULL_DOWN) {
                GPIO_PORTF_PDR_R |= pin_mask; // Enable pull-down resistor
            }

            // Configure drive strength
            GPIO_PORTF_DR2R_R &= ~pin_mask; // Disable 2mA drive
            GPIO_PORTF_DR4R_R &= ~pin_mask; // Disable 4mA drive
            GPIO_PORTF_DR8R_R &= ~pin_mask; // Disable 8mA drive
            switch(handle->config.drive_strength) {
                case GPIO_DRIVE_2MA: GPIO_PORTF_DR2R_R |= pin_mask; break; // Enable 2mA drive
                case GPIO_DRIVE_4MA: GPIO_PORTF_DR4R_R |= pin_mask; break; // Enable 4mA drive
                case GPIO_DRIVE_8MA: GPIO_PORTF_DR8R_R |= pin_mask; break; // Enable 8mA drive
                default: return GPIO_STATUS_INVALID_PARAM;
            }

            // Configure slew rate control
            if(handle->config.slew_rate_control) {
                GPIO_PORTF_SLR_R |= pin_mask; // Enable slew rate control
            } else {
                GPIO_PORTF_SLR_R &= ~pin_mask; // Disable slew rate control
            }

            // Configure open-drain
            if(handle->config.open_drain) {
                GPIO_PORTF_ODR_R |= pin_mask; // Enable open-drain
            } else {
                GPIO_PORTF_ODR_R &= ~pin_mask; // Disable open-drain
            }

            // Configure alternate function
            if(handle->config.alternate_function) {
                GPIO_PORTF_AFSEL_R |= pin_mask; // Enable alternate function
                // Set the appropriate bits in the PCTL register
                GPIO_PORTF_PCTL_R &= ~(0xF << (handle->pin * 4)); // Clear existing bits
                GPIO_PORTF_PCTL_R |= (handle->config.alternate_function << (handle->pin * 4)); // Set new function
            } else {
                GPIO_PORTF_AFSEL_R &= ~pin_mask; // Disable alternate function
            }

            // Configure analog/digital function
            if(handle->config.mode == GPIO_MODE_ANALOG) {
                GPIO_PORTF_DEN_R &= ~pin_mask; // Disable digital function
                GPIO_PORTF_AMSEL_R |= pin_mask; // Enable analog function
            } else {
                GPIO_PORTF_DEN_R |= pin_mask; // Enable digital function
                GPIO_PORTF_AMSEL_R &= ~pin_mask; // Disable analog function
            }

            // Set initial state for output mode
            if(handle->config.mode == GPIO_MODE_OUTPUT) {
                if(handle->config.initial_state == GPIO_STATE_HIGH) {
                    GPIO_PORTF_DATA_R |= pin_mask; // Set pin high
                } else {
                    GPIO_PORTF_DATA_R &= ~pin_mask; // Set pin low
                }
            }
            break;
        default: return GPIO_STATUS_INVALID_PARAM;
    }
    return GPIO_STATUS_SUCCESS;
 }

/*
 * @brief Set GPIO Pin to HIGH
 *
 */
static GPIO_Status_t GPIO_SetPin(GPIO_Handle_t* handle)
{
    // Check for valid handle
    if(!GPIO_IsValidHandle(handle)) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    // Check if pin is configured as output
    if(handle->config.mode != GPIO_MODE_OUTPUT) {
        return GPIO_STATUS_INVALID_PARAM; // Cannot set pin if not output
    }

    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);

    
    switch(handle->port) {
        case GPIO_PORT_A: GPIO_PORTA_DATA_R |= pin_mask; break;
        case GPIO_PORT_B: GPIO_PORTB_DATA_R |= pin_mask; break;
        case GPIO_PORT_C: GPIO_PORTC_DATA_R |= pin_mask; break;
        case GPIO_PORT_D: GPIO_PORTD_DATA_R |= pin_mask; break;
        case GPIO_PORT_E: GPIO_PORTE_DATA_R |= pin_mask; break;
        case GPIO_PORT_F: GPIO_PORTF_DATA_R |= pin_mask; break;
        default: return GPIO_STATUS_INVALID_PARAM;
    }

    handle->current_state = GPIO_STATE_HIGH;
    return GPIO_STATUS_SUCCESS;
}

/*
 * @brief Set GPIO Pin to LOW
 *
 */
static GPIO_Status_t GPIO_ClearPin(GPIO_Handle_t* handle)
{
    // Check for valid handle
    if(!GPIO_IsValidHandle(handle)) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    // Check if pin is configured as output
    if(handle->config.mode != GPIO_MODE_OUTPUT) {
        return GPIO_STATUS_INVALID_PARAM; // Cannot set pin if not output
    }

    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);

    
    switch(handle->port) {
        case GPIO_PORT_A: GPIO_PORTA_DATA_R &= ~pin_mask; break;
        case GPIO_PORT_B: GPIO_PORTB_DATA_R &= ~pin_mask; break;
        case GPIO_PORT_C: GPIO_PORTC_DATA_R &= ~pin_mask; break;
        case GPIO_PORT_D: GPIO_PORTD_DATA_R &= ~pin_mask; break;
        case GPIO_PORT_E: GPIO_PORTE_DATA_R &= ~pin_mask; break;
        case GPIO_PORT_F: GPIO_PORTF_DATA_R &= ~pin_mask; break;
        default: return GPIO_STATUS_INVALID_PARAM;
    }

    handle->current_state = GPIO_STATE_LOW;
    return GPIO_STATUS_SUCCESS;
}

/*
 * @brief Toggle GPIO Pin State
 *
 */
static GPIO_Status_t GPIO_TogglePin(GPIO_Handle_t* handle)
{
    // Check for valid handle
    if(!GPIO_IsValidHandle(handle)) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    // Check if pin is configured as output
    if(handle->config.mode != GPIO_MODE_OUTPUT) {
        return GPIO_STATUS_INVALID_PARAM; // Cannot toggle pin if not output
    }

    if(handle->current_state == GPIO_STATE_LOW) {
        return GPIO_SetPin(handle);
    } else {
        return GPIO_ClearPin(handle);
    }
}

/*
 * @brief Write specific state to GPIO Pin
 *
 */
static GPIO_Status_t GPIO_WritePin(GPIO_Handle_t* handle, GPIO_State_t state)
{
    // Check for valid handle
    if(!GPIO_IsValidHandle(handle)) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    if(state == GPIO_STATE_HIGH) {
        return GPIO_SetPin(handle);
    } else {
        return GPIO_ClearPin(handle);
    }
}

/*
 * @brief Read GPIO Pin State
 *
 */
static GPIO_Status_t GPIO_ReadPin(GPIO_Handle_t* handle, GPIO_State_t* state)
{
    if(!GPIO_IsValidHandle(handle) || !state) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);
    uint32_t pin_value = 0;

    switch (handle->port) {
        case GPIO_PORT_A: pin_value = GPIO_PORTA_DATA_R & pin_mask; break;
        case GPIO_PORT_B: pin_value = GPIO_PORTB_DATA_R & pin_mask; break;
        case GPIO_PORT_C: pin_value = GPIO_PORTC_DATA_R & pin_mask; break;
        case GPIO_PORT_D: pin_value = GPIO_PORTD_DATA_R & pin_mask; break;
        case GPIO_PORT_E: pin_value = GPIO_PORTE_DATA_R & pin_mask; break;
        case GPIO_PORT_F: pin_value = GPIO_PORTF_DATA_R & pin_mask; break;
        default: return GPIO_STATUS_INVALID_PARAM;
    }

    *state = (pin_value != 0) ? GPIO_STATE_HIGH : GPIO_STATE_LOW;
    handle->current_state = *state;
    return GPIO_STATUS_SUCCESS;
}

/*
 * @brief Configure interrupt for GPIO Pin
 *
 */
static GPIO_Status_t GPIO_ConfigureInterrupt(GPIO_Handle_t* handle, GPIO_Interrupt_t interrupt_type, void (*callback)(void))
{
    if(!GPIO_IsValidHandle(handle) || !callback) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    //Store callback
    handle->interrupt_callback = callback;
    handle->config.interrupt_type = interrupt_type;

    //Get pin mask
    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);

    // Configure interrupt based on port
    // Full implementation would configure IS, IBE, IEV registers per port
    switch(handle->port) {
        case GPIO_PORT_A:
            switch(interrupt_type) {
                case GPIO_INT_FALLING_EDGE:
                    GPIO_PORTA_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTA_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTA_IEV_R &= ~pin_mask; // Falling edge
                    break;
                case GPIO_INT_RISING_EDGE:
                    GPIO_PORTA_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTA_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTA_IEV_R |= pin_mask; // Rising edge
                    break;
                case GPIO_INT_BOTH_EDGES:
                    GPIO_PORTA_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTA_IBE_R |= pin_mask;  // Both edges
                    break;
                case GPIO_INT_LOW_LEVEL:
                    GPIO_PORTA_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTA_IEV_R &= ~pin_mask; // Low level
                    break;
                case GPIO_INT_HIGH_LEVEL:
                    GPIO_PORTA_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTA_IEV_R |= pin_mask; // High level
                    break;
                default: return GPIO_STATUS_INVALID_PARAM;
            }
            break;
        case GPIO_PORT_B:
            switch(interrupt_type) {
                case GPIO_INT_FALLING_EDGE:
                    GPIO_PORTB_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTB_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTB_IEV_R &= ~pin_mask; // Falling edge
                    break;
                case GPIO_INT_RISING_EDGE:
                    GPIO_PORTB_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTB_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTB_IEV_R |= pin_mask; // Rising edge
                    break;
                case GPIO_INT_BOTH_EDGES:
                    GPIO_PORTB_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTB_IBE_R |= pin_mask;  // Both edges
                    break;
                case GPIO_INT_LOW_LEVEL:
                    GPIO_PORTB_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTB_IEV_R &= ~pin_mask; // Low level
                    break;
                case GPIO_INT_HIGH_LEVEL:
                    GPIO_PORTB_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTB_IEV_R |= pin_mask; // High level
                    break;
                default: return GPIO_STATUS_INVALID_PARAM;
            }
            break;
        case GPIO_PORT_C:
            switch(interrupt_type) {
                case GPIO_INT_FALLING_EDGE:
                    GPIO_PORTC_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTC_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTC_IEV_R &= ~pin_mask; // Falling edge
                    break;
                case GPIO_INT_RISING_EDGE:
                    GPIO_PORTC_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTC_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTC_IEV_R |= pin_mask; // Rising edge
                    break;
                case GPIO_INT_BOTH_EDGES:
                    GPIO_PORTC_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTC_IBE_R |= pin_mask;  // Both edges
                    break;
                case GPIO_INT_LOW_LEVEL:
                    GPIO_PORTC_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTC_IEV_R &= ~pin_mask; // Low level
                    break;
                case GPIO_INT_HIGH_LEVEL:
                    GPIO_PORTC_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTC_IEV_R |= pin_mask; // High level
                    break;
                default: return GPIO_STATUS_INVALID_PARAM;
            }
            break;
        case GPIO_PORT_D:
            switch(interrupt_type) {
                case GPIO_INT_FALLING_EDGE:
                    GPIO_PORTD_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTD_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTD_IEV_R &= ~pin_mask; // Falling edge
                    break;
                case GPIO_INT_RISING_EDGE:
                    GPIO_PORTD_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTD_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTD_IEV_R |= pin_mask; // Rising edge
                    break;
                case GPIO_INT_BOTH_EDGES:
                    GPIO_PORTD_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTD_IBE_R |= pin_mask;  // Both edges
                    break;
                case GPIO_INT_LOW_LEVEL:
                    GPIO_PORTD_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTD_IEV_R &= ~pin_mask; // Low level
                    break;
                case GPIO_INT_HIGH_LEVEL:
                    GPIO_PORTD_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTD_IEV_R |= pin_mask; // High level
                    break;
                default: return GPIO_STATUS_INVALID_PARAM;
            }
            break;
        case GPIO_PORT_E:
            switch(interrupt_type) {
                case GPIO_INT_FALLING_EDGE:
                    GPIO_PORTE_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTE_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTE_IEV_R &= ~pin_mask; // Falling edge
                    break;
                case GPIO_INT_RISING_EDGE:
                    GPIO_PORTE_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTE_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTE_IEV_R |= pin_mask; // Rising edge
                    break;
                case GPIO_INT_BOTH_EDGES:
                    GPIO_PORTE_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTE_IBE_R |= pin_mask;  // Both edges
                    break;
                case GPIO_INT_LOW_LEVEL:
                    GPIO_PORTE_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTE_IEV_R &= ~pin_mask; // Low level
                    break;
                case GPIO_INT_HIGH_LEVEL:
                    GPIO_PORTE_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTE_IEV_R |= pin_mask; // High level
                    break;
                default: return GPIO_STATUS_INVALID_PARAM;
            }
            break;
        case GPIO_PORT_F:
            switch(interrupt_type) {
                case GPIO_INT_FALLING_EDGE:
                    GPIO_PORTF_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTF_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTF_IEV_R &= ~pin_mask; // Falling edge
                    break;
                case GPIO_INT_RISING_EDGE:
                    GPIO_PORTF_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTF_IBE_R &= ~pin_mask; // Not both edges
                    GPIO_PORTF_IEV_R |= pin_mask; // Rising edge
                    break;
                case GPIO_INT_BOTH_EDGES:
                    GPIO_PORTF_IS_R &= ~pin_mask;  // Edge-sensitive
                    GPIO_PORTF_IBE_R |= pin_mask;  // Both edges
                    break;
                case GPIO_INT_LOW_LEVEL:
                    GPIO_PORTF_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTF_IEV_R &= ~pin_mask; // Low level
                    break;
                case GPIO_INT_HIGH_LEVEL:
                    GPIO_PORTF_IS_R |= pin_mask;   // Level-sensitive
                    GPIO_PORTF_IEV_R |= pin_mask; // High level
                    break;
                default: return GPIO_STATUS_INVALID_PARAM;
            }
            break;
        default: return GPIO_STATUS_INVALID_PARAM;
    }

    return GPIO_STATUS_SUCCESS;
}

/*
 * @brief Enable GPIO interrupt
 *
 */
static GPIO_Status_t GPIO_EnableInterrupt(GPIO_Handle_t* handle)
{
    if(!GPIO_IsValidHandle(handle)) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    // Check if interrupt is configured
    if(!handle->interrupt_callback) {
        return GPIO_STATUS_INVALID_PARAM; // No callback set
    }

    //Get pin mask
    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);
    // Implementation would disable IM register for the specific port/pin
    // Enable interrupt based on port
    switch(handle->port) {
        case GPIO_PORT_A:
            GPIO_PORTA_ICR_R |= pin_mask; // Clear any prior interrupt
            GPIO_PORTA_IM_R |= pin_mask;  // Enable interrupt
            NVIC_EN0_R |= (1U << INTT_GPIOA); // Enable NVIC interrupt for Port A
            break;
        case GPIO_PORT_B:
            GPIO_PORTB_ICR_R |= pin_mask; // Clear any prior interrupt
            GPIO_PORTB_IM_R |= pin_mask;  // Enable interrupt
            NVIC_EN0_R |= (1U << INTT_GPIOB); // Enable NVIC interrupt for Port B
            break;
        case GPIO_PORT_C:
            GPIO_PORTC_ICR_R |= pin_mask; // Clear any prior interrupt
            GPIO_PORTC_IM_R |= pin_mask;  // Enable interrupt
            NVIC_EN0_R |= (1U << INTT_GPIOC); // Enable NVIC interrupt for Port C
            break;
        case GPIO_PORT_D:
            GPIO_PORTD_ICR_R |= pin_mask; // Clear any prior interrupt
            GPIO_PORTD_IM_R |= pin_mask;  // Enable interrupt
            NVIC_EN0_R |= (1U << INTT_GPIOD); // Enable NVIC interrupt for Port D
            break;
        case GPIO_PORT_E:
            GPIO_PORTE_ICR_R |= pin_mask; // Clear any prior interrupt
            GPIO_PORTE_IM_R |= pin_mask;  // Enable interrupt
            NVIC_EN0_R |= (1U << INTT_GPIOE); // Enable NVIC interrupt for Port E
            break;
        case GPIO_PORT_F:
            GPIO_PORTF_ICR_R |= pin_mask; // Clear any prior interrupt
            GPIO_PORTF_IM_R |= pin_mask;  // Enable interrupt
            NVIC_EN0_R |= (1U << INTT_GPIOF); // Enable NVIC interrupt for Port F
            break;
        default: return GPIO_STATUS_INVALID_PARAM;
    }

    return GPIO_STATUS_SUCCESS;
}

/*
 * @brief Disable GPIO Interrupt
 *
 */
static GPIO_Status_t GPIO_DisableInterrupt(GPIO_Handle_t* handle)
{
    if(!GPIO_IsValidHandle(handle)) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    // Get pin mask
    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);

    // Implementation would disable IM register for the specific port/pin
    // Disable interrupt based on port
    switch(handle->port) {
        case GPIO_PORT_A: GPIO_PORTA_IM_R &= ~pin_mask; break;  // Disable PORTA interrupt
        case GPIO_PORT_B: GPIO_PORTB_IM_R &= ~pin_mask; break;  // Disable PORTB interrupt
        case GPIO_PORT_C: GPIO_PORTC_IM_R &= ~pin_mask; break;  // Disable PORTC interrupt
        case GPIO_PORT_D: GPIO_PORTD_IM_R &= ~pin_mask; break;  // Disable PORTD interrupt
        case GPIO_PORT_E: GPIO_PORTE_IM_R &= ~pin_mask; break;  // Disable PORTE interrupt
        case GPIO_PORT_F: GPIO_PORTF_IM_R &= ~pin_mask; break;  // Disable PORTF interrupt
        default: return GPIO_STATUS_INVALID_PARAM;              // Invalid port
    }
    return GPIO_STATUS_SUCCESS; // Success
}

/*
 * @brief Set GPIO Driver Strength
 *
 */
static GPIO_Status_t GPIO_SetDriveStrength(GPIO_Handle_t* handle, GPIO_Drive_t drive)
{
    if(!GPIO_IsValidHandle(handle)) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);

    // Configure drive strength based on port
    switch(handle->port) {
        case GPIO_PORT_A:
            GPIO_PORTA_DR2R_R &= ~pin_mask; // Disable 2mA drive
            GPIO_PORTA_DR4R_R &= ~pin_mask; // Disable 4mA drive
            GPIO_PORTA_DR8R_R &= ~pin_mask; // Disable 8mA drive
            switch(drive) {
                case GPIO_DRIVE_2MA: GPIO_PORTA_DR2R_R |= pin_mask; break; // Enable 2mA drive
                case GPIO_DRIVE_4MA: GPIO_PORTA_DR4R_R |= pin_mask; break; // Enable 4mA drive
                case GPIO_DRIVE_8MA: GPIO_PORTA_DR8R_R |= pin_mask; break; // Enable 8mA drive
                default: return GPIO_STATUS_INVALID_PARAM;
            }
            break;

        case GPIO_PORT_B:
            GPIO_PORTB_DR2R_R &= ~pin_mask; // Disable 2mA drive
            GPIO_PORTB_DR4R_R &= ~pin_mask; // Disable 4mA drive
            GPIO_PORTB_DR8R_R &= ~pin_mask; // Disable 8mA drive
            switch(drive) {
                case GPIO_DRIVE_2MA: GPIO_PORTB_DR2R_R |= pin_mask; break; // Enable 2mA drive
                case GPIO_DRIVE_4MA: GPIO_PORTB_DR4R_R |= pin_mask; break; // Enable 4mA drive
                case GPIO_DRIVE_8MA: GPIO_PORTB_DR8R_R |= pin_mask; break; // Enable 8mA drive
                default: return GPIO_STATUS_INVALID_PARAM;
            }
            break;
        // Add new cases for other ports as needed


        default: return GPIO_STATUS_INVALID_PARAM;
    }

    handle->config.drive_strength = drive;
    return GPIO_STATUS_SUCCESS;
}

/*
 * @brief Set GPIO Pull mode
 *
 */
static GPIO_Status_t GPIO_SetPullMode(GPIO_Handle_t* handle, GPIO_Pull_t pull)
{
    if(!GPIO_IsValidHandle(handle)) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);

    // Configure pull-up/pull-down resitors based on port
    switch(handle->port) {
        case GPIO_PORT_A:
            GPIO_PORTA_PUR_R &= ~pin_mask; // Disable pull-up resistor
            GPIO_PORTA_PDR_R &= ~pin_mask; // Disable pull-down resistor
            switch(pull) {
                case GPIO_PULL_UP: GPIO_PORTA_PUR_R |= pin_mask; break; // Enable pull-up resistor
                case GPIO_PULL_DOWN: GPIO_PORTA_PDR_R |= pin_mask; break; // Enable pull-down resistor
                case GPIO_PULL_NONE: break; // No pull resistor
                default: return GPIO_STATUS_INVALID_PARAM;
            }
            break;
        case GPIO_PORT_B:
            GPIO_PORTB_PUR_R &= ~pin_mask; // Disable pull-up resistor
            GPIO_PORTB_PDR_R &= ~pin_mask; // Disable pull-down resistor
            switch(pull) {
                case GPIO_PULL_UP: GPIO_PORTB_PUR_R |= pin_mask; break; // Enable pull-up resistor
                case GPIO_PULL_DOWN: GPIO_PORTB_PDR_R |= pin_mask; break; // Enable pull-down resistor
                case GPIO_PULL_NONE: break; // No pull resistor
                default: return GPIO_STATUS_INVALID_PARAM;
            }
            break;
        
        // Add new cases for other ports as needed
        default: return GPIO_STATUS_INVALID_PARAM;
    }
    handle->config.pull = pull;
    return GPIO_STATUS_SUCCESS;
}

/*
 * @brief Set GPIO Alternate Function
 *
 */
static GPIO_Status_t GPIO_SetAlternateFunction(GPIO_Handle_t* handle, uint8_t alt_function)
{
    if(!GPIO_IsValidHandle(handle) || alt_function > 15) {
        return GPIO_STATUS_INVALID_PARAM;
    }

    uint32_t pin_mask = GPIO_GetPinMask(handle->pin);

    // Configure alternate function based on port
    switch(handle->port) {
        case GPIO_PORT_A:
            GPIO_PORTA_AFSEL_R |= pin_mask; // Enable alternate function
            GPIO_PORTA_PCTL_R &= ~(0xF << (handle->pin * 4)); // Clear existing bits
            GPIO_PORTA_PCTL_R |= (alt_function << (handle->pin * 4)); // Set new function
            break;
        case GPIO_PORT_B:
            GPIO_PORTB_AFSEL_R |= pin_mask; // Enable alternate function
            GPIO_PORTB_PCTL_R &= ~(0xF << (handle->pin * 4)); // Clear existing bits
            GPIO_PORTB_PCTL_R |= (alt_function << (handle->pin * 4)); // Set new function
            break;
        // Add new cases for other ports as needed
        default: return GPIO_STATUS_INVALID_PARAM;
    }

    handle->config.alternate_function = alt_function;
    handle->config.mode = GPIO_MODE_ALTERNATE;

    return GPIO_STATUS_SUCCESS;
}

/* Public API Instance ----------------------------------------------------------------------------*/
/*
 * @brief Public GPIO API Instance
 *
 */
const GPIO_Interface_t GPIO_API = {
    .Init = GPIO_Init,
    .DeInit = GPIO_DeInit,
    .SetPin = GPIO_SetPin,
    .ClearPin = GPIO_ClearPin,
    .TogglePin = GPIO_TogglePin,
    .WritePin = GPIO_WritePin,
    .ReadPin = GPIO_ReadPin,
    .ConfigureInterrupt = GPIO_ConfigureInterrupt,
    .EnableInterrupt = GPIO_EnableInterrupt,
    .DisableInterrupt = GPIO_DisableInterrupt,
    .DriveStrength = GPIO_SetDriveStrength,
    .SetPullMode = GPIO_SetPullMode,
    .SetAlternateFunction = GPIO_SetAlternateFunction
};

/* Interrupt Service Routines ---------------------------------------------------------------------*/

/*
 * @brief Public GPIO Port B Interrupt Handler
 *
 */
void GPIOPortB_Handler(void)
{
    // Check which pin triggered the interrupt
    uint32_t interrupt_status = GPIO_PORTB_MIS_R; // Get masked interrupt status
    uint8_t pin = 0;

    // Process each pin that triggered the interrupt
    for(pin=0; pin < GPIO_PIN_COUNT; pin++) {
        uint8_t pin_mask = (1U << pin);

        if(interrupt_status & pin_mask) {
            // Clear the interrupt
            GPIO_PORTB_ICR_R |= pin_mask;

            // Call the registered callback if it exists
            GPIO_Handle_t* handle = gpio_interrupt_handles[GPIO_PORT_B][pin];
            if(handle && handle->interrupt_callback) {
                handle->interrupt_callback();
            }
        }
    }
}


