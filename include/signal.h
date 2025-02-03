#pragma once

#include "types.h"

using signal_handler_t = void (*)(void);

enum signal_t {
  SIGTERM = 0,
  SIGKILL,
  SIGMSSG,
  N_SIGNALS,
};

template <signal_t sig>
void default_handler(void);

template <signal_t sig>
class _signal_handler
{
  signal_handler_t handler;

public:
  _signal_handler(signal_handler_t handler) : handler(handler) {}

  void critical_section_start()
  {
    // swap
  }

  void critical_section_end()
  {
    // check if a sigterm was raised
    // swap
  }
};

struct signals_t {
  uint32_t raised = 0;
  signal_handler_t handlers[N_SIGNALS] = {
      default_handler<SIGTERM>,
      default_handler<SIGKILL>,
      default_handler<SIGMSSG>,
  };

  void send_signal(signal_t sig) {
    raised |= (1 << sig);
  }
};

void send_signal(pid_t pid, signal_t sig);
