#ifndef _IR_H_
#define _IR_H_

#include "common.hh"

#include <vector>
#include <iostream>


_START_LAMBDACHINE_NAMESPACE

typedef u2 IRRef1;              /* One stored reference */
typedef u4 IRRef2;            /* Two stored references */
typedef u4 IRRef;               /* Used to pass around references */

// A side effect of this design is that we have to use two IR instructions
// per store, because a store takes three arguments (base pointer,
// offset, and value).  A store is therefore encoded as:
//
//     ref  FREF   base offs   ; reference to a field
//     void FSTORE ref value
//
// Flags:
//   N .. normal
//   C .. commutative
//   G .. guard
//   A .. alloc
//
// Operand Types:
//   ref .. IR reference
//   lit .. 16 bit unsigned literal
//   cst .. 32/64bit constant (pointer, int)
//   none .. unised operand
//
#define IRDEF(_) \
  _(NOP,     N,   ___, ___) \
  _(BASE,    N,   lit, lit) \
  _(FRAME,   S,   lit, lit) \
  _(RET,     S,   lit, lit) \
  _(LOOP,    N,   ___, ___) \
  _(PHI,     N,   ref, ref) \
  _(RENAME,  S,   ref, lit) \
  \
  _(KINT,    N,   cst, ___) \
  _(KWORD,   N,   cst, ___) \
  _(KBASEO,  N,   cst, ___) \
  \
  _(LT,      G,   ref, ref) \
  _(GE,      G,   ref, ref) \
  _(LE,      G,   ref, ref) \
  _(GT,      G,   ref, ref) \
  _(EQ,      G,   ref, ref) \
  _(NE,      G,   ref, ref) \
  _(EQRET,   G,   ref, ref) \
  _(EQINFO,  G,   ref, ref) \
  _(HEAPCHK, G,   cst, ___) \
   \
  _(BNOT,    N,   ref, ___) \
  _(BAND,    C,   ref, ref) \
  _(BOR,     C,   ref, ref) \
  _(BXOR,    C,   ref, ref) \
  _(BSHL,    N,   ref, ref) \
  _(BSHR,    N,   ref, ref) \
  _(BSAR,    N,   ref, ref) \
  _(BROL,    N,   ref, ref) \
  _(BROR,    N,   ref, ref) \
  \
  _(ADD,     C,   ref, ref) \
  _(SUB,     N,   ref, ref) \
  _(MUL,     C,   ref, ref) \
  _(DIV,     N,   ref, ref) \
  _(REM,     N,   ref, ref) \
  _(NEG,     N,   ref, ___) \
  \
  _(FREF,    R,   ref, lit) \
  _(FLOAD,   L,   ref, ___) \
  _(SLOAD,   L,   lit, lit) \
  _(ILOAD,   L,   ref, ___) \
  _(RLOAD,   L,   ___, ___) \
  _(NEW,     A,   ref, lit) \
  _(FSTORE,  S,   ref, ref) \
  _(UPDATE,  S,   ref, ref) \
  _(SAVE,    S,   lit, ___)
/*
 * LuaJIT IR
 *
 * LSB                           MSB
 *
 *    16      16     8   8   8   8
 * +-------+-------+---+---+---+---+
 * |  op1  |  op2  | t | o | r | s |
 * +-------+-------+---+---+---+---+
 * |  op12/i/gco   |  ot   | prev  |
 * +---------------+-------+-------+
 *        32          16      16
 *
 *   o .. opcode
 *   t .. IR type + flags
 *   r .. register allocation
 *   s .. spill slot allocation
 *
 *   prev .. Points to previous instruction with same opcode (or 0).
 *           Used for fast CSE.  Only valid before register allocation.
 */

typedef union _IRInsRaw {
  struct {
    LC_ENDIAN_LOHI(
      IRRef1 op1;               // First operand.
    , IRRef1 op2;               // Second operand
    )
    u2 ot;                      // Opcode + type
    IRRef1 prev;                // Previous instruction of same opcode
  };
  struct {
    IRRef2 op12;                // Unified operands.  (op1 is low)
    LC_ENDIAN_LOHI(
      u1 t;
    , u1 o;
    )
    LC_ENDIAN_LOHI(
      u1 r;
    , u1 s;
    )
  };

  // TODO: I think we need more LC_ENDIAN_LOHI to make sure that these
  // overlap with op1/2, not ot/prev

  i4 i;   // A literal 32bit integer
  u4 u;   // Index into J->kwords
} IRIns;

// type name, printed string (len=3), color
#define IRTDEF(_) \
  _(UNKNOWN, "unk", GREY) \
  _(VOID,    "   ", NONE) \
  _(I64,     "i64", PRIM) \
  _(U64,     "u64", PRIM) \
  _(I32,     "i32", PRIM) \
  _(U32,     "u32", PRIM) \
  _(I16,     "i16", PRIM) \
  _(U16,     "u16", PRIM) \
  _(I8,      "i8 ", PRIM) \
  _(U8,      "u8 ", PRIM) \
  _(F64,     "f64", PRIM) \
  _(F32,     "f32", PRIM) \
  _(CHR,     "chr", PRIM) /* could be made = u32 */ \
  _(CLOS,    "cls", HEAP) /* pointer to a closure */ \
  _(INFO,    "inf", NONE) /* pointer to info table */ \
  _(PC,      "pc ", NONE) /* pointer to bytecode address */ \
  _(PTR,     "ptr", NONE) /* internal pointer */
/* 32 types in total allowed */

typedef enum {
#define IRTENUM(name, str, col) IRT_##name,
  IRTDEF(IRTENUM)
#undef IRTENUM

  // Flags
  IRT_MARK  = 0x20,  // Marker for various purposes
  IRT_ISPHI = 0x40,  // Used by register allocator
  IRT_GUARD = 0x80,  // Used by asm code generator

  // Masks
  IRT_TYPE = 0x1f,
  IRT_T    = 0xff
} IRType;

static const uint32_t kOpIsSigned =
  (1u << (int)IRT_I64) | (1u << (int)IRT_I32) |
  (1u << (int)IRT_I16) | (1u << (int)IRT_I8);

static const uint32_t kOpIsInteger =
  (1u << (int)IRT_CHR) |
  (1u << (int)IRT_U64) | (1u << (int)IRT_U32) |
  (1u << (int)IRT_U16) | (1u << (int)IRT_U8)  |
  (1u << (int)IRT_I64) | (1u << (int)IRT_I32) |
  (1u << (int)IRT_I16) | (1u << (int)IRT_I8);

inline bool isIntegerType(IRType t) {
  return kOpIsInteger & (1u << (int)t);
}

static const uint32_t kOpIsFloat =
  (1u << (int)IRT_F32) | (1u << (int)IRT_F64);

inline bool isFloatType(IRType t) {
  return kOpIsFloat & (1 << (int)t);
}

#define IRT(o, t)      (cast(u4, ((o) << 8) | (t)))

class IRBuffer; // Defined below.

class IR {
public:
  IR() : data_() {}
  IR(IRIns d) : data_(d) { }
  ~IR() {}
  // Opcodes
  typedef enum {
#define IRENUM(name,flags,arg1,arg2) k##name,
    IRDEF(IRENUM)
#undef IRENUM
    k_MAX
  } Opcode;

  // Properties of the instruction.
  typedef enum {
    IRMref = 0,
    IRMlit = 1,
    IRMcst = 2,
    IRMnone = 3,
    IRM___ = IRMnone,

    IRM_N = 0x00,  // normal, CSE ok
    IRM_R = IRM_N,
    IRM_C = 0x10,  // commutative
    IRM_A = 0x20,  // allocation (no CSE)
    IRM_L = 0x40,  // load
    IRM_S = 0x60,  // store
    IRM_G = 0x80   // guard
  } Mode;

  typedef uint8_t IRMode;
  typedef uint8_t Type;

  inline Opcode opcode() { return (Opcode)data_.o; }
  inline Type t() { return data_.t; }
  inline IRType type() { return (IRType)(data_.t & IRT_TYPE); }
  inline uint16_t ot() { return data_.ot; }
  inline IRRef1 op1() { return data_.op1; }
  inline IRRef1 op2() { return data_.op2; }
  inline IRRef2 op12() { return data_.op12; }
  inline int32_t i32() { return data_.i; }
  inline uint32_t u32() { return data_.u; }

  inline void setOpcode(Opcode op) { data_.o = op; }
  inline void setT(uint8_t ty) { data_.t = ty; }
  inline void setOt(uint16_t ot) { data_.ot = ot; }
  inline void setOp1(IRRef1 op1) { data_.op1 = op1; }
  inline void setOp2(IRRef1 op2) { data_.op2 = op2; }
  inline void setPrev(IRRef1 prev) { data_.prev = prev; }
  inline IRRef1 prev() { return data_.prev; }

  static inline IRMode mode(Opcode op) {
    LC_ASSERT(0 <= op && op <= k_MAX);
    return mode_[op];
  }

  static inline bool isCommutative(Opcode op) {
    return mode_[op] & IRM_C;
  }

  static inline bool hasSideEffect(Opcode op) {
    return mode_[op] & IRM_S;
  }

  static void printIRRef(std::ostream &out, IRRef ref);
  void debugPrint(std::ostream &out, IRRef self) {
    debugPrint(out, self, NULL);
  }
  void debugPrint(std::ostream &out, IRRef self, IRBuffer *buf);

private:
  IR(u1 opc, u1 ty, IRRef op1, IRRef op2, u2 prev) {
    data_.ot = (opc << 8) | ty;
    data_.op1 = op1;
    data_.op2 = op2;
    data_.prev = prev;
  }

  static uint8_t mode_[k_MAX + 1];
  static const char *name_[k_MAX + 1];

  friend class IRBuffer;

  IRIns data_;
};

enum {
  REF_BIAS  = 0x8000,
  REF_BASE  = REF_BIAS,
  REF_FIRST = REF_BIAS + 1,
  REF_DROP  = 0xffff
};

inline bool irref_islit(IRRef ref) { return ref < REF_BIAS; }

// A tagged reference
// Tagged IR references (32 bit).
//
// +-------+-------+---------------+
// |  irt  | flags |      ref      |
// +-------+-------+---------------+
//
// The tag holds a copy of the IRType and speeds up IR type checks.
//
class TRef {
public:
  TRef(IRRef1 ref, IR::Type type) {
    raw_ = (uint32_t)ref + ((uint32_t)type << 24);
  }
  TRef() : raw_(0) { }
  inline IRRef1 ref() const { return (IRRef1)raw_; }
  inline IR::Type t() const { return (IR::Type)(raw_ >> 24); }
  inline bool isNone() const { return raw_ == 0; }
  inline bool isLiteral() const { return ref() < REF_BASE; }
  inline void markWritten() { raw_ |= kWritten; }
  
  bool operator==(const TRef &t) const { return raw_ == t.raw_; }
  bool operator!=(const TRef &t) const { return raw_ != t.raw_; }
  operator IRRef1() { return (IRRef1)raw_; }

private:
  uint32_t raw_;

  static const uint32_t kWritten = 0x10000;

  friend class AbstractStack;
};

class SnapshotData;

typedef uint16_t SnapNo;

class Snapshot {
public:
  inline IRRef1 ref() const { return ref_; }
  void debugPrint(std::ostream&, SnapshotData *, SnapNo);
  inline int entries() const { return entries_; }
  Snapshot() {}
private:

  IRRef1 ref_;
  uint16_t mapofs_;
  int16_t relbase_; // base relative to trace entry base pointer
  uint8_t entries_;
  uint8_t framesize_;
  uint16_t exitCounter_;
  uint16_t unused;
  void *pc_;
  friend class AbstractStack;
};


class SnapshotData {
public:
  SnapshotData();
private:
  inline int slotId(size_t index) {
    return (int)(data_.at(index) >> 16);
  }
  inline IRRef1 slotRef(size_t index) {
    return (IRRef1)data_.at(index);
  }

  std::vector<uint32_t> data_;
  size_t index_;

  friend class AbstractStack;
  friend class Snapshot;
};


class AbstractStack {
public:
  AbstractStack(Word *base, Word *top);

  inline TRef get(int n) const {
    return slots_[base_ + n];
  }

  inline void clear(int n) {
    slots_[base_ + n] = TRef();
  }

  inline void set(int n, TRef value) {
    unsigned int slot = base_ + n;
    slots_[slot] = value;
    if (slot < low_) low_ = slot;
    if (slot > high_) high_ = slot;
  }

  inline unsigned int top() const {
    return top_ - base_;
  }

  inline int absolute(int n) const {
    return (base_ + n) - kInitialBase;
  }

  inline TRef getAbsolute(int n) const {
    return slots_[kInitialBase + n];
  }

  // Set the current frame.  Returns false in case of over-/underflow.
  bool frame(Word *base, Word *top);

  inline int baseOffset(Word *p) {
    return p - realOrigBase_;
  }

  // TODO: Create snapshots.
  void snapshot(Snapshot *snap, SnapshotData *snapmap,
                IRRef1 ref, void *pc);

private:
  static const unsigned int kSlots = 500;
  static const unsigned int kInitialBase = 250;

  TRef *slots_;
  unsigned int base_; // current base
  unsigned int top_;  // current top
  unsigned int low_;  // lowest modified value
  unsigned int high_; // highest modified value
  Word *realOrigBase_;
};


typedef struct _FoldState {
  IR ins;
  IR left;
  IR right;
  uint64_t literal;
} FoldState;

class IRBuffer {
public:
  IRBuffer(Word *base, Word *top);
  ~IRBuffer();

  inline IRRef nextIns() {
    IRRef ref = bufmax_;
    if (LC_UNLIKELY(ref >= bufend_)) growTop();
    bufmax_ = ref + 1;
    return ref;
  }

  inline IRRef nextLit() {
    IRRef ref = bufmin_;
    if (LC_UNLIKELY(ref <= bufstart_)) growBottom();
    bufmin_ = --ref;
    return ref;
  }

  inline TRef emit(uint8_t o, uint8_t t, IRRef1 op1, IRRef1 op2) {
    return emit(IRT(o, t), op1, op2);
  }

  inline TRef emit(uint16_t ot, IRRef1 op1, IRRef1 op2) {
    set(ot, op1, op2);
    return optFold();
  }

  /// Emit instruction in current fold state without passing it
  /// through the optimisation pipeline.
  inline TRef emitRaw(uint16_t ot, IRRef1 op1, IRRef1 op2) {
    set(ot, op1, op2);
    return emit();
  }

  inline TRef slot(int n) {
    TRef s = slots_.get(n);
    if (s.isNone()) {
      s = emitRaw(IRT(IR::kSLOAD, IRT_UNKNOWN), slots_.absolute(n), 0);
      slots_.set(n, s);
    }
    return s;
  }

  inline void setSlot(int n, TRef tr) {
    tr.markWritten();
    slots_.set(n, tr);
  }

  TRef literal(IRType ty, uint64_t lit);
  uint64_t literalValue(IRRef ref);

  /// A literal that represents a pointer into the stack.
  TRef baseLiteral(Word *p);
  TRef optFold();
  TRef optCSE();

  inline int size() { return (bufmax_ - bufmin_); }

  inline IR *ir(IRRef ref) {
    LC_ASSERT(bufmin_ <= ref && ref <= bufmax_);
    return &buffer_[ref];
  }

  void debugPrint(std::ostream&, int traceNo);
  inline Word kword(uint32_t n) {
    if (n < kwords_.size())
      return kwords_[n];
    else
      return 0;
  }

  static const int kOptCSE = 0;
  static const int kOptFold = 1;

  inline void enableOptimisation(int optId) { flags_.set(optId); }
  inline void disableOptimisation(int optId) { flags_.clear(optId); }

  inline void snapshot(Snapshot *snap, SnapshotData *snapmap, void *pc) {
    slots_.snapshot(snap, snapmap, bufmax_, pc);
  }

private:
  void growTop();
  void growBottom();
  TRef emit(); // Emit without optimisation.

  IRRef doFold();

  inline void set(uint16_t ot, IRRef1 op1, IRRef op2) {
    fold_.ins.setOt(ot);
    fold_.ins.setOp1(op1);
    fold_.ins.setOp2(op2);
  }

  inline IR *fins() {
    return &fold_.ins;
  }

  IR *realbuffer_;
  IR *buffer_;  // biased
  Flags32 flags_;
  IRRef bufmin_;
  IRRef bufmax_;
  IRRef bufstart_;
  IRRef bufend_;
  size_t size_;
  FoldState fold_;
  IRRef1 chain_[IR::k_MAX];
  AbstractStack slots_;
  std::vector<Word> kwords_;
};

// Can invert condition by toggling lowest bit.
LC_STATIC_ASSERT((IR::kLT ^ 1) == IR::kGE);
LC_STATIC_ASSERT((IR::kGT ^ 1) == IR::kLE);
LC_STATIC_ASSERT((IR::kEQ ^ 1) == IR::kNE);
// Order of comparison operations matters.  Same is enforced for bytecode.
LC_STATIC_ASSERT((IR::kLT & 1) == 0);
LC_STATIC_ASSERT((IR::kLT + 2) == IR::kLE);
LC_STATIC_ASSERT((IR::kLE + 2) == IR::kEQ);

_END_LAMBDACHINE_NAMESPACE

#endif /* _IR_H_ */
