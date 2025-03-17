/* common/ubit-v1/device.ld */
/* Copyright (c) 2018-20 J. M. Spivey */

MEMORY {
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 16K
}

OUTPUT_FORMAT ("elf32-littlearm")

/* Setting the entry address helps GDB to find the stack bottom */
ENTRY(__reset)

SECTIONS {
    .text : {
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata*)
        KEEP(*(.init_array))

        . = ALIGN(4);
        __etext = .;
    } > FLASH

    .data : AT (__etext) {
        __data_start = .;

        *(.data*)

        . = ALIGN(4);
        __data_end = .;
    } > RAM

    .bss : {
        __bss_start = .;

        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        __bss_end = .;
    } > RAM

    __recover_load = __etext + __data_end - __data_start;
    .recover : AT (__recover_load) {
        __recover_start = .;
        KEEP(*(.recover)) ;
        . = ALIGN(4);
        __recover_end = .;
    } > RAM

    __startup_load = __recover_load + __recover_end - __recover_start;
    .startup : AT (__startup_load) {
        __startup_start = .;
        KEEP(*(.startup)) ;
        . = ALIGN(4);
        __startup_end = .;
    } > RAM

    __procs_load = __startup_load + __startup_end - __startup_start;
    .procs : AT (__procs_load) {
        __procs_start = .;
        KEEP(*(.procs)) ;
        . = ALIGN(4);
        __procs_end = .;
    } > RAM

    __apps_load = __procs_load + __procs_end - __procs_start;
    .apps : AT (__apps_load) {
        __apps_start = .;
        KEEP(*(.apps)) ;
        . = ALIGN(4);
        __apps_end = .;
    } > RAM

    __end = .;

    /* Set stack top to end of RAM, and move stack limit down by
       size of stack */
    __stack = ORIGIN(RAM) + LENGTH(RAM);
    __stack_limit = __stack - 1024;

    /* Check if data + stack exceeds RAM limit */
    ASSERT((__stack_limit >= __end), "region RAM overlaps with stack")
}

/* Interrupt handlers are aliases for default_handler if not
   otherwise defined */
PROVIDE(nmi_handler = default_handler);
PROVIDE(hardfault_handler = default_handler);
PROVIDE(svc_handler = default_handler);
PROVIDE(pendsv_handler = default_handler);
PROVIDE(systick_handler = default_handler);
PROVIDE(uart_handler = default_handler);
PROVIDE(timer0_handler = default_handler);
PROVIDE(timer1_handler = default_handler);
PROVIDE(timer2_handler = default_handler);
PROVIDE(power_clock_handler = default_handler);
PROVIDE(radio_handler = default_handler);
PROVIDE(i2c0_spi0_handler = default_handler);
PROVIDE(i2c1_spi1_handler = default_handler);
PROVIDE(gpiote_handler = default_handler);
PROVIDE(adc_handler = default_handler);
PROVIDE(rtc0_handler = default_handler);
PROVIDE(temp_handler = default_handler);
PROVIDE(rng_handler = default_handler);
PROVIDE(ecb_handler = default_handler);
PROVIDE(ccm_aar_handler = default_handler);
PROVIDE(wdt_handler = default_handler);
PROVIDE(rtc1_handler = default_handler);
PROVIDE(qdec_handler = default_handler);
PROVIDE(lpcomp_handler = default_handler);
PROVIDE(swi0_handler = default_handler);
PROVIDE(swi1_handler = default_handler);
PROVIDE(swi2_handler = default_handler);
PROVIDE(swi3_handler = default_handler);
PROVIDE(swi4_handler = default_handler);
PROVIDE(swi5_handler = default_handler);

/* Addresses for hardware devices */
SCB     = 0xe000ed00;
NVIC    = 0xe000e000;
CLOCK   = 0x40000000;
MPU     = 0x40000000;
FICR    = 0x10000000;
POWER   = 0x40000000;
WDT     = 0x40010000;
NVMC    = 0x4001e000;
GPIO    = 0x50000500;
GPIOTE  = 0x40006000;
PPI     = 0x4001f000;
RADIO   = 0x40001000;
TIMER0  = 0x40008000;
TIMER1  = 0x40009000;
TIMER2  = 0x4000a000;
RNG     = 0x4000d000;
TEMP    = 0x4000c000;
I2C0    = 0x40003000;
I2C1    = 0x40004000;
SPI0    = 0x40003000;
SPI1    = 0x40004000;
UART    = 0x40002000;
ADC     = 0x40007000;
