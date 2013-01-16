#ifndef _LAMBDACHINE_CONFIG_H
#define _LAMBDACHINE_CONFIG_H

#include "autoconfig.h"

#ifndef LC_HAS_JIT
# define LC_HAS_JIT      1
#endif

#ifndef LC_HAS_ASM_BACKEND
# define LC_HAS_ASM_BACKEND 1
#endif

/* #define LC_SELF_CHECK_MODE */

/* #undef NDEBUG */
/* #define DEBUG */

#define LC_USE_VALGRIND 1

#ifndef LC_DEBUG_LEVEL
# ifdef NDEBUG
#  define LC_DEBUG_LEVEL  0
# else
#  define LC_DEBUG_LEVEL  1
# endif
#endif

#define LC_JIT   1

// #define LC_DUMP_TRACES
// #define LC_TRACE_STATS
#define LC_CLEAR_DOM_COUNTERS

#define MAX_HEAP_ENTRIES      300

#define HOT_THRESHOLD            53
#define HOT_SIDE_EXIT_THRESHOLD  7

#define LC_DEFAULT_HEAP_SIZE  (1UL * 1024 * 1024)


#define DEBUG_MEMORY_MANAGER  0x00000001L
#define DEBUG_LOADER          0x00000002L
#define DEBUG_INTERPRETER     0x00000004L
#define DEBUG_TRACE_RECORDER  0x00000008L
#define DEBUG_ASSEMBLER       0x00000010L
#define DEBUG_TRACE_ENTEREXIT 0x00000020L
#define DEBUG_FALSE_LOOP_FILT 0x00000040L
#define DEBUG_SANITY_CHECK_GC 0x00000080L
#define DEBUG_TRACE_CREATION  0x00000100L
#define DEBUG_TRACE_PROGRESS  0x00000200L

#ifdef NDEBUG
# define DEBUG_COMPONENTS 0
#else
// #define DEBUG_COMPONENTS     0xffffffffL
// #define DEBUG_COMPONENTS    (DEBUG_MEMORY_MANAGER|DEBUG_INTERPRETER)
// #define DEBUG_COMPONENTS    (DEBUG_ASSEMBLER|DEBUG_TRACE_ENTEREXIT)
// #define DEBUG_COMPONENTS    (DEBUG_ASSEMBLER|DEBUG_SANITY_CHECK_GC)
// #define DEBUG_COMPONENTS    (DEBUG_TRACE_ENTEREXIT|DEBUG_ASSEMBLER)
#define DEBUG_COMPONENTS 0
// #define DEBUG_COMPONENTS    (DEBUG_ASSEMBLER|DEBUG_TRACE_ENTEREXIT|DEBUG_TRACE_RECORDER)
// #define DEBUG_COMPONENTS    (DEBUG_TRACE_RECORDER|DEBUG_INTERPRETER)
#endif

#endif
