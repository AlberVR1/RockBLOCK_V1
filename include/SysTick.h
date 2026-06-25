/*
 * SysTick.h
 *
 *  Created on: 12 feb 2026
 *      Author: Control1
 */

#ifndef INCLUDE_SYSTICK_H_
#define INCLUDE_SYSTICK_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief SysTick Status Codes enumeration
 */
typedef enum {
    SYSTICK_STATUS_SUCCESS = 0,
    SYSTICK_STATUS_HANDLER_NULL,
    SYSTICK_STATUS_ERROR
}SYSTICK_Status_t;

typedef enum {
    SYSTICK_EXIST,
    SYSTICK_NOT_EXIST
}SYSTICK_Exist_t;

/* Public Function Pointers Structure --------------------------------------------------------------*/
/**
 * @brief SysTick Interface structure containing all API functions
 */
typedef struct {
    SYSTICK_Status_t (*init)(void);
    SYSTICK_Status_t (*Stop_Count)(void);
    SYSTICK_Status_t (*Start_Count)(void);
    uint32_t (*milis)(void);
}SYSTICK_Interface_t;

/* Public API Instance -----------------------------------------------------------------------------*/
/**
 * @brief SYSTICK Public API Instance
 */
extern const SYSTICK_Interface_t SYSTICK_API;

#endif /* INCLUDE_SYSTICK_H_ */
