#include <stdint.h>

extern uint8_t __stack[];
void __reset(void);

/*  INTERRUPT VECTORS */

/* We use the linker script to define each handler name as an alias
for default_handler if it is not defined elsewhere.  Applications can
subsitute their own definitions for individual handler names like
uart_handler(). */

/* The linker script makes all these handlers into weak aliases for */
/* default_handler. */

void nmi_handler(void);
void hardfault_handler(void);
void svc_handler(void);
void pendsv_handler(void);
void systick_handler(void);
void uart_handler(void);
void timer0_handler(void);
void timer1_handler(void);
void timer2_handler(void);
void power_clock_handler(void);
void radio_handler(void);
void i2c0_spi0_handler(void);
void i2c1_spi1_handler(void);
void gpiote_handler(void);
void adc_handler(void);
void rtc0_handler(void);
void temp_handler(void);
void rng_handler(void);
void ecb_handler(void);
void ccm_aar_handler(void);
void wdt_handler(void);
void rtc1_handler(void);
void qdec_handler(void);
void lpcomp_handler(void);
void swi0_handler(void);
void swi1_handler(void);
void swi2_handler(void);
void swi3_handler(void);
void swi4_handler(void);
void swi5_handler(void);

/* This vector table is placed at address 0 in the flash by directives
in the linker script. */

__attribute((section(".vectors"))) void *__vectors[] = {
    __stack,                                    /* -16 */
    __reset, nmi_handler, hardfault_handler, 0, /* -12 */
    0, 0, 0, 0,                                 /*  -8 */
    0, 0, svc_handler, 0,                       /* -4 */
    0, pendsv_handler, systick_handler,

    /* external interrupts */
    power_clock_handler,                                               /*  0 */
    radio_handler, uart_handler, i2c0_spi0_handler, i2c1_spi1_handler, /*  4 */
    0, gpiote_handler, adc_handler, timer0_handler,                    /*  8 */
    timer1_handler, timer2_handler, rtc0_handler, temp_handler,        /* 12 */
    rng_handler, ecb_handler, ccm_aar_handler, wdt_handler,            /* 16 */
    rtc1_handler, qdec_handler, lpcomp_handler, swi0_handler,          /* 20 */
    swi1_handler, swi2_handler, swi3_handler, swi4_handler,            /* 24 */
    swi5_handler, 0, 0, 0,                                             /* 28 */
    0, 0, 0};
