#include "serial.h"
#include "debug.h"
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

static constexpr size_t NBUF = 64; /* Buffer size */

static volatile int txidle;       /* Whether UART is idle */
static volatile int bufcnt = 0;   /* Number of chars in buffer */
static unsigned bufin = 0;        /* Index of first free slot */
static unsigned bufout = 0;       /* Index of first occupied slot */
static volatile char txbuf[NBUF]; /* The buffer */

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
  UART.TXDRDY = 0;

  UART.INTENSET = BIT(UART_INT_RXDRDY) | BIT(UART_INT_TXDRDY);
  enable_irq(UART_IRQ);
  txidle = 1;
}

/* buf_put -- add character to buffer */
void buf_put(char ch)
{
  txbuf[bufin] = ch;
  bufcnt++;
  bufin = (bufin + 1) % NBUF;
}

/* buf_get -- fetch character from buffer */
char buf_get(void)
{
  char ch = txbuf[bufout];
  bufcnt--;
  bufout = (bufout + 1) % NBUF;
  return ch;
}

/** FIXME: works for now, but what about processes that are asleep?
 *  not a good implementation */
namespace
{
class listeners_t
{
  listener_t stack[16] = {nullptr};
  size_t idx = 0;

public:
  void push(listener_t listener)
  {
    stack[idx++] = listener;
  }

  void pop()
  {
    assert(idx > 0);
    idx--;
  }

  void operator()(char c)
  {
    assert(idx > 0);
    stack[idx - 1](c);
  }
};

listeners_t listeners;
} // namespace

void register_listener(listener_t listener)
{
  listeners.push(listener);
}

void unregister_listener()
{
  listeners.pop();
}

__extern_C__
void uart_handler(void)
{
  /* intr_guard guard; */

  if (UART.RXDRDY) {
    char ch = UART.RXD;
    listeners(ch);
    UART.RXDRDY = 0;
  }

  if (UART.TXDRDY) {
    UART.TXDRDY = 0;
    if (bufcnt == 0)
      txidle = 1;
    else
      UART.TXD = buf_get();
  }

  clear_pending(UART_IRQ);
  enable_irq(UART_IRQ);
}

/* putc -- send output character */
void putc(char ch)
{
  while (bufcnt == NBUF)
    pause();

  intr_disable();
  if (txidle) {
    UART.TXD = ch;
    txidle = 0;
  } else {
    buf_put(ch);
  }
  intr_enable();
}

/* puts -- send a string character by character */
void puts(const char *s, size_t len)
{
  while (len-- > 0 && *s != '\0')
    putc(*s++);
}

void clear_screen()
{
  printf("\033[2J\033[H");
}

} // namespace serial
