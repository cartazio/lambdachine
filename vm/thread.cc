#include "thread.hh"
#include "utils.hh"
#include "miscclosures.hh"

#include <stdio.h>
#include <stdlib.h>

_START_LAMBDACHINE_NAMESPACE

void Thread::destroy() {
  if (stack_ != NULL) {
    delete[] stack_;
  }
  stack_ = NULL;
  base_ = NULL;
  top_ = NULL;
}

// Used to initialize a new thread.
BcIns Thread::stopCode_[] = { BcIns::ad(BcIns::kSTOP, 0, 0) };

void Thread::initialize(Word stackSizeInWords) {
  header_ = 0;
  base_ = NULL;
  top_ = NULL;
  owner_ = NULL;
  stack_ = NULL;
  if (stackSizeInWords < kMinStackWords) {
    stackSizeInWords = kMinStackWords;
  }
  pc_ = &stopCode_[0];
  stackSize_ = stackSizeInWords;
  stack_ = new Word[stackSize_];
#ifndef NDEBUG
  // This is mainly to shut up Valgrind
  memset(stack_, 0xf0, stackSize_ * sizeof(Word));
#endif
  stack_[0] = (Word)NULL;   // previous base
  stack_[1] = (Word)NULL;   // previous PC
  stack_[2] = (Word)MiscClosures::stg_STOP_closure_addr;
  stack_[3] = (Word)NULL;   // The closure to evaluate.
  base_ = &stack_[3];
  top_ = &stack_[4];
  if (MiscClosures::stg_STOP_closure_addr == NULL) {
    fprintf(stderr, "FATAL: Loader must be initialized "
            "before creating first thread.\n");
    exit(1);
  }
  CodeInfoTable *info = static_cast<CodeInfoTable *>
                        (MiscClosures::stg_STOP_closure_addr->info());
  pc_ = &info->code()->code[0];
}

Thread *Thread::createThread(Capability *cap, Word stackSizeInWords) {
  Thread *T = new Thread;
  T->initialize(stackSizeInWords);
  T->owner_ = cap;
  return T;
}

Thread *Thread::createTestingThread(BcIns *pc, u4 framesize) {
  Thread *T = new Thread;
  T->initialize(framesize);
  T->base_ = T->stack_ + 1;
  T->top_ = T->base_ + framesize;
  T->pc_ = pc;
  return T;
}

#include <stdio.h>

#define CHECK_VALID(expr) if(!(expr)) { fprintf(stderr,  #expr "\n"); return false;}

bool Thread::isValid() const {
#ifndef NDEBUG
  CHECK_VALID(stackSize_ >= kMinStackWords);
  if (stack_ != NULL) {
    CHECK_VALID(within(stack_, stack_ + stackSize_, base_));
    CHECK_VALID(within(stack_, stack_ + stackSize_, top_));
    CHECK_VALID(top_ >= base_);
  }
  CHECK_VALID(pc_ != NULL);
#endif
  return true;
}

_END_LAMBDACHINE_NAMESPACE
