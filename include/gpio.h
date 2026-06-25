/**
 * @file gpio.h
 * @author Alberto Vazquez
 * @brief source file for managing GPIO peripheral
 *
 * This source file contains an API whitch includes and implementation functions
 * for managing TM4C123 GPIO peripheral.
 *
 * @version 0.11
 * @date 2025-09-17
 */

#ifndef INCLUDE_GPIO_H_
#define INCLUDE_GPIO_H_

/**
 * @addtogroup MCU GPIO
 * @{
 */

/* Includes ----------------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* Public Definitions -------------------------------------------------------------------------------------*/
#define float_t                 float
#define GPIO_MAX_PORTS          6
#define GPIO_MAX_PINS           8

/* Public Enums ------------------------------------------------------------------------------------*/
/**
 * @brief GPIO Port enumeration
 */
typedef enum {
    GPIO_PORT_A = 0,
    GPIO_PORT_B,
    GPIO_PORT_C,
    GPIO_PORT_D,
    GPIO_PORT_E,
    GPIO_PORT_F,
    GPIO_PORT_COUNT
}GPIO_Port_t;

/**
 * @brief GPIO Pin enumeration
 */
typedef enum {
    GPIO_PIN_0 = 0,
    GPIO_PIN_1,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_5,
    GPIO_PIN_6,
    GPIO_PIN_7,
    GPIO_PIN_COUNT
}GPIO_Pin_t;

/**
 * @brief GPIO Mode enumeration
 */
typedef enum {
    GPIO_MODE_INPUT = 0,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_ALTERNATE,
    GPIO_MODE_ANALOG
}GPIO_Mode_t;

/**
 * @brief GPIO State enumeration
 */
typedef enum {
    GPIO_STATE_LOW = 0,
    GPIO_STATE_HIGH
}GPIO_State_t;

/**
 * @brief GPIO Drive Strength enumeration
 */
typedef enum {
    GPIO_DRIVE_2MA = 0,
    GPIO_DRIVE_4MA,
    GPIO_DRIVE_8MA
}GPIO_Drive_t;

/**
 * @brief GPIO Pull configuration enumeration
 */
typedef enum {
    GPIO_PULL_NONE = 0,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN
}GPIO_Pull_t;

/**
 * @brief GPIO Interrupt Type enumeration
 */
typedef enum {
    GPIO_INT_NONE = 0,
    GPIO_INT_RISING_EDGE,
    GPIO_INT_FALLING_EDGE,
    GPIO_INT_BOTH_EDGES,
    GPIO_INT_LOW_LEVEL,
    GPIO_INT_HIGH_LEVEL
}GPIO_Interrupt_t;

/**
 * @brief GPIO Status Codes enumeration
 */
typedef enum {
    GPIO_STATUS_SUCCESS = 0,
    GPIO_STATUS_ERROR,
    GPIO_STATUS_INVALID_PARAM,
    GPIO_STATUS_NOT_INITIALIZED,
    GPIO_STATUS_BUSY
}GPIO_Status_t;

/* Public Structures -------------------------------------------------------------------------------*/
/**
 * @brief GPIO Configuration structure
 */
typedef struct {
    GPIO_Mode_t mode;                   // Input or Output
    GPIO_State_t initial_state;         // Just for output mode
    GPIO_Drive_t drive_strength;        // Drive strength
    GPIO_Pull_t pull;                   // Pull-up or Pull-down resistor
    GPIO_Interrupt_t interrupt_type;    // Interrupt configuration
    bool slew_rate_control;             // Slew rate control enable/disable
    bool open_drain;                    // Open-drain configuration
    uint8_t alternate_function;         // Alternate function number (0-7), 0 for GPIO
}GPIO_Config_t;

/**
 * @brief GPIO Handle structure
 */
typedef struct {
    GPIO_Port_t port;                   // Port letter
    GPIO_Pin_t pin;                     // Pin number
    GPIO_Config_t config;               // Pin configuration
    bool is_initialized;                // Initialization flag
    void (*interrupt_callback)(void);  // Interrupt callback function
    GPIO_State_t current_state;         // Current state of the pin
}GPIO_Handle_t;

/* Public Function Pointers Structure --------------------------------------------------------------*/
/**
 * @brief GPIO Interface structure containing all API functions
 */
typedef struct {
    // Basic Functions
    GPIO_Status_t (*Init)(GPIO_Handle_t* handle, GPIO_Port_t port, GPIO_Pin_t pin, const GPIO_Config_t* config);
    GPIO_Status_t (*DeInit)(GPIO_Handle_t* handle);
    // Output control
    GPIO_Status_t (*SetPin)(GPIO_Handle_t* handle);
    GPIO_Status_t (*ClearPin)(GPIO_Handle_t* handle);
    GPIO_Status_t (*TogglePin)(GPIO_Handle_t* handle);
    GPIO_Status_t (*WritePin)(GPIO_Handle_t* handle, GPIO_State_t state);
    // Read inputs
    GPIO_Status_t (*ReadPin)(GPIO_Handle_t* handle, GPIO_State_t* state);
    // Advanced functions
    GPIO_Status_t (*ConfigureInterrupt)(GPIO_Handle_t* handle, GPIO_Interrupt_t interrupt_type, void (*callback)(void));
    GPIO_Status_t (*EnableInterrupt)(GPIO_Handle_t* handle);
    GPIO_Status_t (*DisableInterrupt)(GPIO_Handle_t* handle);
    // Drive strength and pull resistor configuration
    GPIO_Status_t (*DriveStrength)(GPIO_Handle_t* handle, GPIO_Drive_t drive);
    GPIO_Status_t (*SetPullMode)(GPIO_Handle_t* handle, GPIO_Pull_t pull);
    // Alternate function configuration
    GPIO_Status_t (*SetAlternateFunction)(GPIO_Handle_t* handle, uint8_t af_num);
}GPIO_Interface_t;

/* Public API Instance -----------------------------------------------------------------------------*/
/**
 * @brief GPIO Public API Instance
 */
extern const GPIO_Interface_t GPIO_API;

/**
  * @}
*/

#endif /* INCLUDE_GPIO_H_ */
