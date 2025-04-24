#pragma once

#include "core/types.h"
#include "utility/format.h"

constexpr size_t args_len = 64;

struct args_buffer_t {
  size_t sz = 0;
  char str[args_len] = {'\0'};

  char &operator[](size_t i) { return str[i]; }

  void push(char c)
  {
    if (sz < args_len)
      str[sz++] = c;
  }

  void pop()
  {
    if (sz > 0)
      sz--;
  }

  void reset() { sz = 0; }
};

struct args_t {
  char str[args_len];
  bool run_bg;
};

struct parser_t {
  char *cmd, *args;
  fn_t fn;
  bool run_bg;

  enum state_t {
    CMD_START,
    CMD,
    ARGS_START,
    ARGS,
    FN,
    END,
  };

  parser_t(char *const str)
      : cmd{nullptr}, args{nullptr}, fn{null_fn}, run_bg{false}
  {
    if (!str)
      return;

    auto isspace = [](char c) { return c == ' '; };
    auto isdigit = [](char c) { return ('0' <= c && c <= '9'); };
    auto isalph = [](char c) {
      return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
    };

    state_t st = CMD_START;

    auto p = str;
    while (true) {
      auto c = *p;

      switch (st) {
      case CMD_START: {
        if (isspace(c))
          break;

        if (c == '\0' || !isalph(c))
          return;

        st = CMD;
        cmd = p;
        break;
      }

      case CMD: {
        if (c == '\0')
          return;

        if (!isspace(c) && c != '>' && c != '&')
          break;

        *p = '\0'; // mark end of command
        if (isspace(c))
          st = ARGS_START;
        else if (c == '>')
          st = FN;
        else {
          run_bg = true;
          return;
        }
        break;
      }

      case ARGS_START: {
        if (c == '\0')
          return;

        if (isspace(c))
          break;

        if (c != '>' && c != '&') {
          args = p;
          st = ARGS;
          break;
        }

        if (c == '>') {
          st = FN;
          break;
        }

        if (c == '&') {
          run_bg = true;
          return;
        }
      }

      case ARGS: {
        if (c != '>' && c != '&' && c != '\0')
          break;

        auto q = p;
        while (isspace(*--q))
          ;
        *++q = '\0'; // mark end of args

        if (c == '\0')
          return;

        if (c == '>') {
          st = FN;
          break;
        }

        if (c == '&') {
          run_bg = true;
          return;
        }
      }

      case FN: {
        if (isspace(c))
          break;

        if (c == '\0')
          return; // FIXME ERROR

        if (isdigit(c)) {
          const char *end = nullptr;
          auto ufn = atou(p, &end);

          if (ufn <= MAX<fn_t>)
            fn = (fn_t)ufn;
          else
            return;

          p = (char *)end - 1;
          st = END;
          break;
        }

        if (c == '&') {
          run_bg = true;
          return;
        }

        return;
      }

      case END: {
        if (c == '\0')
          return;

        if (c == '&') {
          run_bg = true;
          return;
        }

        break;
      }
      }
      p++;
    }
  }
};
