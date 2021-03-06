#ifndef _LAMBDACHINE_THREAD_H
#define _LAMBDACHINE_THREAD_H

#include "Common.h"
#include "VM.h"
#include "Capability.h"
#include "Bytecode.h"
#include "InfoTables.h"

struct Thread_ {
  Word       header;
  BCIns      *pc;
  u4         stack_size;        /* Stack size in _words_ */
  Word       *base;             /* The current base pointer. */
  Word       *top;              /* Top of stack */
  Word       last_result;
  Word       stack[FLEXIBLE_ARRAY];
};

Thread *createThread(Capability *cap, u4 stack_size);

int stackOverflow(Thread *, Word *top, u4 increment);

Closure *startThread(Thread *, Closure *);

#define THREAD_STRUCT_SIZE  (sizeof(Thread))

#define THREAD_STRUCT_SIZEW (THREAD_STRUCT_SIZE / sizeof(Word))

// -- Stuff likely to be removed -------------------------------------

void printFrame(FILE *, Word *base, Word *top);
void printSlot(FILE *, Word *slot);

#endif
