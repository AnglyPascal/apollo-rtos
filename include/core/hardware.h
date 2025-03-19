/* common/hardware.h */
/* Copyright (c) 2018-20 J. M. Spivey */

#pragma once

#define UBIT 1
#define UBIT_V1 1

#include <cstdint>

/* Hardware register definitions for nRF51822 */

#define BIT(i) (1 << (i))
#define SET_BIT(r, n) r |= BIT(n)
#define GET_BIT(r, n) (((r) >> (n)) & 0x1)
#define CLR_BIT(r, n) r &= ~BIT(n)
#define GET_BYTE(r, n) (((r) >> (8 * (n))) & 0xff)
#define SET_BYTE(r, n, v) r = (r & ~(0xff << 8 * n)) | ((v & 0xff) << 8 * n)

/* The macros SET_FIELD, etc., are defined in an indirect way that
permits (because of the timing of CPP macro expansion) the 'field'
argument to be a macro that expands the a 'position, width' pair. */

#define SET_FIELD(r, field, val) _SET_FIELD(r, field, val)
#define _SET_FIELD(r, pos, wd, val)                                            \
  r = (r & ~_MASK(pos, wd)) | _FIELD(pos, wd, val)

#define GET_FIELD(r, field) _GET_FIELD(r, field)
#define _GET_FIELD(r, pos, wd) ((r >> pos) & _MASK0(wd))

#define FIELD(field, val) _FIELD(field, val)
#define _FIELD(pos, wd, val) (((val) & _MASK0(wd)) << pos)

#define MASK(field) _MASK(field)
#define _MASK(pos, wd) (_MASK0(wd) << pos)

#define _MASK0(wd) (~((-2u) << (wd - 1)))

#define __BIT(pos) pos
#define __FIELD(pos, wd) pos, wd

/* Device pins */
#define PAD19 0
#define PAD2 1
#define PAD1 2
#define PAD0 3
/* LED columns are GPIO 4-12 */
#define PAD3 4
#define PAD4 5
#define PAD10 6
#define PAD9 10
#define PAD7 11
#define PAD6 12
#define ROW1 13
#define ROW2 14
#define ROW3 15
#define PAD16 16
#define PAD5 17
#define PAD8 18
#define PAD12 20
#define PAD15 21
#define PAD14 22
#define PAD13 23
#define PAD11 26
#define PAD20 30

#define USB_TX 24
#define USB_RX 25

#define BUTTON_A PAD5
#define BUTTON_B PAD11

#define I2C0_SCL PAD19
#define I2C0_SDA PAD20

#define SPI_SCK PAD13
#define SPI_MISO PAD14
#define SPI_MOSI PAD15

/* One shared I2C bus (I2C_SCL, I2C_SDA); use SPI1 for SPI */
#define N_I2C 1
#define I2C_INTERNAL 0
#define I2C_EXTERNAL 0
#define SPI_CHAN 1

/* Interrupts 3 and 4 are shared between I2C and SPI: we can
define a handler with either name */
#define i2c0_handler i2c0_spi0_handler
#define spi0_handler i2c0_spi0_handler
#define i2c1_handler i2c1_spi1_handler
#define spi1_handler i2c1_spi1_handler

/* Device registers */
#define _PADDING(n) uint8_t _PAD(__LINE__)[n]
#define _PAD(lnum) _JOIN(_pad, lnum)
#define _JOIN(x, y) x##y

/* System contol block */
struct scb_t {
  uint32_t CPUID; // 0x00
  uint32_t ICSR;  // 0x04
#define SCB_ICSR_PENDSVSET __BIT(28)
#define SCB_ICSR_VECTACTIVE __FIELD(0u, 8u)
  _PADDING(8);
  uint32_t SCR; // 0x10
#define SCB_SCR_SLEEPONEXIT __BIT(1)
#define SCB_SCR_SLEEPDEEP __BIT(2)
#define SCB_SCR_SEVONPEND __BIT(4)
  _PADDING(4);
  uint32_t SHPR[3]; // 0x18
};

extern volatile scb_t SCB;

/* Nested vectored interupt controller */
struct nvic_t {
  _PADDING(256);
  uint32_t ISER[8]; // 0x100
  _PADDING(96);
  uint32_t ICER[8]; // 0x180
  _PADDING(96);
  uint32_t ISPR[8]; // 0x200
  _PADDING(96);
  uint32_t ICPR[8]; // 0x280
  _PADDING(352);
  uint32_t IPR[60]; // 0x400
};

extern volatile nvic_t NVIC;

/* Clock control */
struct clock_t {
  uint32_t HFCLKSTART; // 0x000
  _PADDING(4);
  uint32_t LFCLKSTART; // 0x008
  _PADDING(244);
  uint32_t HFCLKSTARTED; // 0x100
  uint32_t LFCLKSTARTED; // 0x104
  _PADDING(1040);
  uint32_t LFCLKSRC; // 0x518
#define CLOCK_LFCLKSRC_RC 0
  _PADDING(52);
  uint32_t XTALFREQ; // 0x550
#define CLOCK_XTALFREQ_16MHz 0xFF
};

extern volatile clock_t CLOCK;

/* Memory protection unit */
struct mpu_t {
  _PADDING(1536);
  uint32_t PROTENSET0;     // 0x600
  uint32_t PROTENSET1;     // 0x604
  uint32_t DISABLEINDEBUG; // 0x608
};

extern volatile mpu_t MPU;

/* Factory information */
struct ficr_t {
  _PADDING(16);
  uint32_t CODEPAGESIZE; // 0x010
  _PADDING(76);
  uint32_t DEVICEID[2]; // 0x060
  _PADDING(60);
  uint32_t DEVICEADDR[2]; // 0x0a4
  uint32_t OVERRIDEEN;    // 0x0a0
#define FICR_OVERRIDEEN_NRF __BIT(0)
  _PADDING(12);
  uint32_t NRF_1MBIT[5]; // 0x0b0
};

extern volatile ficr_t FICR;

/* Power management */
struct power_t {
  /* Tasks */
  _PADDING(120);
  uint32_t CONSTLAT; // 0x078
  uint32_t LOWPWR;   // 0x07c
  /* Events */
  _PADDING(136);
  uint32_t POFWARN; // 0x108
  /* Registers */
  _PADDING(504);
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(244);
  uint32_t RESETREAS; // 0x400
#define POWER_RESETREAS_RESETPIN __BIT(0)
#define POWER_RESETREAS_DOG __BIT(1)
#define POWER_RESETREAS_SREQ __BIT(2)
#define POWER_RESETREAS_LOCKUP __BIT(3)
#define POWER_RESETREAS_OFF __BIT(16)
#define POWER_RESETREAS_LPCOMP __BIT(17)
#define POWER_RESETREAS_DIF _BIT(18)
#define POWER_RESETREAS_ALL 0x0007000f
  _PADDING(36);
  uint32_t RAMSTATUS; // 0x428
  _PADDING(212);
  uint32_t SYSTEMOFF; // 0x500
  _PADDING(12);
  uint32_t POFCON; // 0x510
#define POWER_POFCON_POF __BIT(1)
#define POWER_POFCON_TRESHOLD _FIELD(1, 2)
#define POWER_THRESHOLD_V21 0
#define POWER_THRESHOLD_V23 1
#define POWER_THRESHOLD_V25 2
#define POWER_THRESHOLD_V27 3
  _PADDING(8);
  uint32_t GPREGRET; // 0x51c
  _PADDING(4);
  uint32_t RAMON; // 0x524
  _PADDING(28);
  uint32_t RESET; // 0x544
  _PADDING(12);
  uint32_t RAMONB; // 0x554
  _PADDING(32);
  uint32_t DCDCEN; // 0x578
};

/* Interrupts */
#define POWER_INT_POFWARN 2

extern volatile power_t POWER;

/* Watchdog timer */
struct wdt_t {
  /* Tasks */
  uint32_t START; // 0x000
  /* Events */
  _PADDING(252);
  uint32_t TIMEOUT; // 0x100
  /* Registers */
  _PADDING(512);
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(244);
  uint32_t RUNSTATUS; // 0x400
  uint32_t REQSTATUS; // 0x404
  _PADDING(252);
  uint32_t CRV; // 0x504
#define WDT_HERTZ 32768
  uint32_t RREN;   // 0x508
  uint32_t CONFIG; // 0x50c
#define WDT_CONFIG_SLEEP __BIT(0)
#define WDT_CONFIG_HALT __BIT(3)
  _PADDING(240);
  uint32_t RR[8]; // 0x600
#define WDT_MAGIC 0x6e524635
};

/* Interrupts */
#define WDT_INT_TIMEOUT 0

extern volatile wdt_t WDT;

/* Non-Volatile Memory Controller */
struct nvmc_t {
  _PADDING(1024);
  uint32_t READY; // 0x400
  _PADDING(256);
  uint32_t CONFIG; // 0x504
#define NVMC_CONFIG_REN 0
#define NVMC_CONFIG_WEN BIT(0)
#define NVMC_CONFIG_EEN BIT(1)
  void *ERASEPAGE; // 0x508
};

extern volatile nvmc_t NVMC;

/* GPIO */
struct gpio_t {
  /* Registers */
  _PADDING(4);
  uint32_t OUT;    // 0x004
  uint32_t OUTSET; // 0x008
  uint32_t OUTCLR; // 0x00c
  uint32_t IN;     // 0x010
  uint32_t DIR;    // 0x014
  uint32_t DIRSET; // 0x018
  uint32_t DIRCLR; // 0x01c
  _PADDING(480);
  uint32_t PINCNF[32]; // 0x200
#define GPIO_PINCNF_DIR __FIELD(0, 1)
#define GPIO_DIR_Input 0
#define GPIO_DIR_Output 1
#define GPIO_PINCNF_INPUT __FIELD(1, 1)
#define GPIO_INPUT_Connect 0
#define GPIO_INPUT_Disconnect 1
#define GPIO_PINCNF_PULL __FIELD(2, 2)
#define GPIO_PULL_Disabled 0
#define GPII_PULL_Pulldown 1
#define GPIO_PULL_Pullup 3
#define GPIO_PINCNF_DRIVE __FIELD(8, 3)
#define GPIO_DRIVE_S0S1 0
#define GPIO_DRIVE_H0S1 1
#define GPIO_DRIVE_S0H1 2
#define GPIO_DRIVE_H0H1 3
#define GPIO_DRIVE_D0S1 4
#define GPIO_DRIVE_D0H1 5
#define GPIO_DRIVE_S0D1 6 /* Open drain */
#define GPIO_DRIVE_H0D1 7
#define GPIO_PINCNF_SENSE __FIELD(16, 2)
#define GPIO_SENSE_Disabled 0
#define GPIO_SENSE_High 2
#define GPIO_SENSE_Low 3
};

extern volatile gpio_t GPIO;

/* GPIOTE */
struct gpiote_t {
  /* Tasks */
  uint32_t OUT[4]; // 0x000
  _PADDING(32);
  uint32_t SET[4]; // 0x030
  _PADDING(32);
  uint32_t CLR[4]; // 0x060
  /* Events */
  _PADDING(144);
  uint32_t IN[4]; // 0x100
  _PADDING(108);
  uint32_t PORT; // 0x17c
  /* Registers */
  _PADDING(388);
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(516);
  uint32_t CONFIG[4]; // 0x510
#define GPIOTE_CONFIG_MODE __FIELD(0, 2)
#define GPIOTE_MODE_Event 1
#define GPIOTE_MODE_Task 3
#define GPIOTE_CONFIG_PSEL __FIELD(8, 5)
#define GPIOTE_CONFIG_POLARITY __FIELD(16, 2)
#define GPIOTE_POLARITY_LoToHi 1
#define GPIOTE_POLARITY_HiToLo 2
#define GPIOTE_POLARITY_Toggle 3
#define GPIOTE_CONFIG_OUTINIT __FIELD(20, 1)
};

/* Interrupts */
#define GPIOTE_INT_IN0 0
#define GPIOTE_INT_IN1 1
#define GPIOTE_INT_IN2 2
#define GPIOTE_INT_IN3 3
#define GPIOTE_INT_PORT 31

extern volatile gpiote_t GPIOTE;

/* PPI */
typedef struct {
  uint32_t EN, DIS;
} ppi_chg;

typedef struct {
  uint32_t volatile *EEP, *TEP;
} ppi_chan;

struct ppi_t {
  /* Tasks */
  ppi_chg CHG[6]; // 0x000
  /* Registers */
  _PADDING(1232);
  uint32_t CHEN;    // 0x500
  uint32_t CHENSET; // 0x504
  uint32_t CHENCLR; // 0x508
  _PADDING(4);
  ppi_chan CH[20]; // 0x510
  _PADDING(592);
  uint32_t CHGRP[6]; // 0x800
};

extern volatile ppi_t PPI;

/* Radio */
struct radio_t {
  /* Tasks */
  uint32_t TXEN;      // 0x000
  uint32_t RXEN;      // 0x004
  uint32_t START;     // 0x008
  uint32_t STOP;      // 0x00c
  uint32_t DISABLE;   // 0x010
  uint32_t RSSISTART; // 0x014
  uint32_t RSSISTOP;  // 0x018
  uint32_t BCSTART;   // 0x01c
  uint32_t BCSTOP;    // 0x020
  /* Events */
  _PADDING(220);
  uint32_t READY;    // 0x100
  uint32_t ADDRESS;  // 0x104
  uint32_t PAYLOAD;  // 0x108
  uint32_t END;      // 0x10c
  uint32_t DISABLED; // 0x110
  uint32_t DEVMATCH; // 0x114
  uint32_t DEVMISS;  // 0x118
  uint32_t RSSIEND;  // 0x11c
  _PADDING(8);
  uint32_t BCMATCH; // 0x128
  /* Registers */
  _PADDING(212);
  uint32_t SHORTS; // 0x200
  _PADDING(256);
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(244);
  uint32_t CRCSTATUS; // 0x400
  _PADDING(4);
  uint32_t RXMATCH; // 0x408
  uint32_t RXCRC;   // 0x40c
  uint32_t DAI;     // 0x410
  _PADDING(240);
  void *PACKETPTR;    // 0x504
  uint32_t FREQUENCY; // 0x508
  uint32_t TXPOWER;   // 0x50c
  uint32_t MODE;      // 0x510
#define RADIO_MODE_NRF_1Mbit 0
  uint32_t PCNF0; // 0x514
#define RADIO_PCNF0_LFLEN __FIELD(0, 4)
#define RADIO_PCNF0_S0LEN __FIELD(8, 1)
#define RADIO_PCNF0_S1LEN __FIELD(16, 4)
  uint32_t PCNF1; // 0x518
#define RADIO_PCNF1_MAXLEN __FIELD(0, 8)
#define RADIO_PCNF1_STATLEN __FIELD(8, 8)
#define RADIO_PCNF1_BALEN __FIELD(16, 3)
#define RADIO_PCNF1_ENDIAN __FIELD(24, 1)
#define RADIO_ENDIAN_Little 0
#define RADIO_ENDIAN_Big 1
#define RADIO_PCNF1_WHITEEN __BIT(25)
  uint32_t BASE0;       // 0x51c
  uint32_t BASE1;       // 0x520
  uint32_t PREFIX0;     // 0x524
  uint32_t PREFIX1;     // 0x528
  uint32_t TXADDRESS;   // 0x52c
  uint32_t RXADDRESSES; // 0x530
  uint32_t CRCCNF;      // 0x534
  uint32_t CRCPOLY;     // 0x538
  uint32_t CRCINIT;     // 0x53c
  uint32_t TEST;        // 0x540
  uint32_t TIFS;        // 0x544
  uint32_t RSSISAMPLE;  // 0x548
  _PADDING(4);
  uint32_t STATE;       // 0x550
  uint32_t DATAWHITEIV; // 0x554
  _PADDING(8);
  uint32_t BCC; // 0x560
  _PADDING(156);
  uint32_t DAB[8]; // 0x600
  uint32_t DAP[8]; // 0x620
  uint32_t DACNF;  // 0x640
  _PADDING(224);
  uint32_t OVERRIDE[5]; // 0x724
  _PADDING(2244);
  uint32_t POWER; // 0xffc
};

/* Interrupts */
#define RADIO_INT_READY 0
#define RADIO_INT_END 3
#define RADIO_INT_DISABLED 4

extern volatile radio_t RADIO;

/* TIMERS: Timer 0 is 8/16/24/32 bit, Timers 1 and 2 are 8/16 bit. */
struct timer_t {
  /* Tasks */
  uint32_t START;    // 0x000
  uint32_t STOP;     // 0x004
  uint32_t COUNT;    // 0x008
  uint32_t CLEAR;    // 0x00c
  uint32_t SHUTDOWN; // 0x010
  _PADDING(44);
  uint32_t CAPTURE[4]; // 0x040
  /* Events */
  _PADDING(240);
  uint32_t COMPARE[4]; // 0x140
  /* Registers */
  _PADDING(176);
  uint32_t SHORTS; // 0x200
  _PADDING(256);
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(504);
  uint32_t MODE; // 0x504
#define TIMER_MODE_Timer 0
#define TIMER_MODE_Counter 1
  uint32_t BITMODE; // 0x508
#define TIMER_BITMODE_16Bit 0
#define TIMER_BITMODE_8Bit 1
#define TIMER_BITMODE_24Bit 2
#define TIMER_BITMODE_32Bit 3
  _PADDING(4);
  uint32_t PRESCALER; // 0x510
  _PADDING(44);
  uint32_t CC[4]; // 0x540
};

/* Interrupts */
#define TIMER_INT_COMPARE0 16
#define TIMER_INT_COMPARE1 17
#define TIMER_INT_COMPARE2 18
#define TIMER_INT_COMPARE3 19
/* Shortcuts */
#define TIMER_COMPARE0_CLEAR 0
#define TIMER_COMPARE1_CLEAR 1
#define TIMER_COMPARE2_CLEAR 2
#define TIMER_COMPARE3_CLEAR 3
#define TIMER_COMPARE0_STOP 8
#define TIMER_COMPARE1_STOP 9
#define TIMER_COMPARE2_STOP 10
#define TIMER_COMPARE3_STOP 11

extern volatile timer_t TIMER0, TIMER1, TIMER2;

/* Random Number Generator */
struct rng_t {
  /* Tasks */
  uint32_t START; // 0x000
  uint32_t STOP;  // 0x004
  /* Events */
  _PADDING(248);
  uint32_t VALRDY; // 0x100
  /* Registers */
  _PADDING(252);
  uint32_t SHORTS; // 0x200
  _PADDING(252);
  uint32_t INTEN;    // 0x300
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(504);
  uint32_t CONFIG; // 0x504
#define RNG_CONFIG_DERCEN __BIT(0)
  uint32_t VALUE; // 0x508
};

/* Interrupts */
#define RNG_INT_VALRDY 0

extern volatile rng_t RNG;

/* Temperature sensor */
struct temp_t {
  /* Tasks */
  uint32_t START; // 0x000
  uint32_t STOP;  // 0x004
  /* Events */
  _PADDING(248);
  uint32_t DATARDY; // 0x100
  /* Registers */
  _PADDING(508);
  uint32_t INTEN;    // 0x300
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(508);
  uint32_t VALUE; // 0x508
};

/* Interrupts */
#define TEMP_INT_DATARDY 0

extern volatile temp_t TEMP;

/* I2C */
struct i2c_t {
  /* Tasks */
  uint32_t STARTRX; // 0x000
  _PADDING(4);
  uint32_t STARTTX; // 0x008
  _PADDING(8);
  uint32_t STOP; // 0x014
  _PADDING(4);
  uint32_t SUSPEND; // 0x01c
  uint32_t RESUME;  // 0x020
  /* Events */
  _PADDING(224);
  uint32_t STOPPED;  // 0x104
  uint32_t RXDREADY; // 0x108
  _PADDING(16);
  uint32_t TXDSENT; // 0x11c
  _PADDING(4);
  uint32_t ERROR; // 0x124
  _PADDING(16);
  uint32_t BB; // 0x138
  _PADDING(12);
  uint32_t SUSPENDED; // 0x148
  /* Registers */
  _PADDING(180);
  uint32_t SHORTS; // 0x200
  _PADDING(252);
  uint32_t INTEN;    // 0x300
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(440);
  uint32_t ERRORSRC; // 0x4c4
#define I2C_ERRORSRC_OVERRUN __BIT(0)
#define I2C_ERRORSRC_ANACK __BIT(1)
#define I2C_ERRORSRC_DNACK __BIT(2)
#define I2C_ERRORSRC_All 0x7
  _PADDING(56);
  uint32_t ENABLE; // 0x500
#define I2C_ENABLE_Disabled 0
#define I2C_ENABLE_Enabled 5
  _PADDING(4);
  uint32_t PSELSCL; // 0x508
  uint32_t PSELSDA; // 0x50c
  _PADDING(8);
  uint32_t RXD; // 0x518
  uint32_t TXD; // 0x51c
  _PADDING(4);
  uint32_t FREQUENCY; // 0x524
#define I2C_FREQUENCY_100kHz 0x01980000
  _PADDING(96);
  uint32_t ADDRESS; // 0x588
  _PADDING(2672);
  uint32_t POWER; // 0xffc
};

/* Interrupts */
#define I2C_INT_STOPPED 1
#define I2C_INT_RXDREADY 2
#define I2C_INT_TXDSENT 7
#define I2C_INT_ERROR 9
#define I2C_INT_BB 14
/* Shortcuts */
#define I2C_BB_SUSPEND 0
#define I2C_BB_STOP 1

extern volatile i2c_t I2C0, I2C1;
extern volatile i2c_t *const I2C[];

/* SPI */
struct spi_t {
  _PADDING(264);
  uint32_t READY; // 0x108
  _PADDING(500);
  uint32_t INTEN;    // 0x300
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(500);
  uint32_t ENABLE; // 0x500
#define SPI_ENABLE_Enabled 1
#define SPI_ENABLE_Disabled 0
  _PADDING(4);
  uint32_t PSELSCK;  // 0x508
  uint32_t PSELMOSI; // 0x50c
  uint32_t PSELMISO; // 0x510
  _PADDING(4);
  uint32_t RXD; // 0x518
  uint32_t TXD; // 0x51c
  _PADDING(4);
  uint32_t FREQUENCY; // 0x524
#define SPI_FREQUENCY_125kHz 0x02000000
#define SPI_FREQUENCY_250kHz 0x04000000
#define SPI_FREQUENCY_500kHz 0x08000000
#define SPI_FREQUENCY_1MHz 0x10000000
#define SPI_FREQUENCY_2MHz 0x20000000
#define SPI_FREQUENCY_4MHz 0x40000000
#define SPI_FREQUENCY_8MHz 0x80000000
  _PADDING(44);
  uint32_t CONFIG; // 0x554
#define SPI_CONFIG_ORDER __FIELD(0, 1)
#define SPI_ORDER_MsbFirst 0
#define SPI_ORDER_LsbFirst 1
#define SPI_CONFIG_CPHASE __FIELD(1, 1)
#define SPI_CPHASE_Leading 0
#define SPI_CPHASE_Trailing 1
#define SPI_CONFIG_CPOLARITY __FIELD(2, 1)
#define SPI_CPOLARITY_ActiveHigh 0
#define SPI_CPOLARITY_ActiveLow 1
};

#define SPI_INT_READY 2

extern volatile spi_t SPI0, SPI1;
extern volatile spi_t *const SPI[];

/* UART */
struct uart_t {
  /* Tasks */
  uint32_t STARTRX; // 0x000
  _PADDING(4);
  uint32_t STARTTX; // 0x008
  /* Events */
  _PADDING(252);
  uint32_t RXDRDY; // 0x108
  _PADDING(16);
  uint32_t TXDRDY; // 0x11c
  /* Registers */
  _PADDING(484);
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(500);
  uint32_t ENABLE; // 0x500
#define UART_ENABLE_Disabled 0
#define UART_ENABLE_Enabled 4
  _PADDING(8);
  uint32_t PSELTXD; // 0x50c
  _PADDING(4);
  uint32_t PSELRXD; // 0x514
  uint32_t RXD;     // 0x518
  uint32_t TXD;     // 0x51c
  _PADDING(4);
  uint32_t BAUDRATE; // 0x524
#define UART_BAUDRATE_1200 0x0004f000
#define UART_BAUDRATE_2400 0x0009d000
#define UART_BAUDRATE_4800 0x0013b000
#define UART_BAUDRATE_9600 0x00275000
#define UART_BAUDRATE_14400 0x003af000
#define UART_BAUDRATE_19200 0x004ea000
#define UART_BAUDRATE_28800 0x0075c000
#define UART_BAUDRATE_31250 0x00800000
#define UART_BAUDRATE_38400 0x009d0000
#define UART_BAUDRATE_56000 0x00e50000
#define UART_BAUDRATE_57600 0x00eb0000
#define UART_BAUDRATE_76800 0x013a9000
#define UART_BAUDRATE_115200 0x01d60000
#define UART_BAUDRATE_230400 0x03b00000
#define UART_BAUDRATE_250000 0x04000000
#define UART_BAUDRATE_460800 0x07400000
#define UART_BAUDRATE_921600 0x0f000000
#define UART_BAUDRATE_1M 0x10000000
  _PADDING(68);
  uint32_t CONFIG; // 0x56c
#define UART_CONFIG_HWFC __BIT(0)
#define UART_CONFIG_PARITY __FIELD(1, 3)
#define UART_PARITY_None 0
#define UART_PARITY_Even 7
};

/* Interrupts */
#define UART_INT_RXDRDY 2
#define UART_INT_TXDRDY 7

extern volatile uart_t UART;

/* ADC */
struct adc_t {
  /* Tasks */
  uint32_t START; // 0x000
  uint32_t STOP;  // 0x004
  /* Events */
  _PADDING(248);
  uint32_t END; // 0x100
  /* Registers */
  _PADDING(508);
  uint32_t INTEN;    // 0x300
  uint32_t INTENSET; // 0x304
  uint32_t INTENCLR; // 0x308
  _PADDING(244);
  uint32_t BUSY; // 0x400
  _PADDING(252);
  uint32_t ENABLE; // 0x500
  uint32_t CONFIG; // 0x504
#define ADC_CONFIG_RES __FIELD(0, 2)
#define ADC_RES_8Bit 0
#define ADC_RES_9bit 1
#define ADC_RES_10bit 2
#define ADC_CONFIG_INPSEL __FIELD(2, 3)
#define ADC_INPSEL_AIn_1_1 0
#define ADC_INPSEL_AIn_2_3 1
#define ADC_INPSEL_AIn_1_3 2
#define ADC_INPSEL_Vdd_2_3 5
#define ADC_INPSEL_Vdd_1_3 6
#define ADC_CONFIG_REFSEL __FIELD(5, 2)
#define ADC_REFSEL_BGap 0
#define ADC_REFSEL_Ext 1
#define ADC_REFSEL_Vdd_1_2 2
#define ADC_REFSEL_Vdd_1_3 3
#define ADC_CONFIG_PSEL __FIELD(8, 8)
#define ADC_CONFIG_EXTREFSEL __FIELD(16, 2)
#define ADC_EXTREFSEL_Ref0 1
#define ADC_EXTREFSEL_Ref1 2
  uint32_t RESULT; // 0x508
};

/* Interrupts */
#define ADC_INT_END 0

extern volatile adc_t ADC;

/* A few assembler macros for single instructions. */
#define pause() asm volatile("wfe")
#define get_primask()                                                          \
  ({                                                                           \
    uint32_t x;                                                                \
    asm volatile("mrs %0, primask" : "=r"(x));                                 \
    x;                                                                         \
  })
#define set_primask(x) asm volatile("msr primask, %0" : : "r"(x))
#define nop() asm volatile("nop")

inline uint32_t get_msp(void)
{
  uint32_t msp;
  asm volatile("mrs %[stk], msp" : [stk] "=r"(msp));
  return msp;
}

inline void set_msp(void *stk_ptr)
{
  asm volatile("msr msp, %[stk]" : : [stk] "r"(stk_ptr));
}

