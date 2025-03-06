#pragma once

#include "core/types.h"
#include "utility/debug.h"

using signal_handler_t = void (*)(void);

enum signal_t {
  SIGTERM = 0,
  SIGKILL,
  SIGMSSG,
  SIGDUMP,
  N_SIGNALS,
};

template <signal_t sig>
void default_handler(void);

template <>
void default_handler<SIGTERM>(void);
template <>
void default_handler<SIGKILL>(void);

template <>
inline void default_handler<SIGMSSG>(void) {};
template <>
inline void default_handler<SIGDUMP>(void) {};

struct signals_t {
private:
  uint32_t raised = 0;

  signal_handler_t handlers[N_SIGNALS] = {
      default_handler<SIGTERM>,
      default_handler<SIGKILL>,
      default_handler<SIGMSSG>,
      default_handler<SIGDUMP>,
  };

  static constexpr int sigmsk(int sig) { return 1 << sig; }

public:
  void send(signal_t sig) { raised |= sigmsk(sig); }

  void handle_signals()
  {
    if (raised == 0)
      return;

    for (int sig = 0; sig < N_SIGNALS; sig++) {
      auto msk = sigmsk(sig);
      if (raised & msk) {
        handlers[sig]();
        raised &= ~msk;
      }
    }
  }

  signal_handler_t swap(signal_t sig, signal_handler_t new_handler)
  {
    if (sig == SIGKILL) {
      debug<ERROR>("can't override sigkill handler\r\n");
      return new_handler;
    }

    auto old_handler = handlers[sig];
    handlers[sig] = new_handler;
    return old_handler;
  }
};

signal_handler_t swap_handler(signal_t sig, signal_handler_t new_handler);
void send_signal(pid_t pid, signal_t sig);

template <int>
class sigterm_handler_t
{
  inline static bool exit = false;

public:
  sigterm_handler_t()
  {
    swap_handler(SIGTERM, [] { exit = true; });
  }

  ~sigterm_handler_t() { exit = false; }

  bool run() const { return !exit; }
};
