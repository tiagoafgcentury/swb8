#pragma once

#include <signal.h>
#include <stdio.h>

#ifndef NDEBUG
#define mb_assert(V) do { if (!(V)) { fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #V, __FILE__, __LINE__); /*raise(SIGINT);*/ } } while (0)
#else
#define mb_assert(V) do { } while (0)
#endif

