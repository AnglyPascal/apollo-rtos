#include <stdint.h>

extern uint8_t __stack[];
void __reset(void);

//  INTERRUPT VECTORS

/* We use the linker script to define each handler name as an alias for
 * default_handler if it is not defined elsewhere. Applications can subsitute
 * their own definitions for individual handler names like uart_handler(). */

/* The linker script makes all these handlers into weak aliases for
 * default_handler. */

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

/* This vector table is placed at address 0 in the flash by directives in the
 * linker script. */

__attribute((section(".vectors"))) void *__vectors[] = {
    __stack, // -16: Initial Main Stack Pointer (MSP) value
    __reset, // Reset Handler: Entry point after power-up or reset

    nmi_handler,       // Non-Maskable Interrupt (NMI) Handler
    hardfault_handler, // Hard Fault Handler

    // -12
    0, // Reserved (must be 0)
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved

    svc_handler, // Supervisor Call (SVC) Handler

    // -4
    0, // Reserved (Debug Monitor, not present in Cortex-M0)
    0, // Reserved

    pendsv_handler,  // PendSV Handler (used for context switching in RTOS)
    systick_handler, // SysTick Timer Handler (used for periodic interrupts)

    // External interrupt handlers (specific to the Nordic nRF51822 chip)
    power_clock_handler, // Power and clock control events
    radio_handler,       // Radio interrupt (Bluetooth Low Energy)
    uart_handler,        // UART (serial communication) interrupt

    i2c0_spi0_handler, // I2C0 / SPI0 interrupt
    i2c1_spi1_handler, // I2C1 / SPI1 interrupt

    0, // Reserved

    gpiote_handler, // GPIO Task and Event Handler
    adc_handler,    // Analog-to-Digital Converter (ADC) interrupt

    timer0_handler, // Timer 0 interrupt
    timer1_handler, // Timer 1 interrupt
    timer2_handler, // Timer 2 interrupt

    rtc0_handler,    // Real-Time Counter 0 interrupt
    temp_handler,    // Temperature sensor interrupt
    rng_handler,     // Random Number Generator (RNG) interrupt
    ecb_handler,     // AES Electronic Codebook (ECB) encryption interrupt
    ccm_aar_handler, // AES CCM and Address Resolution interrupt
    wdt_handler,     // Watchdog Timer (WDT) interrupt
    rtc1_handler,    // Real-Time Counter 1 interrupt
    qdec_handler,    // Quadrature Decoder (used for rotary encoders)
    lpcomp_handler,  // Low Power Comparator interrupt

    swi0_handler, // Software interrupt 0
    swi1_handler, // Software interrupt 1
    swi2_handler, // Software interrupt 2
    swi3_handler, // Software interrupt 3
    swi4_handler, // Software interrupt 4
    swi5_handler, // Software interrupt 5

    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved
    0, // Reserved
};
