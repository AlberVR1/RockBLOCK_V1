#include "power.h"
#include "tm4c123gh6pm.h"

#define POWER_SLEEP_GPIO_MASK (SYSCTL_SCGCGPIO_S1 | SYSCTL_SCGCGPIO_S3)
#define POWER_SLEEP_UART_MASK (SYSCTL_SCGCUART_S1 | SYSCTL_SCGCUART_S2)

Power_Status_t Power_ConfigureSleepWakeSources(void)
{
    /* UART1 receives the MCU Master wake byte; UART2 can receive SBDRING. */
    SYSCTL_SCGCGPIO_R |= POWER_SLEEP_GPIO_MASK;
    SYSCTL_SCGCUART_R |= POWER_SLEEP_UART_MASK;

    if((SYSCTL_SCGCGPIO_R & POWER_SLEEP_GPIO_MASK) != POWER_SLEEP_GPIO_MASK ||
       (SYSCTL_SCGCUART_R & POWER_SLEEP_UART_MASK) != POWER_SLEEP_UART_MASK) {
        return POWER_STATUS_ERROR;
    }

    return POWER_STATUS_SUCCESS;
}

void Power_EnterSleep(void)
{
    /* Keep SLEEPDEEP clear: UART RX interrupts remain available in Sleep. */
    NVIC_SYS_CTRL_R &= ~NVIC_SYS_CTRL_SLEEPDEEP;
    __asm("    DSB");
    __asm("    WFI");
    __asm("    ISB");
}
