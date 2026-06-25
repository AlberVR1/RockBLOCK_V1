/**
 * @file PWM.c
 * @author Alberto V�zquez
 * @brief source file for MCU PWM Module x Configuration
 *
 *
 * @version 0.11
 * @date 2025-07-01
 */
#include "include/pwm.h"

/* Private Defines --------------------------------------------------------------------------------*/
#define DEFAULT_SYSTEM_CLOCK_HZ 16000000UL
#define DUTY_CYCLE_TO_DECIMAL(x) ((float)(x) / 100.0f)


//*****************************************************************************
//
// System Control registers (SYSCTL)
//
//*****************************************************************************
#define SYSCTL_RCC_R            (*((volatile uint32_t *)0x400FE060))
#define SYSCTL_RCC2_R           (*((volatile uint32_t *)0x400FE070))
#define SYSCTL_RCGCPWM_R        (*((volatile uint32_t *)0x400FE640))
#define SYSCTL_RCGCGPIO_R       (*((volatile uint32_t *)0x400FE608))
#define SYSCTL_PRGPIO_R         (*((volatile uint32_t *)0x400FEA08))
#define SYSCTL_PRPWM_R          (*((volatile uint32_t *)0x400FEA40))
//*****************************************************************************
//
// GPIO registers (PORTB)
//
//*****************************************************************************
#define GPIO_PORTB_AFSEL_R      (*((volatile uint32_t *)0x40005420))
#define GPIO_PORTB_DR2R_R       (*((volatile uint32_t *)0x40005500))
#define GPIO_PORTB_DR4R_R       (*((volatile uint32_t *)0x40005504))
#define GPIO_PORTB_DR8R_R       (*((volatile uint32_t *)0x40005508))
#define GPIO_PORTB_DEN_R        (*((volatile uint32_t *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile uint32_t *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile uint32_t *)0x4000552C))
//*****************************************************************************
//
// GPIO registers (PORTC)
//
//*****************************************************************************
#define GPIO_PORTC_AFSEL_R      (*((volatile uint32_t *)0x40006420))
#define GPIO_PORTC_DR2R_R       (*((volatile uint32_t *)0x40006500))
#define GPIO_PORTC_DR4R_R       (*((volatile uint32_t *)0x40006504))
#define GPIO_PORTC_DR8R_R       (*((volatile uint32_t *)0x40006508))
#define GPIO_PORTC_DEN_R        (*((volatile uint32_t *)0x4000651C))
#define GPIO_PORTC_AMSEL_R      (*((volatile uint32_t *)0x40006528))
#define GPIO_PORTC_PCTL_R       (*((volatile uint32_t *)0x4000652C))
//*****************************************************************************
//
// GPIO registers (PORTD)
//
//*****************************************************************************
#define GPIO_PORTD_AFSEL_R      (*((volatile uint32_t *)0x40007420))
#define GPIO_PORTD_DR2R_R       (*((volatile uint32_t *)0x40007500))
#define GPIO_PORTD_DR4R_R       (*((volatile uint32_t *)0x40007504))
#define GPIO_PORTD_DR8R_R       (*((volatile uint32_t *)0x40007508))
#define GPIO_PORTD_DEN_R        (*((volatile uint32_t *)0x4000751C))
#define GPIO_PORTD_AMSEL_R      (*((volatile uint32_t *)0x40007528))
#define GPIO_PORTD_PCTL_R       (*((volatile uint32_t *)0x4000752C))
//*****************************************************************************
//
// GPIO registers (PORTE)
//
//*****************************************************************************
#define GPIO_PORTE_AFSEL_R      (*((volatile uint32_t *)0x40024420))
#define GPIO_PORTE_DR2R_R       (*((volatile uint32_t *)0x40024500))
#define GPIO_PORTE_DR4R_R       (*((volatile uint32_t *)0x40024504))
#define GPIO_PORTE_DR8R_R       (*((volatile uint32_t *)0x40024508))
#define GPIO_PORTE_DEN_R        (*((volatile uint32_t *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile uint32_t *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile uint32_t *)0x4002452C))
//*****************************************************************************
//
// GPIO registers (PORTF)
//
//*****************************************************************************
#define GPIO_PORTF_AFSEL_R      (*((volatile uint32_t *)0x40025420))
#define GPIO_PORTF_DR2R_R       (*((volatile uint32_t *)0x40025500))
#define GPIO_PORTF_DR4R_R       (*((volatile uint32_t *)0x40025504))
#define GPIO_PORTF_DR8R_R       (*((volatile uint32_t *)0x40025508))
#define GPIO_PORTF_DEN_R        (*((volatile uint32_t *)0x4002551C))
#define GPIO_PORTF_AMSEL_R      (*((volatile uint32_t *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile uint32_t *)0x4002552C))
//*****************************************************************************
//
// PWM registers (PWM0)
//
//*****************************************************************************
#define PWM0_ENABLE_R           (*((volatile uint32_t *)0x40028008))
#define PWM0_0_CTL_R            (*((volatile uint32_t *)0x40028040))
#define PWM0_0_LOAD_R           (*((volatile uint32_t *)0x40028050))
#define PWM0_0_CMPA_R           (*((volatile uint32_t *)0x40028058))
#define PWM0_0_CMPB_R           (*((volatile uint32_t *)0x4002805C))
#define PWM0_0_GENA_R           (*((volatile uint32_t *)0x40028060))
#define PWM0_0_GENB_R           (*((volatile uint32_t *)0x40028064))
#define PWM0_1_CTL_R            (*((volatile uint32_t *)0x40028080))
#define PWM0_1_LOAD_R           (*((volatile uint32_t *)0x40028090))
#define PWM0_1_CMPA_R           (*((volatile uint32_t *)0x40028098))
#define PWM0_1_CMPB_R           (*((volatile uint32_t *)0x4002809C))
#define PWM0_1_GENA_R           (*((volatile uint32_t *)0x400280A0))
#define PWM0_1_GENB_R           (*((volatile uint32_t *)0x400280A4))
#define PWM0_2_CTL_R            (*((volatile uint32_t *)0x400280C0))
#define PWM0_2_LOAD_R           (*((volatile uint32_t *)0x400280D0))
#define PWM0_2_CMPA_R           (*((volatile uint32_t *)0x400280D8))
#define PWM0_2_CMPB_R           (*((volatile uint32_t *)0x400280DC))
#define PWM0_2_GENA_R           (*((volatile uint32_t *)0x400280E0))
#define PWM0_2_GENB_R           (*((volatile uint32_t *)0x400280E4))
#define PWM0_3_CTL_R            (*((volatile uint32_t *)0x40028100))
#define PWM0_3_LOAD_R           (*((volatile uint32_t *)0x40028110))
#define PWM0_3_CMPA_R           (*((volatile uint32_t *)0x40028118))
#define PWM0_3_CMPB_R           (*((volatile uint32_t *)0x4002811C))
#define PWM0_3_GENA_R           (*((volatile uint32_t *)0x40028120))
#define PWM0_3_GENB_R           (*((volatile uint32_t *)0x40028124))
//*****************************************************************************
//
// PWM registers (PWM1)
//
//*****************************************************************************
#define PWM1_ENABLE_R           (*((volatile uint32_t *)0x40029008))
#define PWM1_0_CTL_R            (*((volatile uint32_t *)0x40029040))
#define PWM1_0_LOAD_R           (*((volatile uint32_t *)0x40029050))
#define PWM1_0_CMPA_R           (*((volatile uint32_t *)0x40029058))
#define PWM1_0_CMPB_R           (*((volatile uint32_t *)0x4002905C))
#define PWM1_0_GENA_R           (*((volatile uint32_t *)0x40029060))
#define PWM1_0_GENB_R           (*((volatile uint32_t *)0x40029064))
#define PWM1_1_CTL_R            (*((volatile uint32_t *)0x40029080))
#define PWM1_1_LOAD_R           (*((volatile uint32_t *)0x40029090))
#define PWM1_1_CMPA_R           (*((volatile uint32_t *)0x40029098))
#define PWM1_1_CMPB_R           (*((volatile uint32_t *)0x4002909C))
#define PWM1_1_GENA_R           (*((volatile uint32_t *)0x400290A0))
#define PWM1_1_GENB_R           (*((volatile uint32_t *)0x400290A4))
#define PWM1_2_CTL_R            (*((volatile uint32_t *)0x400290C0))
#define PWM1_2_LOAD_R           (*((volatile uint32_t *)0x400290D0))
#define PWM1_2_CMPA_R           (*((volatile uint32_t *)0x400290D8))
#define PWM1_2_CMPB_R           (*((volatile uint32_t *)0x400290DC))
#define PWM1_2_GENA_R           (*((volatile uint32_t *)0x400290E0))
#define PWM1_2_GENB_R           (*((volatile uint32_t *)0x400290E4))
#define PWM1_3_CTL_R            (*((volatile uint32_t *)0x40029100))
#define PWM1_3_LOAD_R           (*((volatile uint32_t *)0x40029110))
#define PWM1_3_CMPA_R           (*((volatile uint32_t *)0x40029118))
#define PWM1_3_CMPB_R           (*((volatile uint32_t *)0x4002911C))
#define PWM1_3_GENA_R           (*((volatile uint32_t *)0x40029120))
#define PWM1_3_GENB_R           (*((volatile uint32_t *)0x40029124))
//*****************************************************************************
//
// The following are defines for the bit fields in the SYSCTL_RCC register.
//
//*****************************************************************************
#define SYSCTL_RCC_USEPWMDIV    0x00100000  // Enable PWM Clock Divisor
#define SYSCTL_RCC_PWMDIV_M     0x000E0000  // PWM Unit Clock Divisor
#define SYSCTL_RCC_PWMDIV_2     0x00000000  // PWM clock /2
#define SYSCTL_RCC_PWMDIV_4     0x00020000  // PWM clock /4
#define SYSCTL_RCC_PWMDIV_8     0x00040000  // PWM clock /8
#define SYSCTL_RCC_PWMDIV_16    0x00060000  // PWM clock /16
#define SYSCTL_RCC_PWMDIV_32    0x00080000  // PWM clock /32
#define SYSCTL_RCC_PWMDIV_64    0x000A0000  // PWM clock /64
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
// The following are defines for the bit fields in the SYSCTL_RCGCPWM register.
//
//*****************************************************************************
#define SYSCTL_RCGCPWM_R1       0x00000002  // PWM Module 1 Run Mode Clock
                                            // Gating Control
#define SYSCTL_RCGCPWM_R0       0x00000001  // PWM Module 0 Run Mode Clock
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
//*****************************************************************************
//
// The following are defines for the bit fields in the SYSCTL_PRPWM register.
//
//*****************************************************************************
#define SYSCTL_PRPWM_R1         0x00000002  // PWM Module 1 Peripheral Ready
#define SYSCTL_PRPWM_R0         0x00000001  // PWM Module 0 Peripheral Ready
//*****************************************************************************
//
// The following are defines for the bit fields in the GPIO_PCTL register for
// port B.
//
//*****************************************************************************
#define GPIO_PCTL_PB7_M         0xF0000000  // PB7 Mask
#define GPIO_PCTL_PB7_M0PWM1    0x40000000  // M0PWM1 on PB7
#define GPIO_PCTL_PB6_M         0x0F000000  // PB6 Mask
#define GPIO_PCTL_PB6_M0PWM0    0x04000000  // M0PWM0 on PB6
#define GPIO_PCTL_PB5_M         0x00F00000  // PB5 Mask
#define GPIO_PCTL_PB5_M0PWM3    0x00400000  // M0PWM3 on PB5
#define GPIO_PCTL_PB4_M         0x000F0000  // PB4 Mask
#define GPIO_PCTL_PB4_M0PWM2    0x00040000  // M0PWM2 on PB4
//*****************************************************************************
//
// The following are defines for the bit fields in the GPIO_PCTL register for
// port C.
//
//*****************************************************************************
#define GPIO_PCTL_PC5_M         0x00F00000  // PC5 Mask
#define GPIO_PCTL_PC5_M0PWM7    0x00400000  // M0PWM7 on PC5
#define GPIO_PCTL_PC4_M         0x000F0000  // PC4 Mask
#define GPIO_PCTL_PC4_M0PWM6    0x00040000  // M0PWM6 on PC4
//*****************************************************************************
//
// The following are defines for the bit fields in the GPIO_PCTL register for
// port D.
//
//*****************************************************************************
#define GPIO_PCTL_PD6_M0FAULT0  0x04000000  // M0FAULT0 on PD6
#define GPIO_PCTL_PD2_M0FAULT0  0x00000400  // M0FAULT0 on PD2
#define GPIO_PCTL_PD1_M         0x000000F0  // PD1 Mask
#define GPIO_PCTL_PD1_M0PWM7    0x00000040  // M0PWM7 on PD1
#define GPIO_PCTL_PD1_M1PWM1    0x00000050  // M1PWM1 on PD1
#define GPIO_PCTL_PD0_M         0x0000000F  // PD0 Mask
#define GPIO_PCTL_PD0_M0PWM6    0x00000004  // M0PWM6 on PD0
#define GPIO_PCTL_PD0_M1PWM0    0x00000005  // M1PWM0 on PD0
//*****************************************************************************
//
// The following are defines for the bit fields in the GPIO_PCTL register for
// port E.
//
//*****************************************************************************
#define GPIO_PCTL_PE5_M         0x00F00000  // PE5 Mask
#define GPIO_PCTL_PE5_M0PWM5    0x00400000  // M0PWM5 on PE5
#define GPIO_PCTL_PE5_M1PWM3    0x00500000  // M1PWM3 on PE5
#define GPIO_PCTL_PE4_M         0x000F0000  // PE4 Mask
#define GPIO_PCTL_PE4_M0PWM4    0x00040000  // M0PWM4 on PE4
#define GPIO_PCTL_PE4_M1PWM2    0x00050000  // M1PWM2 on PE4
//*****************************************************************************
//
// The following are defines for the bit fields in the GPIO_PCTL register for
// port F.
//
//*****************************************************************************
#define GPIO_PCTL_PF4_M         0x000F0000  // PF4 Mask
#define GPIO_PCTL_PF4_M1FAULT0  0x00050000  // M1FAULT0 on PF4
#define GPIO_PCTL_PF3_M         0x0000F000  // PF3 Mask
#define GPIO_PCTL_PF3_M1PWM7    0x00005000  // M1PWM7 on PF3
#define GPIO_PCTL_PF2_M         0x00000F00  // PF2 Mask
#define GPIO_PCTL_PF2_M0FAULT0  0x00000400  // M0FAULT0 on PF2
#define GPIO_PCTL_PF2_M1PWM6    0x00000500  // M1PWM6 on PF2
#define GPIO_PCTL_PF1_M         0x000000F0  // PF1 Mask
#define GPIO_PCTL_PF1_M1PWM5    0x00000050  // M1PWM5 on PF1
#define GPIO_PCTL_PF0_M         0x0000000F  // PF0 Mask
#define GPIO_PCTL_PF0_M1PWM4    0x00000005  // M1PWM4 on PF0
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_0_CTL register.
//
//*****************************************************************************
#define PWM_0_CTL_MODE          0x00000002  // Counter Mode
#define PWM_0_CTL_ENABLE        0x00000001  // PWM Block Enable
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_1_CTL register.
//
//*****************************************************************************
#define PWM_1_CTL_MODE          0x00000002  // Counter Mode
#define PWM_1_CTL_ENABLE        0x00000001  // PWM Block Enable
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_2_CTL register.
//
//*****************************************************************************
#define PWM_2_CTL_MODE          0x00000002  // Counter Mode
#define PWM_2_CTL_ENABLE        0x00000001  // PWM Block Enable
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_3_CTL register.
//
//*****************************************************************************
#define PWM_3_CTL_MODE          0x00000002  // Counter Mode
#define PWM_3_CTL_ENABLE        0x00000001  // PWM Block Enable
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_CTL register.
//
//*****************************************************************************
#define PWM_CTL_GLOBALSYNC3     0x00000008  // Update PWM Generator 3
#define PWM_CTL_GLOBALSYNC2     0x00000004  // Update PWM Generator 2
#define PWM_CTL_GLOBALSYNC1     0x00000002  // Update PWM Generator 1
#define PWM_CTL_GLOBALSYNC0     0x00000001  // Update PWM Generator 0
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_ENABLE register.
//
//*****************************************************************************
#define PWM_ENABLE_PWM7EN       0x00000080  // MnPWM7 Output Enable
#define PWM_ENABLE_PWM6EN       0x00000040  // MnPWM6 Output Enable
#define PWM_ENABLE_PWM5EN       0x00000020  // MnPWM5 Output Enable
#define PWM_ENABLE_PWM4EN       0x00000010  // MnPWM4 Output Enable
#define PWM_ENABLE_PWM3EN       0x00000008  // MnPWM3 Output Enable
#define PWM_ENABLE_PWM2EN       0x00000004  // MnPWM2 Output Enable
#define PWM_ENABLE_PWM1EN       0x00000002  // MnPWM1 Output Enable
#define PWM_ENABLE_PWM0EN       0x00000001  // MnPWM0 Output Enable
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_0_GENA register.
//
//*****************************************************************************
#define PWM_0_GENA_ACTCMPBD_M   0x00000C00  // Action for Comparator B Down
#define PWM_0_GENA_ACTCMPBD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_0_GENA_ACTCMPBD_INV 0x00000400  // Invert pwmA
#define PWM_0_GENA_ACTCMPBD_ZERO                                              \
                                0x00000800  // Drive pwmA Low
#define PWM_0_GENA_ACTCMPBD_ONE 0x00000C00  // Drive pwmA High
#define PWM_0_GENA_ACTCMPBU_M   0x00000300  // Action for Comparator B Up
#define PWM_0_GENA_ACTCMPBU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_0_GENA_ACTCMPBU_INV 0x00000100  // Invert pwmA
#define PWM_0_GENA_ACTCMPBU_ZERO                                              \
                                0x00000200  // Drive pwmA Low
#define PWM_0_GENA_ACTCMPBU_ONE 0x00000300  // Drive pwmA High
#define PWM_0_GENA_ACTCMPAD_M   0x000000C0  // Action for Comparator A Down
#define PWM_0_GENA_ACTCMPAD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_0_GENA_ACTCMPAD_INV 0x00000040  // Invert pwmA
#define PWM_0_GENA_ACTCMPAD_ZERO                                              \
                                0x00000080  // Drive pwmA Low
#define PWM_0_GENA_ACTCMPAD_ONE 0x000000C0  // Drive pwmA High
#define PWM_0_GENA_ACTCMPAU_M   0x00000030  // Action for Comparator A Up
#define PWM_0_GENA_ACTCMPAU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_0_GENA_ACTCMPAU_INV 0x00000010  // Invert pwmA
#define PWM_0_GENA_ACTCMPAU_ZERO                                              \
                                0x00000020  // Drive pwmA Low
#define PWM_0_GENA_ACTCMPAU_ONE 0x00000030  // Drive pwmA High
#define PWM_0_GENA_ACTLOAD_M    0x0000000C  // Action for Counter=LOAD
#define PWM_0_GENA_ACTLOAD_NONE 0x00000000  // Do nothing
#define PWM_0_GENA_ACTLOAD_INV  0x00000004  // Invert pwmA
#define PWM_0_GENA_ACTLOAD_ZERO 0x00000008  // Drive pwmA Low
#define PWM_0_GENA_ACTLOAD_ONE  0x0000000C  // Drive pwmA High
#define PWM_0_GENA_ACTZERO_M    0x00000003  // Action for Counter=0
#define PWM_0_GENA_ACTZERO_NONE 0x00000000  // Do nothing
#define PWM_0_GENA_ACTZERO_INV  0x00000001  // Invert pwmA
#define PWM_0_GENA_ACTZERO_ZERO 0x00000002  // Drive pwmA Low
#define PWM_0_GENA_ACTZERO_ONE  0x00000003  // Drive pwmA High
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_0_GENB register.
//
//*****************************************************************************
#define PWM_0_GENB_ACTCMPBD_M   0x00000C00  // Action for Comparator B Down
#define PWM_0_GENB_ACTCMPBD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_0_GENB_ACTCMPBD_INV 0x00000400  // Invert pwmB
#define PWM_0_GENB_ACTCMPBD_ZERO                                              \
                                0x00000800  // Drive pwmB Low
#define PWM_0_GENB_ACTCMPBD_ONE 0x00000C00  // Drive pwmB High
#define PWM_0_GENB_ACTCMPBU_M   0x00000300  // Action for Comparator B Up
#define PWM_0_GENB_ACTCMPBU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_0_GENB_ACTCMPBU_INV 0x00000100  // Invert pwmB
#define PWM_0_GENB_ACTCMPBU_ZERO                                              \
                                0x00000200  // Drive pwmB Low
#define PWM_0_GENB_ACTCMPBU_ONE 0x00000300  // Drive pwmB High
#define PWM_0_GENB_ACTCMPAD_M   0x000000C0  // Action for Comparator A Down
#define PWM_0_GENB_ACTCMPAD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_0_GENB_ACTCMPAD_INV 0x00000040  // Invert pwmB
#define PWM_0_GENB_ACTCMPAD_ZERO                                              \
                                0x00000080  // Drive pwmB Low
#define PWM_0_GENB_ACTCMPAD_ONE 0x000000C0  // Drive pwmB High
#define PWM_0_GENB_ACTCMPAU_M   0x00000030  // Action for Comparator A Up
#define PWM_0_GENB_ACTCMPAU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_0_GENB_ACTCMPAU_INV 0x00000010  // Invert pwmB
#define PWM_0_GENB_ACTCMPAU_ZERO                                              \
                                0x00000020  // Drive pwmB Low
#define PWM_0_GENB_ACTCMPAU_ONE 0x00000030  // Drive pwmB High
#define PWM_0_GENB_ACTLOAD_M    0x0000000C  // Action for Counter=LOAD
#define PWM_0_GENB_ACTLOAD_NONE 0x00000000  // Do nothing
#define PWM_0_GENB_ACTLOAD_INV  0x00000004  // Invert pwmB
#define PWM_0_GENB_ACTLOAD_ZERO 0x00000008  // Drive pwmB Low
#define PWM_0_GENB_ACTLOAD_ONE  0x0000000C  // Drive pwmB High
#define PWM_0_GENB_ACTZERO_M    0x00000003  // Action for Counter=0
#define PWM_0_GENB_ACTZERO_NONE 0x00000000  // Do nothing
#define PWM_0_GENB_ACTZERO_INV  0x00000001  // Invert pwmB
#define PWM_0_GENB_ACTZERO_ZERO 0x00000002  // Drive pwmB Low
#define PWM_0_GENB_ACTZERO_ONE  0x00000003  // Drive pwmB High
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_1_GENA register.
//
//*****************************************************************************
#define PWM_1_GENA_ACTCMPBD_M   0x00000C00  // Action for Comparator B Down
#define PWM_1_GENA_ACTCMPBD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_1_GENA_ACTCMPBD_INV 0x00000400  // Invert pwmA
#define PWM_1_GENA_ACTCMPBD_ZERO                                              \
                                0x00000800  // Drive pwmA Low
#define PWM_1_GENA_ACTCMPBD_ONE 0x00000C00  // Drive pwmA High
#define PWM_1_GENA_ACTCMPBU_M   0x00000300  // Action for Comparator B Up
#define PWM_1_GENA_ACTCMPBU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_1_GENA_ACTCMPBU_INV 0x00000100  // Invert pwmA
#define PWM_1_GENA_ACTCMPBU_ZERO                                              \
                                0x00000200  // Drive pwmA Low
#define PWM_1_GENA_ACTCMPBU_ONE 0x00000300  // Drive pwmA High
#define PWM_1_GENA_ACTCMPAD_M   0x000000C0  // Action for Comparator A Down
#define PWM_1_GENA_ACTCMPAD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_1_GENA_ACTCMPAD_INV 0x00000040  // Invert pwmA
#define PWM_1_GENA_ACTCMPAD_ZERO                                              \
                                0x00000080  // Drive pwmA Low
#define PWM_1_GENA_ACTCMPAD_ONE 0x000000C0  // Drive pwmA High
#define PWM_1_GENA_ACTCMPAU_M   0x00000030  // Action for Comparator A Up
#define PWM_1_GENA_ACTCMPAU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_1_GENA_ACTCMPAU_INV 0x00000010  // Invert pwmA
#define PWM_1_GENA_ACTCMPAU_ZERO                                              \
                                0x00000020  // Drive pwmA Low
#define PWM_1_GENA_ACTCMPAU_ONE 0x00000030  // Drive pwmA High
#define PWM_1_GENA_ACTLOAD_M    0x0000000C  // Action for Counter=LOAD
#define PWM_1_GENA_ACTLOAD_NONE 0x00000000  // Do nothing
#define PWM_1_GENA_ACTLOAD_INV  0x00000004  // Invert pwmA
#define PWM_1_GENA_ACTLOAD_ZERO 0x00000008  // Drive pwmA Low
#define PWM_1_GENA_ACTLOAD_ONE  0x0000000C  // Drive pwmA High
#define PWM_1_GENA_ACTZERO_M    0x00000003  // Action for Counter=0
#define PWM_1_GENA_ACTZERO_NONE 0x00000000  // Do nothing
#define PWM_1_GENA_ACTZERO_INV  0x00000001  // Invert pwmA
#define PWM_1_GENA_ACTZERO_ZERO 0x00000002  // Drive pwmA Low
#define PWM_1_GENA_ACTZERO_ONE  0x00000003  // Drive pwmA High

//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_1_GENB register.
//
//*****************************************************************************
#define PWM_1_GENB_ACTCMPBD_M   0x00000C00  // Action for Comparator B Down
#define PWM_1_GENB_ACTCMPBD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_1_GENB_ACTCMPBD_INV 0x00000400  // Invert pwmB
#define PWM_1_GENB_ACTCMPBD_ZERO                                              \
                                0x00000800  // Drive pwmB Low
#define PWM_1_GENB_ACTCMPBD_ONE 0x00000C00  // Drive pwmB High
#define PWM_1_GENB_ACTCMPBU_M   0x00000300  // Action for Comparator B Up
#define PWM_1_GENB_ACTCMPBU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_1_GENB_ACTCMPBU_INV 0x00000100  // Invert pwmB
#define PWM_1_GENB_ACTCMPBU_ZERO                                              \
                                0x00000200  // Drive pwmB Low
#define PWM_1_GENB_ACTCMPBU_ONE 0x00000300  // Drive pwmB High
#define PWM_1_GENB_ACTCMPAD_M   0x000000C0  // Action for Comparator A Down
#define PWM_1_GENB_ACTCMPAD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_1_GENB_ACTCMPAD_INV 0x00000040  // Invert pwmB
#define PWM_1_GENB_ACTCMPAD_ZERO                                              \
                                0x00000080  // Drive pwmB Low
#define PWM_1_GENB_ACTCMPAD_ONE 0x000000C0  // Drive pwmB High
#define PWM_1_GENB_ACTCMPAU_M   0x00000030  // Action for Comparator A Up
#define PWM_1_GENB_ACTCMPAU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_1_GENB_ACTCMPAU_INV 0x00000010  // Invert pwmB
#define PWM_1_GENB_ACTCMPAU_ZERO                                              \
                                0x00000020  // Drive pwmB Low
#define PWM_1_GENB_ACTCMPAU_ONE 0x00000030  // Drive pwmB High
#define PWM_1_GENB_ACTLOAD_M    0x0000000C  // Action for Counter=LOAD
#define PWM_1_GENB_ACTLOAD_NONE 0x00000000  // Do nothing
#define PWM_1_GENB_ACTLOAD_INV  0x00000004  // Invert pwmB
#define PWM_1_GENB_ACTLOAD_ZERO 0x00000008  // Drive pwmB Low
#define PWM_1_GENB_ACTLOAD_ONE  0x0000000C  // Drive pwmB High
#define PWM_1_GENB_ACTZERO_M    0x00000003  // Action for Counter=0
#define PWM_1_GENB_ACTZERO_NONE 0x00000000  // Do nothing
#define PWM_1_GENB_ACTZERO_INV  0x00000001  // Invert pwmB
#define PWM_1_GENB_ACTZERO_ZERO 0x00000002  // Drive pwmB Low
#define PWM_1_GENB_ACTZERO_ONE  0x00000003  // Drive pwmB High
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_2_GENA register.
//
//*****************************************************************************
#define PWM_2_GENA_ACTCMPBD_M   0x00000C00  // Action for Comparator B Down
#define PWM_2_GENA_ACTCMPBD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_2_GENA_ACTCMPBD_INV 0x00000400  // Invert pwmA
#define PWM_2_GENA_ACTCMPBD_ZERO                                              \
                                0x00000800  // Drive pwmA Low
#define PWM_2_GENA_ACTCMPBD_ONE 0x00000C00  // Drive pwmA High
#define PWM_2_GENA_ACTCMPBU_M   0x00000300  // Action for Comparator B Up
#define PWM_2_GENA_ACTCMPBU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_2_GENA_ACTCMPBU_INV 0x00000100  // Invert pwmA
#define PWM_2_GENA_ACTCMPBU_ZERO                                              \
                                0x00000200  // Drive pwmA Low
#define PWM_2_GENA_ACTCMPBU_ONE 0x00000300  // Drive pwmA High
#define PWM_2_GENA_ACTCMPAD_M   0x000000C0  // Action for Comparator A Down
#define PWM_2_GENA_ACTCMPAD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_2_GENA_ACTCMPAD_INV 0x00000040  // Invert pwmA
#define PWM_2_GENA_ACTCMPAD_ZERO                                              \
                                0x00000080  // Drive pwmA Low
#define PWM_2_GENA_ACTCMPAD_ONE 0x000000C0  // Drive pwmA High
#define PWM_2_GENA_ACTCMPAU_M   0x00000030  // Action for Comparator A Up
#define PWM_2_GENA_ACTCMPAU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_2_GENA_ACTCMPAU_INV 0x00000010  // Invert pwmA
#define PWM_2_GENA_ACTCMPAU_ZERO                                              \
                                0x00000020  // Drive pwmA Low
#define PWM_2_GENA_ACTCMPAU_ONE 0x00000030  // Drive pwmA High
#define PWM_2_GENA_ACTLOAD_M    0x0000000C  // Action for Counter=LOAD
#define PWM_2_GENA_ACTLOAD_NONE 0x00000000  // Do nothing
#define PWM_2_GENA_ACTLOAD_INV  0x00000004  // Invert pwmA
#define PWM_2_GENA_ACTLOAD_ZERO 0x00000008  // Drive pwmA Low
#define PWM_2_GENA_ACTLOAD_ONE  0x0000000C  // Drive pwmA High
#define PWM_2_GENA_ACTZERO_M    0x00000003  // Action for Counter=0
#define PWM_2_GENA_ACTZERO_NONE 0x00000000  // Do nothing
#define PWM_2_GENA_ACTZERO_INV  0x00000001  // Invert pwmA
#define PWM_2_GENA_ACTZERO_ZERO 0x00000002  // Drive pwmA Low
#define PWM_2_GENA_ACTZERO_ONE  0x00000003  // Drive pwmA High

//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_2_GENB register.
//
//*****************************************************************************
#define PWM_2_GENB_ACTCMPBD_M   0x00000C00  // Action for Comparator B Down
#define PWM_2_GENB_ACTCMPBD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_2_GENB_ACTCMPBD_INV 0x00000400  // Invert pwmB
#define PWM_2_GENB_ACTCMPBD_ZERO                                              \
                                0x00000800  // Drive pwmB Low
#define PWM_2_GENB_ACTCMPBD_ONE 0x00000C00  // Drive pwmB High
#define PWM_2_GENB_ACTCMPBU_M   0x00000300  // Action for Comparator B Up
#define PWM_2_GENB_ACTCMPBU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_2_GENB_ACTCMPBU_INV 0x00000100  // Invert pwmB
#define PWM_2_GENB_ACTCMPBU_ZERO                                              \
                                0x00000200  // Drive pwmB Low
#define PWM_2_GENB_ACTCMPBU_ONE 0x00000300  // Drive pwmB High
#define PWM_2_GENB_ACTCMPAD_M   0x000000C0  // Action for Comparator A Down
#define PWM_2_GENB_ACTCMPAD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_2_GENB_ACTCMPAD_INV 0x00000040  // Invert pwmB
#define PWM_2_GENB_ACTCMPAD_ZERO                                              \
                                0x00000080  // Drive pwmB Low
#define PWM_2_GENB_ACTCMPAD_ONE 0x000000C0  // Drive pwmB High
#define PWM_2_GENB_ACTCMPAU_M   0x00000030  // Action for Comparator A Up
#define PWM_2_GENB_ACTCMPAU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_2_GENB_ACTCMPAU_INV 0x00000010  // Invert pwmB
#define PWM_2_GENB_ACTCMPAU_ZERO                                              \
                                0x00000020  // Drive pwmB Low
#define PWM_2_GENB_ACTCMPAU_ONE 0x00000030  // Drive pwmB High
#define PWM_2_GENB_ACTLOAD_M    0x0000000C  // Action for Counter=LOAD
#define PWM_2_GENB_ACTLOAD_NONE 0x00000000  // Do nothing
#define PWM_2_GENB_ACTLOAD_INV  0x00000004  // Invert pwmB
#define PWM_2_GENB_ACTLOAD_ZERO 0x00000008  // Drive pwmB Low
#define PWM_2_GENB_ACTLOAD_ONE  0x0000000C  // Drive pwmB High
#define PWM_2_GENB_ACTZERO_M    0x00000003  // Action for Counter=0
#define PWM_2_GENB_ACTZERO_NONE 0x00000000  // Do nothing
#define PWM_2_GENB_ACTZERO_INV  0x00000001  // Invert pwmB
#define PWM_2_GENB_ACTZERO_ZERO 0x00000002  // Drive pwmB Low
#define PWM_2_GENB_ACTZERO_ONE  0x00000003  // Drive pwmB High
//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_3_GENA register.
//
//*****************************************************************************
#define PWM_3_GENA_ACTCMPBD_M   0x00000C00  // Action for Comparator B Down
#define PWM_3_GENA_ACTCMPBD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_3_GENA_ACTCMPBD_INV 0x00000400  // Invert pwmA
#define PWM_3_GENA_ACTCMPBD_ZERO                                              \
                                0x00000800  // Drive pwmA Low
#define PWM_3_GENA_ACTCMPBD_ONE 0x00000C00  // Drive pwmA High
#define PWM_3_GENA_ACTCMPBU_M   0x00000300  // Action for Comparator B Up
#define PWM_3_GENA_ACTCMPBU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_3_GENA_ACTCMPBU_INV 0x00000100  // Invert pwmA
#define PWM_3_GENA_ACTCMPBU_ZERO                                              \
                                0x00000200  // Drive pwmA Low
#define PWM_3_GENA_ACTCMPBU_ONE 0x00000300  // Drive pwmA High
#define PWM_3_GENA_ACTCMPAD_M   0x000000C0  // Action for Comparator A Down
#define PWM_3_GENA_ACTCMPAD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_3_GENA_ACTCMPAD_INV 0x00000040  // Invert pwmA
#define PWM_3_GENA_ACTCMPAD_ZERO                                              \
                                0x00000080  // Drive pwmA Low
#define PWM_3_GENA_ACTCMPAD_ONE 0x000000C0  // Drive pwmA High
#define PWM_3_GENA_ACTCMPAU_M   0x00000030  // Action for Comparator A Up
#define PWM_3_GENA_ACTCMPAU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_3_GENA_ACTCMPAU_INV 0x00000010  // Invert pwmA
#define PWM_3_GENA_ACTCMPAU_ZERO                                              \
                                0x00000020  // Drive pwmA Low
#define PWM_3_GENA_ACTCMPAU_ONE 0x00000030  // Drive pwmA High
#define PWM_3_GENA_ACTLOAD_M    0x0000000C  // Action for Counter=LOAD
#define PWM_3_GENA_ACTLOAD_NONE 0x00000000  // Do nothing
#define PWM_3_GENA_ACTLOAD_INV  0x00000004  // Invert pwmA
#define PWM_3_GENA_ACTLOAD_ZERO 0x00000008  // Drive pwmA Low
#define PWM_3_GENA_ACTLOAD_ONE  0x0000000C  // Drive pwmA High
#define PWM_3_GENA_ACTZERO_M    0x00000003  // Action for Counter=0
#define PWM_3_GENA_ACTZERO_NONE 0x00000000  // Do nothing
#define PWM_3_GENA_ACTZERO_INV  0x00000001  // Invert pwmA
#define PWM_3_GENA_ACTZERO_ZERO 0x00000002  // Drive pwmA Low
#define PWM_3_GENA_ACTZERO_ONE  0x00000003  // Drive pwmA High

//*****************************************************************************
//
// The following are defines for the bit fields in the PWM_O_3_GENB register.
//
//*****************************************************************************
#define PWM_3_GENB_ACTCMPBD_M   0x00000C00  // Action for Comparator B Down
#define PWM_3_GENB_ACTCMPBD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_3_GENB_ACTCMPBD_INV 0x00000400  // Invert pwmB
#define PWM_3_GENB_ACTCMPBD_ZERO                                              \
                                0x00000800  // Drive pwmB Low
#define PWM_3_GENB_ACTCMPBD_ONE 0x00000C00  // Drive pwmB High
#define PWM_3_GENB_ACTCMPBU_M   0x00000300  // Action for Comparator B Up
#define PWM_3_GENB_ACTCMPBU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_3_GENB_ACTCMPBU_INV 0x00000100  // Invert pwmB
#define PWM_3_GENB_ACTCMPBU_ZERO                                              \
                                0x00000200  // Drive pwmB Low
#define PWM_3_GENB_ACTCMPBU_ONE 0x00000300  // Drive pwmB High
#define PWM_3_GENB_ACTCMPAD_M   0x000000C0  // Action for Comparator A Down
#define PWM_3_GENB_ACTCMPAD_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_3_GENB_ACTCMPAD_INV 0x00000040  // Invert pwmB
#define PWM_3_GENB_ACTCMPAD_ZERO                                              \
                                0x00000080  // Drive pwmB Low
#define PWM_3_GENB_ACTCMPAD_ONE 0x000000C0  // Drive pwmB High
#define PWM_3_GENB_ACTCMPAU_M   0x00000030  // Action for Comparator A Up
#define PWM_3_GENB_ACTCMPAU_NONE                                              \
                                0x00000000  // Do nothing
#define PWM_3_GENB_ACTCMPAU_INV 0x00000010  // Invert pwmB
#define PWM_3_GENB_ACTCMPAU_ZERO                                              \
                                0x00000020  // Drive pwmB Low
#define PWM_3_GENB_ACTCMPAU_ONE 0x00000030  // Drive pwmB High
#define PWM_3_GENB_ACTLOAD_M    0x0000000C  // Action for Counter=LOAD
#define PWM_3_GENB_ACTLOAD_NONE 0x00000000  // Do nothing
#define PWM_3_GENB_ACTLOAD_INV  0x00000004  // Invert pwmB
#define PWM_3_GENB_ACTLOAD_ZERO 0x00000008  // Drive pwmB Low
#define PWM_3_GENB_ACTLOAD_ONE  0x0000000C  // Drive pwmB High
#define PWM_3_GENB_ACTZERO_M    0x00000003  // Action for Counter=0
#define PWM_3_GENB_ACTZERO_NONE 0x00000000  // Do nothing
#define PWM_3_GENB_ACTZERO_INV  0x00000001  // Invert pwmB
#define PWM_3_GENB_ACTZERO_ZERO 0x00000002  // Drive pwmB Low
#define PWM_3_GENB_ACTZERO_ONE  0x00000003  // Drive pwmB High

/* Private Function Prototypes --------------------------------------------------------------------*/
static PWM_Status_t PWM_InitHardware(PWM_Handle_t* handle);
static PWM_Status_t PWM_ConfigureGPIO(PWM_Handle_t* handle);
static PWM_Status_t PWM_ConfigureDivisor(PWM_Divisor_t divisor);
static uint32_t PWM_CalculateLoadValue(uint32_t frequency_hz, PWM_Divisor_t divisor);
static uint32_t PWM_GetSystemClock(void);
static bool PWM_IsValidHandle(const PWM_Handle_t* handle);
static bool PWM_IsValidDutyCycle(uint8_t duty_cycle);
static bool PWM_IsValidFrequency(uint32_t frequency_hz);
static bool PWM_IsValidDriveStrength(const PWM_Config_t* config);
static bool PWM_IsValidCountMode(const PWM_Config_t* config);

/* Private Implementation -------------------------------------------------------------------------*/
/**
 * @brief Initialize PWM module and channel
 *
 */
static PWM_Status_t PWM_Init(PWM_Handle_t* handle, PWM_Pin_t pin, const PWM_Config_t* config)
{
    if(!handle || !config){
        return PWM_STATUS_INVALID_PARAM;
    }
    if(pin >= PWM_PIN_COUNT){
        return PWM_STATUS_PIN_ERROR;
    }
    if(!PWM_IsValidDutyCycle(config->duty_cycle_percent) || !PWM_IsValidFrequency(config->frequency_hz) ||
       !PWM_IsValidDriveStrength(config) || !PWM_IsValidCountMode(config)){
        return PWM_STATUS_CONFIG_ERROR;
    }

    //Initialize handle
    handle->pin = pin;
    handle->config = *config;
    handle->is_initialized = false;

    //Initialize hardware
    PWM_Status_t status = PWM_InitHardware(handle);
    if(status == PWM_STATUS_SUCCESS){
        handle->is_initialized = true;
    }

    return status;
}

/*
 * @brief set PWM duty cycle
 *
 * This function sets the duty cycle for the specified PWM handle,
 * The duty_cycle_percent(param) is based un percent(0-100).
 *
 * @param handle Pointer to PWM handle
 * @param duty_cycle_percent Duty cycle in percent (0-100)
 * @return PWM_Status_t Status of the operation
 *
 */
static PWM_Status_t PWM_SetDutyCycle(PWM_Handle_t* handle, uint8_t duty_cycle_percent)
{
    if(!PWM_IsValidHandle(handle)){
        return PWM_STATUS_INVALID_PARAM;
    }
    else if(!PWM_IsValidDutyCycle(duty_cycle_percent)){
        return PWM_STATUS_DUTY_CYCLE_ERROR;
    }

    uint32_t load_value = PWM_CalculateLoadValue(handle->config.frequency_hz, handle->config.divisor);
    uint32_t compare_value = (uint32_t)(load_value * DUTY_CYCLE_TO_DECIMAL(duty_cycle_percent));

    //Update hardware register based on module and channel
    switch(handle->pin)
    {
        case PWM_PIN_M0_PB6:
            PWM0_0_CMPA_R = compare_value;
            break;
        case PWM_PIN_M0_PB7:
            PWM0_0_CMPB_R = compare_value;
            break;
        case PWM_PIN_M0_PB4:
            PWM0_1_CMPA_R = compare_value;
            break;
        case PWM_PIN_M0_PB5:
            PWM0_1_CMPB_R = compare_value;
            break;
        case PWM_PIN_M0_PE4:
            PWM0_2_CMPA_R = compare_value;
            break;
        case PWM_PIN_M0_PE5:
            PWM0_2_CMPB_R = compare_value;
            break;
        case PWM_PIN_M0_PC4:
        case PWM_PIN_M0_PD0:
            PWM0_3_CMPA_R = compare_value;
            break;
        case PWM_PIN_M0_PC5:
        case PWM_PIN_M0_PD1:
            PWM0_3_CMPB_R = compare_value;
            break;
        case PWM_PIN_M1_PD0:
            PWM1_0_CMPA_R = compare_value;
            break;
        case PWM_PIN_M1_PD1:
            PWM1_0_CMPB_R = compare_value;
            break;
        //Add other pin mappings here
        default:
            return PWM_STATUS_PIN_ERROR;
    }

    //Add other module/channel combinations here


    handle->config.duty_cycle_percent = duty_cycle_percent;
    return PWM_STATUS_SUCCESS;
}

/*
 * @brief Enable PWM Output
 *
 */
static PWM_Status_t PWM_Enable(PWM_Handle_t* handle)
{
    if(!PWM_IsValidHandle(handle)){
        return PWM_STATUS_INVALID_PARAM;
    }

    switch(handle->pin)
    {
        case PWM_PIN_M0_PB6:
            PWM0_ENABLE_R |= PWM_ENABLE_PWM0EN;
            break;
        case PWM_PIN_M0_PB7:
            PWM0_ENABLE_R |= PWM_ENABLE_PWM1EN;
            break;
        case PWM_PIN_M0_PB4:
            PWM0_ENABLE_R |= PWM_ENABLE_PWM2EN;
            break;
        case PWM_PIN_M0_PB5:
            PWM0_ENABLE_R |= PWM_ENABLE_PWM3EN;
            break;
        case PWM_PIN_M0_PE4:
            PWM0_ENABLE_R |= PWM_ENABLE_PWM4EN;
            break;
        case PWM_PIN_M0_PE5:
            PWM0_ENABLE_R |= PWM_ENABLE_PWM5EN;
            break;
        case PWM_PIN_M0_PC4:
        case PWM_PIN_M0_PD0:
            PWM0_ENABLE_R |= PWM_ENABLE_PWM6EN;
            break;
        case PWM_PIN_M0_PC5:
        case PWM_PIN_M0_PD1:
            PWM0_ENABLE_R |= PWM_ENABLE_PWM7EN;
            break;
        case PWM_PIN_M1_PD0:
            PWM1_ENABLE_R |= PWM_ENABLE_PWM0EN;
            break;
        case PWM_PIN_M1_PD1:
            PWM1_ENABLE_R |= PWM_ENABLE_PWM1EN;
            break;
        //Add other pin mappings here
        default:
            return PWM_STATUS_PIN_ERROR;
    }


    return PWM_STATUS_SUCCESS;
}

/*
 * @brief Disable PWM Output
 *
 */
static PWM_Status_t PWM_Disable(PWM_Handle_t* handle)
{
    if(!PWM_IsValidHandle(handle)){
        return PWM_STATUS_INVALID_PARAM;
    }

    switch(handle->pin)
    {
        case PWM_PIN_M0_PB6:
            PWM0_ENABLE_R &= ~PWM_ENABLE_PWM0EN;
            break;
        case PWM_PIN_M0_PB7:
            PWM0_ENABLE_R &= ~PWM_ENABLE_PWM1EN;
            break;
        case PWM_PIN_M0_PB4:
            PWM0_ENABLE_R &= ~PWM_ENABLE_PWM2EN;
            break;
        case PWM_PIN_M0_PB5:
            PWM0_ENABLE_R &= ~PWM_ENABLE_PWM3EN;
            break;
        case PWM_PIN_M0_PE4:
            PWM0_ENABLE_R &= ~PWM_ENABLE_PWM4EN;
            break;
        case PWM_PIN_M0_PE5:
            PWM0_ENABLE_R &= ~PWM_ENABLE_PWM5EN;
            break;
        case PWM_PIN_M0_PC4:
        case PWM_PIN_M0_PD0:
            PWM0_ENABLE_R &= ~PWM_ENABLE_PWM6EN;
            break;
        case PWM_PIN_M0_PC5:
        case PWM_PIN_M0_PD1:
            PWM0_ENABLE_R &= ~PWM_ENABLE_PWM7EN;
            break;
        case PWM_PIN_M1_PD0:
            PWM1_ENABLE_R &= ~PWM_ENABLE_PWM0EN;
            break;
        case PWM_PIN_M1_PD1:
            PWM1_ENABLE_R &= ~PWM_ENABLE_PWM1EN;
            break;
        //Add other pin mappings here
        default:
            return PWM_STATUS_PIN_ERROR;
    }

    return PWM_STATUS_SUCCESS;
}

/*
 * @brief Deinitialize PWM module
 *
 */
static PWM_Status_t PWM_DeInit(PWM_Handle_t* handle){
    if(!PWM_IsValidHandle(handle)){
        return PWM_STATUS_INVALID_PARAM;
    }

    PWM_Disable(handle);
    //Add free resources like (Clock, GPIOs)

    return PWM_STATUS_SUCCESS;
}

/*
 * @brief Initialize PWM hardware
 * 
 * This function configures the GPIO pins, clock divisor and PWM (module, generator and PWM)
 * based on the provided handle.
 *
 * @param handle Pointer to PWM handle
 * @return PWM_Status_t Status of the initialization
 */
static PWM_Status_t PWM_InitHardware(PWM_Handle_t* handle)
{
    PWM_Status_t status;
    static uint32_t load_value = 0, compare_value = 0;

    //Configure clock divisor
    status = PWM_ConfigureDivisor(handle->config.divisor);
    if(status != PWM_STATUS_SUCCESS){
        return status;
    }


    //Configure GPIO
    status = PWM_ConfigureGPIO(handle);
    if(status != PWM_STATUS_SUCCESS){
        return status;
    }

    //Enable PWM module clock
    if(handle->pin <= PWM_PIN_M0_PD1){
        SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0; // Enable clock for PWM module 0
        while ((SYSCTL_PRPWM_R & SYSCTL_PRPWM_R0) == 0); // Wait for PWM module 0 to be ready
    } else if(handle->pin >= PWM_PIN_M1_PD0 && handle->pin <= PWM_PIN_M1_PF3){
        SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1; // Enable clock for PWM module 1
        while ((SYSCTL_PRPWM_R & SYSCTL_PRPWM_R1) == 0); // Wait for PWM module 1 to be ready
    }

    load_value = PWM_CalculateLoadValue(handle->config.frequency_hz, handle->config.divisor);  // Calculate load value based on frequency and divisor
    compare_value = (uint32_t)(load_value * DUTY_CYCLE_TO_DECIMAL(handle->config.duty_cycle_percent)); // Calculate compare value based on duty cycle
    switch(handle->pin){
        case PWM_PIN_M0_PB6:
            if(!(PWM0_0_CTL_R & PWM_0_CTL_ENABLE)){ // If PWM generator 0 is disabled
                PWM0_0_CTL_R = 0; // Disable PWM generator 0
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM0_0_CTL_R &= ~PWM_0_CTL_MODE; // Set PWM generator 0 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                    PWM0_0_CTL_R |= PWM_0_CTL_MODE; // Set PWM generator 0 to count-up/down mode
                }
                PWM0_0_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTZERO_ZERO;  //0x000000C2 Motor control: Generate PWM signal,
                                    //drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAD_ZERO | PWM_0_GENA_ACTZERO_ONE;  //0x00000083 Negative logic: Generate Negative PWM signal,
                                                                    //drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAD_INV| PWM_0_GENA_ACTZERO_INV;    //0x00000041 Toggle on match in frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTZERO_ONE;   //0x000000C3 ByPass PWM: Hold pwmA High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTLOAD_ZERO; //0x000000C8 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAD_ZERO | PWM_0_GENA_ACTZERO_ZERO;;    //0x00000082 Always low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM0_0_GENA_R = PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ZERO;    //0x000000B0 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }
            
            PWM0_0_CMPA_R = compare_value;  // Set the compare value
            PWM0_0_CTL_R |= PWM_0_CTL_ENABLE; // Enable PWM generator 0
            break;
        case PWM_PIN_M0_PB7:
            if(!(PWM0_0_CTL_R & PWM_0_CTL_ENABLE)){ // If PWM generator 0 is disabled
                PWM0_0_CTL_R = 0; // Disable PWM generator 0
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM0_0_CTL_R &= ~PWM_0_CTL_MODE; // Set PWM generator 0 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                    PWM0_0_CTL_R |= PWM_0_CTL_MODE; // Set PWM generator 0 to count-up/down mode
                }
                PWM0_0_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM0_0_GENB_R = PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTZERO_ZERO;    //0x00000C02 Motor control: Generate PWM signal
                                                                                    // drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM0_0_GENB_R = PWM_0_GENB_ACTCMPBD_ZERO | PWM_0_GENB_ACTZERO_ONE;    //0x00000803 Negative logic: Generate Negative PWM signal,
                                                                                    // drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM0_0_GENB_R =  PWM_0_GENB_ACTCMPBD_INV| PWM_0_GENB_ACTZERO_INV;//0x00000401 // Toggle on match Frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM0_0_GENB_R = PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTZERO_ONE;   //0x00000C03 ByPass PWM: Hold pwmB High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM0_0_GENB_R = PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTLOAD_ZERO; //0x0000C08 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM0_0_GENB_R = PWM_0_GENB_ACTCMPBD_ZERO | PWM_0_GENB_ACTZERO_ZERO; //0x00000802 Always Low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM0_0_GENB_R = PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ZERO;     //0x00000B00 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }

            PWM0_0_CMPB_R = compare_value;  // Set the compare value
            PWM0_0_CTL_R |= PWM_0_CTL_ENABLE; // Enable PWM generator 0
            break;
        case PWM_PIN_M0_PB4:
            if(!(PWM0_1_CTL_R & PWM_1_CTL_ENABLE)){ // If PWM generator 1 is disabled
                PWM0_1_CTL_R = 0; // Disable PWM generator 0
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM0_1_CTL_R &= ~PWM_1_CTL_MODE; // Set PWM generator 1 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                PWM0_1_CTL_R |= PWM_1_CTL_MODE; // Set PWM generator 1 to count-up/down mode
                }
                PWM0_1_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM0_1_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTZERO_ZERO;  //0x000000C2 Motor control: Generate PWM signal,
                                    //drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM0_1_GENA_R = PWM_1_GENA_ACTCMPAD_ZERO | PWM_1_GENA_ACTZERO_ONE;  //0x00000083 Negative logic: Generate Negative PWM signal,
                                                                    //drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM0_1_GENA_R = PWM_1_GENA_ACTCMPAD_INV| PWM_0_GENA_ACTZERO_INV;    //0x00000041 Toggle on match in frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM0_1_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTZERO_ONE;   //0x000000C3 ByPass PWM: Hold pwmA High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM0_1_GENA_R = PWM_1_GENA_ACTCMPAD_ONE | PWM_1_GENA_ACTLOAD_ZERO; //0x000000C8 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM0_1_GENA_R = PWM_1_GENA_ACTCMPAD_ZERO | PWM_1_GENA_ACTZERO_ZERO;;    //0x00000082 Always low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM0_1_GENA_R = PWM_1_GENA_ACTCMPAU_ONE | PWM_1_GENA_ACTCMPAD_ZERO;    //0x000000B0 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }
            
            PWM0_1_CMPA_R = compare_value;  // Set the compare value
            PWM0_1_CTL_R |= PWM_1_CTL_ENABLE; // Enable PWM generator 1
            break;
        case PWM_PIN_M0_PB5:
            if(!(PWM0_1_CTL_R & PWM_1_CTL_ENABLE)){ // If PWM generator 1 is disabled
                PWM0_1_CTL_R = 0; // Disable PWM generator 1
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM0_1_CTL_R &= ~PWM_1_CTL_MODE; // Set PWM generator 1 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                    PWM0_1_CTL_R |= PWM_1_CTL_MODE; // Set PWM generator 1 to count-up/down mode
                }
                PWM0_1_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM0_1_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTZERO_ZERO;    //0x00000C02 Motor control: Generate PWM signal
                                                                                    // drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM0_1_GENB_R = PWM_1_GENB_ACTCMPBD_ZERO | PWM_1_GENB_ACTZERO_ONE;    //0x00000803 Negative logic: Generate Negative PWM signal,
                                                                                    // drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM0_1_GENB_R =  PWM_1_GENB_ACTCMPBD_INV| PWM_1_GENB_ACTZERO_INV;//0x00000401 // Toggle on match Frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM0_1_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTZERO_ONE;   //0x00000C03 ByPass PWM: Hold pwmB High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM0_1_GENB_R = PWM_1_GENB_ACTCMPBD_ONE | PWM_1_GENB_ACTLOAD_ZERO; //0x0000C08 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM0_1_GENB_R = PWM_1_GENB_ACTCMPBD_ZERO | PWM_1_GENB_ACTZERO_ZERO; //0x00000802 Always Low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM0_1_GENB_R = PWM_0_GENB_ACTCMPBU_ONE | PWM_1_GENB_ACTCMPBD_ZERO;     //0x00000B00 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }


            PWM0_1_CMPB_R = compare_value;  // Set the compare value
            PWM0_1_CTL_R |= PWM_1_CTL_ENABLE; // Enable PWM generator 1
            break;
        case PWM_PIN_M0_PE4:
            if(!(PWM0_2_CTL_R & PWM_2_CTL_ENABLE)){ // If PWM generator 2 is disabled
                PWM0_2_CTL_R = 0; // Disable PWM generator 2
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM0_2_CTL_R &= ~PWM_2_CTL_MODE; // Set PWM generator 2 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                PWM0_2_CTL_R |= PWM_2_CTL_MODE; // Set PWM generator 2 to count-up/down mode
                }
                PWM0_2_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM0_2_GENA_R = PWM_2_GENA_ACTCMPAD_ONE | PWM_2_GENA_ACTZERO_ZERO;  //0x000000C2 Motor control: Generate PWM signal,
                                    //drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM0_2_GENA_R = PWM_2_GENA_ACTCMPAD_ZERO | PWM_2_GENA_ACTZERO_ONE;  //0x00000083 Negative logic: Generate Negative PWM signal,
                                                                    //drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM0_2_GENA_R = PWM_2_GENA_ACTCMPAD_INV| PWM_2_GENA_ACTZERO_INV;    //0x00000041 Toggle on match in frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM0_2_GENA_R = PWM_2_GENA_ACTCMPAD_ONE | PWM_2_GENA_ACTZERO_ONE;   //0x000000C3 ByPass PWM: Hold pwmA High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM0_2_GENA_R = PWM_2_GENA_ACTCMPAD_ONE | PWM_2_GENA_ACTLOAD_ZERO; //0x000000C8 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM0_2_GENA_R = PWM_2_GENA_ACTCMPAD_ZERO | PWM_2_GENA_ACTZERO_ZERO;;    //0x00000082 Always low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM0_2_GENA_R = PWM_2_GENA_ACTCMPAU_ONE | PWM_2_GENA_ACTCMPAD_ZERO;    //0x000000B0 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }
        

            PWM0_2_CMPA_R = compare_value;  // Set the compare value
            PWM0_2_CTL_R |= PWM_2_CTL_ENABLE; // Disable PWM generator 2
            break;
        case PWM_PIN_M0_PE5:
            if(!(PWM0_2_CTL_R & PWM_2_CTL_ENABLE)){ // If PWM generator 2 is disabled
                PWM0_2_CTL_R = 0; // Disable PWM generator 2
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM0_2_CTL_R &= ~PWM_2_CTL_MODE; // Set PWM generator 1 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                    PWM0_2_CTL_R |= PWM_2_CTL_MODE; // Set PWM generator 1 to count-up/down mode
                }
                PWM0_2_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM0_2_GENB_R = PWM_2_GENB_ACTCMPBD_ONE | PWM_2_GENB_ACTZERO_ZERO;    //0x00000C02 Motor control: Generate PWM signal
                                                                                    // drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM0_2_GENB_R = PWM_2_GENB_ACTCMPBD_ZERO | PWM_2_GENB_ACTZERO_ONE;    //0x00000803 Negative logic: Generate Negative PWM signal,
                                                                                    // drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM0_2_GENB_R =  PWM_2_GENB_ACTCMPBD_INV| PWM_2_GENB_ACTZERO_INV;//0x00000401 // Toggle on match Frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM0_2_GENB_R = PWM_2_GENB_ACTCMPBD_ONE | PWM_2_GENB_ACTZERO_ONE;   //0x00000C03 ByPass PWM: Hold pwmB High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM0_2_GENB_R = PWM_2_GENB_ACTCMPBD_ONE | PWM_2_GENB_ACTLOAD_ZERO; //0x0000C08 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM0_2_GENB_R = PWM_2_GENB_ACTCMPBD_ZERO | PWM_2_GENB_ACTZERO_ZERO; //0x00000802 Always Low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM0_2_GENB_R = PWM_2_GENB_ACTCMPBU_ONE | PWM_2_GENB_ACTCMPBD_ZERO;     //0x00000B00 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }
        

            PWM0_2_CMPB_R = compare_value;
            PWM0_2_CTL_R |= PWM_2_CTL_ENABLE; // Enable PWM generator 2
            break;
        case PWM_PIN_M0_PC4:
        case PWM_PIN_M0_PD0:
            if(!(PWM0_3_CTL_R & PWM_3_CTL_ENABLE)){ // If PWM generator 3 is disabled
                PWM0_3_CTL_R = 0; // Disable PWM generator 3
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM0_3_CTL_R &= ~PWM_3_CTL_MODE; // Set PWM generator 3 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                PWM0_3_CTL_R |= PWM_3_CTL_MODE; // Set PWM generator 3 to count-up/down mode
                }
                PWM0_3_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM0_3_GENA_R = PWM_3_GENA_ACTCMPAD_ONE | PWM_3_GENA_ACTZERO_ZERO;  //0x000000C2 Motor control: Generate PWM signal,
                                    //drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM0_3_GENA_R = PWM_3_GENA_ACTCMPAD_ZERO | PWM_3_GENA_ACTZERO_ONE;  //0x00000083 Negative logic: Generate Negative PWM signal,
                                                                    //drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM0_3_GENA_R = PWM_3_GENA_ACTCMPAD_INV| PWM_3_GENA_ACTZERO_INV;    //0x00000041 Toggle on match in frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM0_3_GENA_R = PWM_3_GENA_ACTCMPAD_ONE | PWM_3_GENA_ACTZERO_ONE;   //0x000000C3 ByPass PWM: Hold pwmA High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM0_3_GENA_R = PWM_3_GENA_ACTCMPAD_ONE | PWM_3_GENA_ACTLOAD_ZERO; //0x000000C8 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM0_3_GENA_R = PWM_3_GENA_ACTCMPAD_ZERO | PWM_3_GENA_ACTZERO_ZERO;;    //0x00000082 Always low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM0_3_GENA_R = PWM_3_GENA_ACTCMPAU_ONE | PWM_3_GENA_ACTCMPAD_ZERO;    //0x000000B0 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }
         


            PWM0_3_CMPA_R = compare_value;  // Set the compare value
            PWM0_3_CTL_R |= PWM_3_CTL_ENABLE; // Enable PWM generator 3
            break;
        case PWM_PIN_M0_PC5:
        case PWM_PIN_M0_PD1:
            if(!(PWM0_3_CTL_R & PWM_3_CTL_ENABLE)){ // If PWM generator 3 is disabled
                PWM0_3_CTL_R = 0; // Disable PWM generator 3
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM0_3_CTL_R &= ~PWM_3_CTL_MODE; // Set PWM generator 3 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                    PWM0_3_CTL_R |= PWM_3_CTL_MODE; // Set PWM generator 3 to count-up/down mode
                }
                PWM0_3_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM0_3_GENB_R = PWM_3_GENB_ACTCMPBD_ONE | PWM_3_GENB_ACTZERO_ZERO;    //0x00000C02 Motor control: Generate PWM signal
                                                                                    // drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM0_3_GENB_R = PWM_3_GENB_ACTCMPBD_ZERO | PWM_3_GENB_ACTZERO_ONE;    //0x00000803 Negative logic: Generate Negative PWM signal,
                                                                                    // drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM0_3_GENB_R =  PWM_3_GENB_ACTCMPBD_INV| PWM_3_GENB_ACTZERO_INV;//0x00000401 // Toggle on match Frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM0_3_GENB_R = PWM_3_GENB_ACTCMPBD_ONE | PWM_3_GENB_ACTZERO_ONE;   //0x00000C03 ByPass PWM: Hold pwmB High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM0_3_GENB_R = PWM_3_GENB_ACTCMPBD_ONE | PWM_3_GENB_ACTLOAD_ZERO; //0x0000C08 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM0_3_GENB_R = PWM_3_GENB_ACTCMPBD_ZERO | PWM_3_GENB_ACTZERO_ZERO; //0x00000802 Always Low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM0_3_GENB_R = PWM_3_GENB_ACTCMPBU_ONE | PWM_3_GENB_ACTCMPBD_ZERO;     //0x00000B00 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }
        


            PWM0_3_CMPB_R = compare_value;  // Set the compare value
            PWM0_3_CTL_R |= PWM_3_CTL_ENABLE; // Enable PWM generator 3
            break;
// Configuring PWM for Module 1
        case PWM_PIN_M1_PD0:
            if(!(PWM1_0_CTL_R & PWM_0_CTL_ENABLE)){ // If PWM generator 0 is disabled
                PWM1_0_CTL_R = 0; // Disable PWM generator 0
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM1_0_CTL_R &= ~PWM_0_CTL_MODE; // Set PWM generator 0 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                    PWM1_0_CTL_R |= PWM_0_CTL_MODE; // Set PWM generator 0 to count-up/down mode
                }
                PWM1_0_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM1_0_GENA_R = PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTZERO_ZERO;  //0x000000C2 Motor control: Generate PWM signal,
                                    //drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM1_0_GENA_R = PWM_0_GENA_ACTCMPAD_ZERO | PWM_0_GENA_ACTZERO_ONE;  //0x00000083 Negative logic: Generate Negative PWM signal,
                                                                    //drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM1_0_GENA_R = PWM_0_GENA_ACTCMPAD_INV| PWM_0_GENA_ACTZERO_INV;    //0x00000041 Toggle on match in frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM1_0_GENA_R = PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTZERO_ONE;   //0x000000C3 ByPass PWM: Hold pwmA High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM1_0_GENA_R = PWM_0_GENA_ACTCMPAD_ONE | PWM_0_GENA_ACTLOAD_ZERO; //0x000000C8 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM1_0_GENA_R = PWM_0_GENA_ACTCMPAD_ZERO | PWM_0_GENA_ACTZERO_ZERO;;    //0x00000082 Always low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM1_0_GENA_R = PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ZERO;    //0x000000B0 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }
        



            PWM1_0_CMPA_R = compare_value;  // Set the compare value
            PWM1_0_CTL_R |= PWM_0_CTL_ENABLE; // Enable PWM generator 0
            break;
        case PWM_PIN_M1_PD1: 
            if(!(PWM1_0_CTL_R & PWM_0_CTL_ENABLE)){ // If PWM generator 0 is disabled
                PWM1_0_CTL_R = 0; // Disable PWM generator 0
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM1_0_CTL_R &= ~PWM_0_CTL_MODE; // Set PWM generator 0 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                    PWM1_0_CTL_R |= PWM_0_CTL_MODE; // Set PWM generator 0 to count-up/down mode
                }
                PWM1_0_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM1_0_GENB_R = PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTZERO_ZERO;    //0x00000C02 Motor control: Generate PWM signal
                                                                                    // drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM1_0_GENB_R = PWM_0_GENB_ACTCMPBD_ZERO | PWM_0_GENB_ACTZERO_ONE;    //0x00000803 Negative logic: Generate Negative PWM signal,
                                                                                    // drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM1_0_GENB_R =  PWM_0_GENB_ACTCMPBD_INV| PWM_0_GENB_ACTZERO_INV;//0x00000401 // Toggle on match Frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM1_0_GENB_R = PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTZERO_ONE;   //0x00000C03 ByPass PWM: Hold pwmB High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM1_0_GENB_R = PWM_0_GENB_ACTCMPBD_ONE | PWM_0_GENB_ACTLOAD_ZERO; //0x0000C08 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM1_0_GENB_R = PWM_0_GENB_ACTCMPBD_ZERO | PWM_0_GENB_ACTZERO_ZERO; //0x00000802 Always Low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM1_0_GENB_R = PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ZERO;     //0x00000B00 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }
        



            PWM1_0_CMPB_R = compare_value;  // Set the compare value
            PWM1_0_CTL_R |= PWM_0_CTL_ENABLE; // Enable PWM generator 0
            break;
        //Add other pin mappings here
        /*case PWM_PIN_M1_PA6:
            break;
        case PWM_PIN_M1_PE4:
            break;
        case PWM_PIN_M1_PA7:
            break;
        case PWM_PIN_M1_PE5:
            break;
        case PWM_PIN_M1_PF0:
            break;
        case PWM_PIN_M1_PF1:
            break;*/
        case PWM_PIN_M1_PF2:
            if(!(PWM1_3_CTL_R & PWM_3_CTL_ENABLE)){ // If PWM generator 3 is disabled
                PWM1_3_CTL_R = 0; // Disable PWM generator 3
                if(handle->config.count_mode <= DOWN_COUNT_MODE_PWM_ALWAYS_LOW){
                    PWM1_3_CTL_R &= ~PWM_3_CTL_MODE; // Set PWM generator 3 to count-down mode
                } else if(handle->config.count_mode >= UP_DOWN_COUNT_MODE_PWM_BYPASS && handle->config.count_mode < MODE_COUNT){
                    PWM1_3_CTL_R |= PWM_3_CTL_MODE; // Set PWM generator 3 to count-up/down mode
                }
                PWM1_3_LOAD_R = load_value; // Set the load value
            }
            switch(handle->config.count_mode){
                case DOWN_COUNT_MODE_PWM_SIGNAL:
                PWM1_3_GENA_R = PWM_3_GENA_ACTCMPAD_ONE | PWM_3_GENA_ACTZERO_ZERO;    //0x00000C02 Motor control: Generate PWM signal
                                                                                    // drive high on compare match and low on counter zero (Issue PWM signal: Noise). Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_NEGATIVE_SIGNAL:
                PWM1_3_GENA_R = PWM_3_GENA_ACTCMPAD_ZERO | PWM_3_GENA_ACTZERO_ONE;    //0x00000803 Negative logic: Generate Negative PWM signal,
                                                                                    // drive low on compare match and high on counter zero. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_TOGGLE_ON_MATCH:
                PWM1_3_GENA_R =  PWM_3_GENA_ACTCMPAD_INV| PWM_3_GENA_ACTZERO_INV;//0x00000401 // Toggle on match Frequency: Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_BYPASS_HIGH:
                case UP_DOWN_COUNT_MODE_PWM_BYPASS:
                PWM1_3_GENA_R = PWM_3_GENA_ACTCMPAD_ONE | PWM_3_GENA_ACTZERO_ONE;   //0x00000C03 ByPass PWM: Hold pwmB High. Down-Count Mode & Up-Down Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_SIGNAL_2:
                PWM1_3_GENA_R = PWM_3_GENA_ACTCMPAD_ONE | PWM_3_GENA_ACTLOAD_ZERO; //0x0000C08 Down-Load PWM: As Motor control: Generate PWM signal. Down-Count Mode
                break;
                case DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                case UP_DOWN_COUNT_MODE_PWM_ALWAYS_LOW:
                PWM1_3_GENA_R = PWM_3_GENA_ACTCMPAD_ZERO | PWM_3_GENA_ACTZERO_ZERO; //0x00000802 Always Low: Signal held low Down-Count Mode & Up-Down Count Mode
                break;
                case UP_DOWN_COUNT_MODE_PWM_PHASE_CORRECT:
                PWM1_3_GENA_R = PWM_3_GENA_ACTCMPAU_ONE | PWM_3_GENA_ACTCMPAD_ZERO;     //0x00000B00 Phase correct PWM: Cut the frequency in half, Up-Down Count Mode
                break;
                default:
                return PWM_STATUS_MODE_COUNT_ERROR;
            }


            PWM1_3_CMPA_R = compare_value;  // Set the compare value
            PWM1_3_CTL_R |= PWM_3_CTL_ENABLE; // Enable PWM generator 3
            break;
        /*case PWM_PIN_M1_PF3:
            break;*/
        default:
            return PWM_STATUS_PIN_ERROR;
    }
    return PWM_STATUS_SUCCESS;
}

/*
 * @brief Configure GPIO for PWM channel.
 *
 */
static PWM_Status_t PWM_ConfigureGPIO(PWM_Handle_t* handle)
{
    switch(handle->pin){
        case PWM_PIN_M0_PB6:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;    // Enable clock for GPIO Port B
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R1) == 0);  // Wait for GPIO Port B to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTB_DR2R_R |= (1<<6); break;     //Enable 2mA drive strength on PB6
                case DRIVE_SELECT_4MA:
                    GPIO_PORTB_DR4R_R |= (1<<6); break;     //Enable 4mA drive strength on PB6
                case DRIVE_SELECT_8MA:
                    GPIO_PORTB_DR8R_R |= (1<<6); break;     //Enable 8mA drive strength on PB6
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTB_DEN_R   |= (1<<6);   //Enable digital output on PB6
            GPIO_PORTB_AFSEL_R |= (1<<6);   // Set PB6 as alternate function
            GPIO_PORTB_PCTL_R &= ~GPIO_PCTL_PB6_M;  // Clear PB6 PCTL bits
            GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB6_M0PWM0;  // Configure PB6 for M0PWM0
            break;
        case PWM_PIN_M0_PB7:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;    // Enable clock for GPIO Port B
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R1) == 0);  // Wait for GPIO Port B to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTB_DR2R_R |= (1<<7); break;     //Enable 2mA drive strength on PB7
                case DRIVE_SELECT_4MA:
                    GPIO_PORTB_DR4R_R |= (1<<7); break;     //Enable 4mA drive strength on PB7
                case DRIVE_SELECT_8MA:
                    GPIO_PORTB_DR8R_R |= (1<<7); break;     //Enable 8mA drive strength on PB7
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTB_DEN_R   |= (1<<7);   //Enable digital output on PB7
            GPIO_PORTB_AFSEL_R |= (1<<7);   // Set PB7 as alternate function
            GPIO_PORTB_PCTL_R &= ~GPIO_PCTL_PB7_M;  // Clear PB7 PCTL bits
            GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB7_M0PWM1;  // Configure PB7 for M0PWM1
            break;
        case PWM_PIN_M0_PB4:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;    // Enable clock for GPIO Port B
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R1) == 0);  // Wait for GPIO Port B to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTB_DR2R_R |= (1<<4); break;     //Enable 2mA drive strength on PB4
                case DRIVE_SELECT_4MA:
                    GPIO_PORTB_DR4R_R |= (1<<4); break;     //Enable 4mA drive strength on PB4
                case DRIVE_SELECT_8MA:
                    GPIO_PORTB_DR8R_R |= (1<<4); break;     //Enable 8mA drive strength on PB4
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTB_DEN_R   |= (1<<4);   //Enable digital output on PB4
            GPIO_PORTB_AFSEL_R |= (1<<4);   // Set PB7 as alternate function
            GPIO_PORTB_PCTL_R &= ~GPIO_PCTL_PB4_M;  // Clear PB4 PCTL bits
            GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB4_M0PWM2;  // Configure PB4 for M0PWM2
            break;
        case PWM_PIN_M0_PB5:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;    // Enable clock for GPIO Port B
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R1) == 0);  // Wait for GPIO Port B to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTB_DR2R_R |= (1<<5); break;     //Enable 2mA drive strength on PB5
                case DRIVE_SELECT_4MA:
                    GPIO_PORTB_DR4R_R |= (1<<5); break;     //Enable 4mA drive strength on PB5
                case DRIVE_SELECT_8MA:
                    GPIO_PORTB_DR8R_R |= (1<<5); break;     //Enable 8mA drive strength on PB5
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTB_DEN_R   |= (1<<5);   //Enable digital output on PB5
            GPIO_PORTB_AFSEL_R |= (1<<5);   // Set PB5 as alternate function
            GPIO_PORTB_PCTL_R &= ~GPIO_PCTL_PB5_M;  // Clear PB5 PCTL bits
            GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB5_M0PWM3;  // Configure PB5 for M0PWM3
            break;
        case PWM_PIN_M0_PE4:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;    // Enable clock for GPIO Port E
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R4) == 0);  // Wait for GPIO Port E to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTE_DR2R_R |= (1<<4); break;     //Enable 2mA drive strength on PE4
                case DRIVE_SELECT_4MA:
                    GPIO_PORTE_DR4R_R |= (1<<4); break;     //Enable 4mA drive strength on PE4
                case DRIVE_SELECT_8MA:
                    GPIO_PORTE_DR8R_R |= (1<<4); break;     //Enable 8mA drive strength on PE4
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTE_DEN_R   |= (1<<4);   //Enable digital output on PE4
            GPIO_PORTE_AFSEL_R |= (1<<4);   // Set PE4 as alternate function
            GPIO_PORTE_PCTL_R &= ~GPIO_PCTL_PE4_M;  // Clear PE4 PCTL bits
            GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE4_M0PWM4;  // Configure PE4 for M0PWM4
            break;
        case PWM_PIN_M0_PE5:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;    // Enable clock for GPIO Port E
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R4) == 0);  // Wait for GPIO Port E to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTE_DR2R_R |= (1<<5); break;     //Enable 2mA drive strength on PE5
                case DRIVE_SELECT_4MA:
                    GPIO_PORTE_DR4R_R |= (1<<5); break;     //Enable 4mA drive strength on PE5
                case DRIVE_SELECT_8MA:
                    GPIO_PORTE_DR8R_R |= (1<<5); break;     //Enable 8mA drive strength on PE5
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTE_DEN_R   |= (1<<5);   //Enable digital output on PE5
            GPIO_PORTE_AFSEL_R |= (1<<5);   // Set PE5 as alternate function
            GPIO_PORTE_PCTL_R &= ~GPIO_PCTL_PE5_M;  // Clear PE5 PCTL bits
            GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE5_M0PWM5;  // Configure PE5 for M0PWM5
            break;
        case PWM_PIN_M0_PC4:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;    // Enable clock for GPIO Port C
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R2) == 0);  // Wait for GPIO Port C to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTC_DR2R_R |= (1<<4); break;     //Enable 2mA drive strength on PC4
                case DRIVE_SELECT_4MA:
                    GPIO_PORTC_DR4R_R |= (1<<4); break;     //Enable 4mA drive strength on PC4
                case DRIVE_SELECT_8MA:
                    GPIO_PORTC_DR8R_R |= (1<<4); break;     //Enable 8mA drive strength on PC4
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTC_DEN_R   |= (1<<4);   //Enable digital output on PC4
            GPIO_PORTC_AFSEL_R |= (1<<4);   // Set PC4 as alternate function
            GPIO_PORTC_PCTL_R &= ~GPIO_PCTL_PC4_M;  // Clear PC4 PCTL bits
            GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC4_M0PWM6;  // Configure PC4 for M0PWM6
            break;
        case PWM_PIN_M0_PD0:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3;    // Enable clock for GPIO Port D
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R3) == 0);  // Wait for GPIO Port D to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTD_DR2R_R |= (1<<0); break;     //Enable 2mA drive strength on PD0
                case DRIVE_SELECT_4MA:
                    GPIO_PORTD_DR4R_R |= (1<<0); break;     //Enable 4mA drive strength on PD0
                case DRIVE_SELECT_8MA:
                    GPIO_PORTD_DR8R_R |= (1<<0); break;     //Enable 8mA drive strength on PD0
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTD_DEN_R   |= (1<<0);   //Enable digital output on PD0
            GPIO_PORTD_AFSEL_R |= (1<<0);   // Set PD0 as alternate function
            GPIO_PORTD_PCTL_R &= ~GPIO_PCTL_PD0_M;  // Clear PD0 PCTL bits
            GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD0_M0PWM6;  // Configure PD0 for M0PWM6
            break;
        case PWM_PIN_M0_PC5:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;    // Enable clock for GPIO Port C
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R2) == 0);  // Wait for GPIO Port C to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTC_DR2R_R |= (1<<5); break;     //Enable 2mA drive strength on PC5
                case DRIVE_SELECT_4MA:
                    GPIO_PORTC_DR4R_R |= (1<<5); break;     //Enable 4mA drive strength on PC5
                case DRIVE_SELECT_8MA:
                    GPIO_PORTC_DR8R_R |= (1<<5); break;     //Enable 8mA drive strength on PC5
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTC_DEN_R   |= (1<<5);   //Enable digital output on PC5
            GPIO_PORTC_AFSEL_R |= (1<<5);   // Set PC5 as alternate function
            GPIO_PORTC_PCTL_R &= ~GPIO_PCTL_PC5_M;  // Clear PC5 PCTL bits
            GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC5_M0PWM7;  // Configure PC5 for M0PWM7
            break;
        case PWM_PIN_M0_PD1:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3;    // Enable clock for GPIO Port D
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R3) == 0);  // Wait for GPIO Port D to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTD_DR2R_R |= (1<<1); break;     //Enable 2mA drive strength on PD1
                case DRIVE_SELECT_4MA:
                    GPIO_PORTD_DR4R_R |= (1<<1); break;     //Enable 4mA drive strength on PD1
                case DRIVE_SELECT_8MA:
                    GPIO_PORTD_DR8R_R |= (1<<1); break;     //Enable 8mA drive strength on PD1
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTD_DEN_R   |= (1<<1);   //Enable digital output on PD1
            GPIO_PORTD_AFSEL_R |= (1<<1);   // Set PD1 as alternate function
            GPIO_PORTD_PCTL_R &= ~GPIO_PCTL_PD1_M;  // Clear PD1 PCTL bits
            GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD1_M0PWM7;  // Configure PD1 for M0PWM7
            break;
        case PWM_PIN_M1_PD0:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3;    // Enable clock for GPIO Port D
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R3) == 0);  // Wait for GPIO Port D to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTD_DR2R_R |= (1<<0); break;     //Enable 2mA drive strength on PD0
                case DRIVE_SELECT_4MA:
                    GPIO_PORTD_DR4R_R |= (1<<0); break;     //Enable 4mA drive strength on PD0
                case DRIVE_SELECT_8MA:
                    GPIO_PORTD_DR8R_R |= (1<<0); break;     //Enable 8mA drive strength on PD0
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTD_DEN_R   |= (1<<0);   //Enable digital output on PD0
            GPIO_PORTD_AFSEL_R |= (1<<0);   // Set PD0 as alternate function
            GPIO_PORTD_PCTL_R &= ~GPIO_PCTL_PD0_M;  // Clear PD0 PCTL bits
            GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD0_M1PWM0;  // Configure PD0 for M1PWM0
            break;
        case PWM_PIN_M1_PD1:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3;    // Enable clock for GPIO Port D
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R3) == 0);  // Wait for GPIO Port D to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTD_DR2R_R |= (1<<1); break;     //Enable 2mA drive strength on PD1
                case DRIVE_SELECT_4MA:
                    GPIO_PORTD_DR4R_R |= (1<<1); break;     //Enable 4mA drive strength on PD1
                case DRIVE_SELECT_8MA:
                    GPIO_PORTD_DR8R_R |= (1<<1); break;     //Enable 8mA drive strength on PD1
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTD_DEN_R   |= (1<<1);   //Enable digital output on PD1
            GPIO_PORTD_AFSEL_R |= (1<<1);   // Set PD1 as alternate function
            GPIO_PORTD_PCTL_R &= ~GPIO_PCTL_PD1_M;  // Clear PD1 PCTL bits
            GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD1_M1PWM1;  // Configure PD1 for M1PWM1
            break;
        /*case PWM_PIN_M1_PA6:
            break;
        case PWM_PIN_M1_PE4:
            break;
        case PWM_PIN_M1_PA7:
            break;
        case PWM_PIN_M1_PE5:
            break;
        case PWM_PIN_M1_PF0:
            break;
        case PWM_PIN_M1_PF1:
            break;*/
        case PWM_PIN_M1_PF2:
            SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;    // Enable clock for GPIO Port F
            while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0);  // Wait for GPIO Port F to be ready
            switch(handle->config.drive_strength){
                case DRIVE_SELECT_2MA:
                    GPIO_PORTF_DR2R_R |= (1<<2); break;     //Enable 2mA drive strength on PF2
                case DRIVE_SELECT_4MA:
                    GPIO_PORTF_DR4R_R |= (1<<2); break;     //Enable 4mA drive strength on PF2
                case DRIVE_SELECT_8MA:
                    GPIO_PORTF_DR8R_R |= (1<<2); break;     //Enable 8mA drive strength on PF2
                default:
                    return PWM_STATUS_CONFIG_ERROR;         //Invalid drive strength
            }
            GPIO_PORTF_DEN_R   |= (1<<2);   //Enable digital output on PF2
            GPIO_PORTF_AFSEL_R |= (1<<2);   // Set PF2 as alternate function
            GPIO_PORTF_PCTL_R &= ~GPIO_PCTL_PF2_M;  // Clear PF2 PCTL bits
            GPIO_PORTF_PCTL_R |= GPIO_PCTL_PF2_M1PWM6;  // Configure PF2 for M1PWM6
            break;
        /*case PWM_PIN_M1_PF3:
            break;*/
        default:
            return PWM_STATUS_PIN_ERROR;
    }
    return PWM_STATUS_SUCCESS;
}

/*
 * @brief Configure PWM Clock divisor
 *
 */
static PWM_Status_t PWM_ConfigureDivisor(PWM_Divisor_t divisor){
    switch(divisor){
    case PWM_DIVISOR_NONE:
        SYSCTL_RCC_R &= ~SYSCTL_RCC_USEPWMDIV; // Do not use PWM divider
        break;
    case PWM_DIVISOR_2:
        SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // Use PWM divider
        SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_PWMDIV_M) | SYSCTL_RCC_PWMDIV_2; // Clear PWM divider bits || Set PWM divider to 2
        break;
    case PWM_DIVISOR_4:
        SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // Use PWM divider
        SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_PWMDIV_M) | SYSCTL_RCC_PWMDIV_4; // Clear PWM divider bits || Set PWM divider to 4
        break;
    case PWM_DIVISOR_8:
        SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // Use PWM divider
        SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_PWMDIV_M) | SYSCTL_RCC_PWMDIV_8; // Clear PWM divider bits || Set PWM divider to 8
        break;
    case PWM_DIVISOR_16:
        SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // Use PWM divider
        SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_PWMDIV_M) | SYSCTL_RCC_PWMDIV_16; // Clear PWM divider bits || Set PWM divider to 16
        break;
    case PWM_DIVISOR_32:
        SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // Use PWM divider
        SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_PWMDIV_M) | SYSCTL_RCC_PWMDIV_32; // Clear PWM divider bits || Set PWM divider to 32
        break;
    case PWM_DIVISOR_64:
        SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // Use PWM divider
        SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_PWMDIV_M) | SYSCTL_RCC_PWMDIV_64; // Clear PWM divider bits || Set PWM divider to 64
        break;
    default:
        return PWM_STATUS_INVALID_PARAM;
    }
    return PWM_STATUS_SUCCESS;
}

/*
 * @brief Calculate PWM load value based on frequency and divisor
 */
static uint32_t PWM_CalculateLoadValue(uint32_t frequency_hz, PWM_Divisor_t divisor)
{
    uint32_t system_clock = PWM_GetSystemClock();
    uint32_t divisor_value = 1;

    switch(divisor){
    case PWM_DIVISOR_2: divisor_value = 2; break;
    case PWM_DIVISOR_4: divisor_value = 4; break;
    case PWM_DIVISOR_8: divisor_value = 8; break;
    case PWM_DIVISOR_16: divisor_value = 16; break;
    case PWM_DIVISOR_32: divisor_value = 32; break;
    case PWM_DIVISOR_64: divisor_value = 64; break;
    default: divisor_value = 1; break;
    }

    uint32_t effective_clock = system_clock / divisor_value;
    return (effective_clock / frequency_hz) - 1;
}

/*
 * @brief Get current system clock frequency
 */
static uint32_t PWM_GetSystemClock(void)
{
    uint32_t sysdiv = (SYSCTL_RCC2_R & 0x1FC00000) >> 22;   //Get the current value of the sysdiv
    float freq_hz = (400.0f / ((float)sysdiv + 1.0f)) * 1000000.0f;//calculate the current frequency of the clock

    if(freq_hz == 0.0f){
        return DEFAULT_SYSTEM_CLOCK_HZ;
    }

    return (uint32_t)freq_hz;
}



/*
 * @brief Validate PWM handle
 *
 */
static bool PWM_IsValidHandle(const PWM_Handle_t* handle)
{
    return (handle != NULL && handle->is_initialized);
}

/*
 * @brief Validate PWM handle
 *
 */
static bool PWM_IsValidDutyCycle(uint8_t duty_cycle)
{
    return (duty_cycle <= PWM_MAX_DUTY_CYCLE);
}

/*
 * @brief Validate PWM handle
 *
 */
static bool PWM_IsValidFrequency(uint32_t frequency_hz)
{
    return (frequency_hz > 0 && frequency_hz < 1000000);//1MHz as Max
}

/*
 * @brief Validate PWM Drive Strength
 *
 */
static bool PWM_IsValidDriveStrength(const PWM_Config_t* config)
{
    return (config->drive_strength == DRIVE_SELECT_2MA ||
            config->drive_strength == DRIVE_SELECT_4MA ||
            config->drive_strength == DRIVE_SELECT_8MA);
}

/*
 * @brief Validate PWM Count Mode
 *
 * This function checks if the count mode specified in the PWM configuration is valid.
 *
 * @param config Pointer to PWM configuration structure
 * @return true if count mode is valid, false otherwise
 *
 */
static bool PWM_IsValidCountMode(const PWM_Config_t* config)
{
    return (config->count_mode < MODE_COUNT);
}

/* Public API Instance ----------------------------------------------------------------------------*/
const PWM_Interface_t PWM_API = {
    .Init = PWM_Init,
    .SetDutyCycle = PWM_SetDutyCycle,
    .Enable = PWM_Enable,
    .Disable = PWM_Disable,
    .DeInit = PWM_DeInit
};



