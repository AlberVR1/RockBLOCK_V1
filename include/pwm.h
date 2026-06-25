/**
 * @file main.c
 * @author Alberto V�zquez
 *
 * @brief header file for PWM  signal control functionalities
 *
 * This header includes all prototype functions about PWM modules
 *
 * @version 0.11
 * @date 2025-07-02
 */


#ifndef INCLUDE_PWM_H_
#define INCLUDE_PWM_H_


/**
 * @addtogroup PWM signal control
 * @{
 */
/* Includes ----------------------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Public Defines ---------------------------------------------------------------------------------*/
#define PWM_MAX_CHANNELS    8
#define PWM_MAX_DUTY_CYCLE  100
#define PWM_MIN_DUTY_CYCLE  0

/* Public Enums -----------------------------------------------------------------------------------*/
typedef enum {
    PWM_PIN_M0_PB6 = 0, // Motion Control Module 0 PWM 0. This signal is controlled by Module 0 PWM Generator 0.
    PWM_PIN_M0_PB7 = 1, // Motion Control Module 0 PWM 1. This signal is controlled by Module 0 PWM Generator 0.
    PWM_PIN_M0_PB4 = 2, // Motion Control Module 0 PWM 2. This signal is controlled by Module 0 PWM Generator 1.
    PWM_PIN_M0_PB5 = 3, // Motion Control Module 0 PWM 3. This signal is controlled by Module 0 PWM Generator 1.
    PWM_PIN_M0_PE4 = 4, // Motion Control Module 0 PWM 4. This signal is controlled by Module 0 PWM Generator 2.
    PWM_PIN_M0_PE5 = 5, // Motion Control Module 0 PWM 5. This signal is controlled by Module 0 PWM Generator 2.
    PWM_PIN_M0_PC4 = 6, // Motion Control Module 0 PWM 6. This signal is controlled by Module 0 PWM Generator 3.
    PWM_PIN_M0_PD0 = 7, // Motion Control Module 0 PWM 6. This signal is controlled by Module 0 PWM Generator 3.
    PWM_PIN_M0_PC5 = 8, // Motion Control Module 0 PWM 7. This signal is controlled by Module 0 PWM Generator 3.
    PWM_PIN_M0_PD1 = 9, // Motion Control Module 0 PWM 7. This signal is controlled by Module 0 PWM Generator 3.
    PWM_PIN_M1_PD0 = 10,    // Motion Control Module 1 PWM 0. This signal is controlled by Module 1 PWM Generator 0.
    PWM_PIN_M1_PD1 = 11,    // Motion Control Module 1 PWM 1. This signal is controlled by Module 1 PWM Generator 0.
    PWM_PIN_M1_PA6 = 12,    // Motion Control Module 1 PWM 2. This signal is controlled by Module 1 PWM Generator 1.
    PWM_PIN_M1_PE4 = 13,    // Motion Control Module 1 PWM 2. This signal is controlled by Module 1 PWM Generator 1.
    PWM_PIN_M1_PA7 = 14,    // Motion Control Module 1 PWM 3. This signal is controlled by Module 1 PWM Generator 1.
    PWM_PIN_M1_PE5 = 15,    // Motion Control Module 1 PWM 3. This signal is controlled by Module 1 PWM Generator 1.
    PWM_PIN_M1_PF0 = 16,    // Motion Control Module 1 PWM 4. This signal is controlled by Module 1 PWM Generator 2.
    PWM_PIN_M1_PF1 = 17,    // Motion Control Module 1 PWM 5. This signal is controlled by Module 1 PWM Generator 2.
    PWM_PIN_M1_PF2 = 18,    // Motion Control Module 1 PWM 6. This signal is controlled by Module 1 PWM Generator 3.
    PWM_PIN_M1_PF3 = 19,    // Motion Control Module 1 PWM 7. This signal is controlled by Module 1 PWM Generator 3.
    PWM_PIN_COUNT
}PWM_Pin_t;

typedef enum {
    DRIVE_SELECT_2MA = 0,   // GPIO 2mA Drive Select
    DRIVE_SELECT_4MA = 1,   // GPIO 4mA Drive Select
    DRIVE_SELECT_8MA = 2,    // GPIO 8mA Drive Select
    DRIVE_SELECT_COUNT
}PWM_DriveSelect_t;

typedef enum {
    PWM_DIVISOR_NONE    = 0,    // No PWM Clock Divisor
    PWM_DIVISOR_2       = 1,    // PWM Clock Divisor /2
    PWM_DIVISOR_4       = 2,    // PWM Clock Divisor /4
    PWM_DIVISOR_8       = 3,    // PWM Clock Divisor /8
    PWM_DIVISOR_16      = 4,    // PWM Clock Divisor /16
    PWM_DIVISOR_32      = 5,    // PWM Clock Divisor /32
    PWM_DIVISOR_64      = 6,    // PWM Clock Divisor /64
    PWM_DIVISOR_COUNT
}PWM_Divisor_t;

typedef enum {
    DOWN_COUNT_MODE_PWM_SIGNAL = 0,   // PWM Signal generated in Down-Count Mode (Use full frequency and used in Motor Control, LED dimming, etc.)
    DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL = 1,   // PWM Negative Signal generated in Down-Count Mode (Inverted signal, Use full frequency and used in Motor Control, LED dimming, etc.)
    DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH = 2, // Toggle PWM Signal on match in Down-Count Mode (Used in Frequency generation applications)
    DOWN_COUNT_MODE_PWM_BYPASS_HIGH = 3, // Bypass PWM Signal in Down-Count Mode (Signal held High)
    DOWN_COUNT_MODE_PWM_SIGNAL_2 =  4, //Same as DOWN_COUNT_MODE_PWM_SIGNAL but with different action settings
    DOWN_COUNT_MODE_PWM_ALWAYS_LOW = 5, // PWM Signal held Low in Down-Count Mode
    UP_DOWN_COUNT_MODE_PWM_BYPASS = 6, // Bypass PWM Signal in Up-Down Count Mode (Signal held High)
    UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT = 7, // Phase Correct PWM Signal in Up-Down Count Mode (Frequency is cut in half)
    UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW = 8, // PWM Signal held Low in Up-Down Count Mode
    MODE_COUNT // Number of count modes
}PWM_CountMode_t;

typedef enum {
    PWM_STATUS_SUCCESS = 0, // Operation successful
    PWM_STATUS_ERROR,   // General error
    PWM_STATUS_PIN_ERROR,   // Invalid pin error
    PWM_STATUS_CONFIG_ERROR,    // Invalid configuration error
    PWM_STATUS_DRIVE_STRENGTH_ERROR,    // Invalid drive strength error
    PWM_STATUS_DIVISOR_ERROR,   // Invalid divisor error
    PWM_STATUS_FREQUENCY_ERROR, // Invalid frequency error
    PWM_STATUS_DUTY_CYCLE_ERROR,   // Invalid duty cycle error
    PWM_STATUS_MODE_COUNT_ERROR,    // Invalid count mode error
    PWM_STATUS_INVALID_PARAM,   // Invalid parameter error
    PWM_STATUS_NOT_INITIALIZED  // PWM module not initialized error
}PWM_Status_t;

/* Public Structures ------------------------------------------------------------------------------*/
typedef struct {
    uint32_t frequency_hz;  //Frequency in Hz
    uint8_t duty_cycle_percent; //Duty cycle as percentage (0-100)
    PWM_DriveSelect_t drive_strength; //Drive strength selection
    PWM_Divisor_t divisor; //PWM clock divisor
    PWM_CountMode_t count_mode; //PWM count mode selection
}PWM_Config_t;

typedef struct {
    PWM_Pin_t pin;  //PWM pin selection
    PWM_Config_t config;    //PWM configuration structure
    bool is_initialized;    //Initialization flag
}PWM_Handle_t;

/* Public Function Pointers Structure -------------------------------------------------------------*/
typedef struct {
    PWM_Status_t (*Init)(PWM_Handle_t* handle, PWM_Pin_t pin, const PWM_Config_t* config);
    PWM_Status_t (*SetDutyCycle)(PWM_Handle_t* handle, uint8_t duty_cycle_percent);
    PWM_Status_t (*Enable)(PWM_Handle_t* handle);
    PWM_Status_t (*Disable)(PWM_Handle_t* handle);
    PWM_Status_t (*DeInit)(PWM_Handle_t* handle);
}PWM_Interface_t;

/* Public API Instance ----------------------------------------------------------------------------*/
extern const PWM_Interface_t PWM_API;   // PWM API instance

/**
  * @}
  */

#endif /* INCLUDE_PWM_H_ */
