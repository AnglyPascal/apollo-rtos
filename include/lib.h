/* lib.h */
/* Copyright (c) 2018 J. M. Spivey */

#pragma once

#include <cstdarg>
#include <cstdint>

/* do_print -- the device-independent guts of printf */
void do_printf(void (*putch)(char), const char *fmt, ...);

/* atoi -- convert decimal string to int */
int32_t atoi(const char *p);

/* xtou -- convert hex string to unsigned */
uint32_t xtou(char *p);

/* prandom -- pseudo-random number in range [1..2^31-1) */
uint32_t prandom(void);
