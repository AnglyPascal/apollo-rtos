#include "drivers/serial.h"
#include "core/hardware.h"
#include "core/irq.h"
#include "utility/circular_buffer.h"

namespace serial
{

namespace
{
/* Pins to use for serial communication */
constexpr auto TX = USB_TX;
constexpr auto RX = USB_RX;

static volatile int txidle; /* Whether UART is idle */

static constexpr size_t NBUF = 64; /* Buffer size */
circular_buffer<char, NBUF> buf;

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
  irq_priority(UART_IRQ, IRQ_PRIO_LO);

  txidle = 1;
}

namespace
{
class listeners_t
{
  listener_t stack[16] = {nullptr};
  size_t idx = 0;

public:
  void push(listener_t listener) { stack[idx++] = listener; }

  void pop()
  {
    assert(idx > 0, TERM);
    idx--;
  }

  void operator()(char c)
  {
    auto i = idx;
    while (i > 0) {
      if (stack[--i](c))
        break;
    }
  }
};

listeners_t listeners;
} // namespace

void register_listener(listener_t listener) { listeners.push(listener); }

void unregister_listener() { listeners.pop(); }

__extern_C__
void uart_handler(void)
{
  if (UART.RXDRDY) {
    char ch = UART.RXD;
    listeners(ch);
    UART.RXDRDY = 0;
  }

  if (UART.TXDRDY) {
    UART.TXDRDY = 0;
    if (buf.empty())
      txidle = 1;
    else
      UART.TXD = buf.dequeue();
  }

  clear_pending(UART_IRQ);
  enable_irq(UART_IRQ);
}

/* putc -- send output character */
void intr_putc(char ch)
{
  while (buf.size() == NBUF)
    pause();

  if (txidle) {
    UART.TXD = ch;
    txidle = 0;
  } else {
    buf.enqueue(ch);
  }
}

void busy_putc(char ch)
{
  if (!txidle) {
    while (!UART.TXDRDY)
      ;
  }
  txidle = 0;
  UART.TXDRDY = 0;
  UART.TXD = ch;
}

char getc()
{
  while (!UART.RXDRDY)
    ;
  char ch = UART.RXD;
  UART.RXDRDY = 0;
  return ch;
}

} // namespace serial
