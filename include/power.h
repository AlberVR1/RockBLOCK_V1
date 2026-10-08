#ifndef INCLUDE_POWER_H_
#define INCLUDE_POWER_H_

#include <stdint.h>

typedef enum {
    POWER_STATUS_SUCCESS = 0,
    POWER_STATUS_ERROR
} Power_Status_t;

/* Keep UART1, UART2, and their GPIO ports clocked while the core sleeps. */
Power_Status_t Power_ConfigureSleepWakeSources(void);

/* Enter normal Cortex-M Sleep until an enabled interrupt occurs. */
void Power_EnterSleep(void);

#endif /* INCLUDE_POWER_H_ */
