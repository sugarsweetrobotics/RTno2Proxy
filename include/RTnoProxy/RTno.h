#pragma once

#define DEBUG

#ifdef DEBUG
#include <stdio.h>
#define dbg(fmt, ...) printf("[RTnoProxy]:" fmt "\n", ## __VA_ARGS__)
#else
#define dbg(...)
#endif
 
