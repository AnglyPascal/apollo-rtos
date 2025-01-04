#include "serial.h"
#include "hardware.h"
#include "irq.h"
#include "lib.h"

namespace serial
{

namespace
{
/* Pins to use for serial communication */
constexpr auto TX = USB_TX;
constexpr auto RX = USB_RX;

volatile int txinit; /* UART ready to transmit first char */
} // namespace

/* init -- set up UART connection to host */
void init(void)
{
  UART.ENABLE = UART_ENABLE_Disabled;
  UART.BAUDRATE = UART_BAUDRATE_9600; /* 9600 baud */
  UART.CONFIG = FIELD(UART_CONFIG_PARITY, UART_PARITY_None);

  /* format 8N1 */
  UART.PSELTXD = TX; /* choose pins */
  UART.PSELRXD = RX;
  UART.ENABLE = UART_ENABLE_Enabled;

  UART.STARTTX = 1;
  UART.STARTRX = 1;
  UART.RXDRDY = 0;
  txinit = 1;

  UART.INTENSET = BIT(UART_INT_RXDRDY) | BIT(UART_INT_TXDRDY);
  enable_irq(UART_IRQ);
}

/* wait for input character and return it */
int getc(void)
{
  while (!UART.RXDRDY)
    ;
  char ch = UART.RXD;
  UART.RXDRDY = 0;
  return ch;
}

void listener(char c);

__extern_C__
void uart_handler(void)
{
  /* intr_disable(); */

  if (UART.RXDRDY) {
    char ch = UART.RXD;
    putc(ch);
    listener(ch);
    UART.RXDRDY = 0;
  }

  if (UART.TXDRDY) {
    txinit = 1;
    UART.TXDRDY = 0;
  }

  clear_pending(UART_IRQ);
  enable_irq(UART_IRQ);

  /* intr_enable(); */
}

/* putc -- send output character */
void putc(char ch)
{
  if (!txinit) {
    while (!UART.TXDRDY)
      ;
  }
  // FIXME: what's the correct way of doing IO?
  txinit = 0;
  UART.TXDRDY = 0;
  UART.TXD = ch;
}

/* puts -- send a string character by character */
void puts(const char *s)
{
  while (*s != '\0')
    putc(*s++);
}

/* puts -- send a string character by character */
void puts(const char *s, size_t len)
{
  while (len-- > 0 && *s != '\0')
    putc(*s++);
}

/* getline -- input a line of text into buf with line editing */
void getline(const char *prompt, char *buf, int nbuf)
{
  char *p = buf;

  puts(prompt);

  while (1) {
    char x = getc();

    switch (x) {
    case '\b':
    case 0177:
      if (p > buf) {
        p--;
        puts("\b \b");
      }
      break;

    case '\r':
      *p = '\0';
      puts("\r\n");
      return;

    default:
      /* Ignore other non-printing characters */
      if (x >= 040 && x < 0177 && p < &buf[nbuf]) {
        *p++ = x;
        putc(x);
      }
    }
  }
}

} // namespace serial
