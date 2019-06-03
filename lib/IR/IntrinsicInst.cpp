//===-- InstrinsicInst.cpp - Intrinsic Instruction Wrappers ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements methods that make it really easy to deal with intrinsic
// functions.
//
// All intrinsic function calls are instances of the call instruction, so these
// are all subclasses of the CallInst class.  Note that none of these classes
// has state or virtual methods, which is an important part of this gross/neat
// hack working.
//
// In some cases, arguments to intrinsics need to be generic and are defined as
// type pointer to empty struct { }*.  To access the real item of interest the
// cast instruction needs to be stripped away.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
/// DbgVariableIntrinsic - This is the common base class for debug info
/// intrinsics for variables.
///

Value *DbgVariableIntrinsic::getVariableLocation(bool AllowNullOp) const {
  Value *Op = getArgOperand(0);
  if (AllowNullOp && !Op)
    return nullptr;

  auto *MD = cast<MetadataAsValue>(Op)->getMetadata();
  if (auto *V = dyn_cast<ValueAsMetadata>(MD))
    return V->getValue();

  // When the value goes to null, it gets replaced by an empty MDNode.
  assert(!cast<MDNode>(MD)->getNumOperands() && "Expected an empty MDNode");
  return nullptr;
}

Optional<uint64_t> DbgVariableIntrinsic::getFragmentSizeInBits() const {
  if (auto Fragment = getExpression()->getFragmentInfo())
    return Fragment->SizeInBits;
  return getVariable()->getSizeInBits();
}

int llvm::Intrinsic::lookupLLVMIntrinsicByName(ArrayRef<const char *> NameTable,
                                               StringRef Name) {
  assert(Name.startswith("llvm."));

  // Do successive binary searches of the dotted name components. For
  // "llvm.gc.experimental.statepoint.p1i8.p1i32", we will find the range of
  // intrinsics starting with "llvm.gc", then "llvm.gc.experimental", then
  // "llvm.gc.experimental.statepoint", and then we will stop as the range is
  // size 1. During the search, we can skip the prefix that we already know is
  // identical. By using strncmp we consider names with differing suffixes to
  // be part of the equal range.
  size_t CmpStart = 0;
  size_t CmpEnd = 4; // Skip the "llvm" component.
  const char *const *Low = NameTable.begin();
  const char *const *High = NameTable.end();
  const char *const *LastLow = Low;
  while (CmpEnd < Name.size() && High - Low > 0) {
    CmpStart = CmpEnd;
    CmpEnd = Name.find('.', CmpStart + 1);
    CmpEnd = CmpEnd == StringRef::npos ? Name.size() : CmpEnd;
    auto Cmp = [CmpStart, CmpEnd](const char *LHS, const char *RHS) {
      return strncmp(LHS + CmpStart, RHS + CmpStart, CmpEnd - CmpStart) < 0;
    };
    LastLow = Low;
    std::tie(Low, High) = std::equal_range(Low, High, Name.data(), Cmp);
  }
  if (High - Low > 0)
    LastLow = Low;

  if (LastLow == NameTable.end())
    return -1;
  StringRef NameFound = *LastLow;
  if (Name == NameFound ||
      (Name.startswith(NameFound) && Name[NameFound.size()] == '.'))
    return LastLow - NameTable.begin();
  return -1;
}

Value *InstrProfIncrementInst::getStep() const {
  if (InstrProfIncrementInstStep::classof(this)) {
    return const_cast<Value *>(getArgOperand(4));
  }
  const Module *M = getModule();
  LLVMContext &Context = M->getContext();
  return ConstantInt::get(Type::getInt64Ty(Context), 1);
}

static
RoundingMode
DecodeRoundingMode(StringRef RoundingArg) {
  // For dynamic rounding mode, we use round to nearest but we will set the
  // 'exact' SDNodeFlag so that the value will not be rounded.
  return StringSwitch<RoundingMode>(RoundingArg)
    .Case("round.dynamic",    RoundingMode::rmDynamic)
    .Case("round.tonearest",  RoundingMode::rmToNearest)
    .Case("round.downward",   RoundingMode::rmDownward)
    .Case("round.upward",     RoundingMode::rmUpward)
    .Case("round.towardzero", RoundingMode::rmTowardZero)
    .Default(RoundingMode::rmInvalid);
}

static
ExceptionBehavior
DecodeExceptionBehavior(StringRef ExceptionArg) {
  return StringSwitch<ExceptionBehavior>(ExceptionArg)
    .Case("fpexcept.ignore",  ExceptionBehavior::ebIgnore)
    .Case("fpexcept.maytrap", ExceptionBehavior::ebMayTrap)
    .Case("fpexcept.strict",  ExceptionBehavior::ebStrict)
    .Default(ExceptionBehavior::ebInvalid);
}

RoundingMode
ConstrainedFPIntrinsic::getRoundingMode() const {
  unsigned NumOperands = getNumArgOperands();
  assert(NumOperands >= 2 && "underflow");
  Metadata *MD =
      dyn_cast<MetadataAsValue>(getArgOperand(NumOperands - 2))->getMetadata();
  if (!MD || !isa<MDString>(MD))
    return RoundingMode::rmInvalid;
  StringRef RoundingArg = cast<MDString>(MD)->getString();
  return DecodeRoundingMode(RoundingArg);
}

ExceptionBehavior
ConstrainedFPIntrinsic::getExceptionBehavior() const {
  unsigned NumOperands = getNumArgOperands();
  assert(NumOperands >= 1 && "underflow");
  Metadata *MD =
      dyn_cast<MetadataAsValue>(getArgOperand(NumOperands - 1))->getMetadata();
  if (!MD || !isa<MDString>(MD))
    return ExceptionBehavior::ebInvalid;
  StringRef ExceptionArg = cast<MDString>(MD)->getString();
  return DecodeExceptionBehavior(ExceptionArg);
}

CmpInst::Predicate
VPIntrinsic::getCmpPredicate() const {
  return static_cast<CmpInst::Predicate>(cast<ConstantInt>(getArgOperand(4))->getZExtValue());
}

RoundingMode
VPIntrinsic::getRoundingMode() const {
  unsigned NumOperands = getNumArgOperands();
  assert(NumOperands >= 4 && "underflow");
  Metadata *MD =
      dyn_cast<MetadataAsValue>(getArgOperand(NumOperands - 4))->getMetadata();
  if (!MD || !isa<MDString>(MD))
    return RoundingMode::rmInvalid;
  StringRef RoundingArg = cast<MDString>(MD)->getString();
  return DecodeRoundingMode(RoundingArg);
}

ExceptionBehavior
VPIntrinsic::getExceptionBehavior() const {
  unsigned NumOperands = getNumArgOperands();
  assert(NumOperands >= 3 && "underflow");
  Metadata *MD =
      dyn_cast<MetadataAsValue>(getArgOperand(NumOperands - 3))->getMetadata();
  if (!MD || !isa<MDString>(MD))
    return ExceptionBehavior::ebInvalid;
  StringRef ExceptionArg = cast<MDString>(MD)->getString();
  return DecodeExceptionBehavior(ExceptionArg);
}

bool VPIntrinsic::isUnaryOp() const {
  switch (getIntrinsicID()) {
    default:
      return false;
    case Intrinsic::vp_fneg:
    case Intrinsic::vp_constrained_sin:
    case Intrinsic::vp_constrained_cos:
    case Intrinsic::vp_constrained_exp:
    case Intrinsic::vp_constrained_exp2:
    case Intrinsic::vp_constrained_log:
    case Intrinsic::vp_constrained_log10:
    case Intrinsic::vp_constrained_log2:
    case Intrinsic::vp_constrained_sqrt:
    case Intrinsic::vp_constrained_ceil:
    case Intrinsic::vp_constrained_floor:
    case Intrinsic::vp_constrained_round:
    case Intrinsic::vp_constrained_trunc:
    case Intrinsic::vp_constrained_rint:
    case Intrinsic::vp_constrained_nearbyint:
      return true;
  }
}

Value*
VPIntrinsic::getMask() const {
  int offset = 0;
  if (isConstrainedOp()) offset += 2; // skip rounding, exception args

  if (isBinaryOp()) { return getArgOperand(offset + 2); }
  else if (isTernaryOp()) { return getArgOperand(offset + 3); }
  else if (isUnaryOp()) { return getArgOperand(offset + 1); }
  else return nullptr;
}

Value*
VPIntrinsic::getVectorLength() const {
  int offset = 0;
  if (isConstrainedOp()) offset += 2; // skip rounding, exception args

  if (isBinaryOp()) { return getArgOperand(offset + 3); }
  else if (isTernaryOp()) { return getArgOperand(offset + 4); }
  else if (isUnaryOp()) { return getArgOperand(offset + 2); }
  else return nullptr;
}

bool VPIntrinsic::isReductionOp() const {
  switch (getIntrinsicID()) {
    default:
      return false;

    case Intrinsic::vp_reduce_and:
    case Intrinsic::vp_reduce_or:
    case Intrinsic::vp_reduce_xor:

    case Intrinsic::vp_reduce_add:
    case Intrinsic::vp_reduce_mul:
    case Intrinsic::vp_reduce_fadd:
    case Intrinsic::vp_reduce_fmul:

    case Intrinsic::vp_reduce_fmin:
    case Intrinsic::vp_reduce_fmax:
    case Intrinsic::vp_reduce_smin:
    case Intrinsic::vp_reduce_smax:
    case Intrinsic::vp_reduce_umin:
    case Intrinsic::vp_reduce_umax:

      return true;
  }
}

bool VPIntrinsic::isConstrainedOp() const {
  switch (getIntrinsicID()) {
    default:
      return false;

    case Intrinsic::vp_constrained_fadd:
    case Intrinsic::vp_constrained_fsub:
    case Intrinsic::vp_constrained_fmul:
    case Intrinsic::vp_constrained_fdiv:
    case Intrinsic::vp_constrained_frem:
    case Intrinsic::vp_constrained_fma:
    case Intrinsic::vp_constrained_sqrt:
    case Intrinsic::vp_constrained_pow:
    case Intrinsic::vp_constrained_powi:
    case Intrinsic::vp_constrained_sin:
    case Intrinsic::vp_constrained_cos:
    case Intrinsic::vp_constrained_exp:
    case Intrinsic::vp_constrained_exp2:
    case Intrinsic::vp_constrained_log:
    case Intrinsic::vp_constrained_log10:
    case Intrinsic::vp_constrained_log2:
    case Intrinsic::vp_constrained_rint:
    case Intrinsic::vp_constrained_nearbyint:
    case Intrinsic::vp_constrained_maxnum:
    case Intrinsic::vp_constrained_minnum:
    case Intrinsic::vp_constrained_ceil:
    case Intrinsic::vp_constrained_floor:
    case Intrinsic::vp_constrained_round:
    case Intrinsic::vp_constrained_trunc:
      return true;
  }
}

bool VPIntrinsic::isBinaryOp() const {
  switch (getIntrinsicID()) {
    default:
      return false;

    case Intrinsic::vp_and:
    case Intrinsic::vp_or:
    case Intrinsic::vp_xor:
    case Intrinsic::vp_ashr:
    case Intrinsic::vp_lshr:
    case Intrinsic::vp_shl:

    case Intrinsic::vp_fadd:
    case Intrinsic::vp_fsub:
    case Intrinsic::vp_fmul:
    case Intrinsic::vp_fdiv:
    case Intrinsic::vp_frem:

    case Intrinsic::vp_reduce_or:
    case Intrinsic::vp_reduce_xor:
    case Intrinsic::vp_reduce_add:
    case Intrinsic::vp_reduce_mul:
    case Intrinsic::vp_reduce_smax:
    case Intrinsic::vp_reduce_smin:
    case Intrinsic::vp_reduce_umax:
    case Intrinsic::vp_reduce_umin:

    case Intrinsic::vp_reduce_fadd:
    case Intrinsic::vp_reduce_fmul:
    case Intrinsic::vp_reduce_fmax:
    case Intrinsic::vp_reduce_fmin:

    case Intrinsic::vp_add:
    case Intrinsic::vp_sub:
    case Intrinsic::vp_mul:
    case Intrinsic::vp_udiv:
    case Intrinsic::vp_sdiv:
    case Intrinsic::vp_urem:
    case Intrinsic::vp_srem:

    case Intrinsic::vp_constrained_fadd:
    case Intrinsic::vp_constrained_fsub:
    case Intrinsic::vp_constrained_fmul:
    case Intrinsic::vp_constrained_fdiv:
    case Intrinsic::vp_constrained_frem:
    case Intrinsic::vp_constrained_pow:
    case Intrinsic::vp_constrained_powi:
    case Intrinsic::vp_constrained_maxnum:
    case Intrinsic::vp_constrained_minnum:

      return true;
  }
}

bool VPIntrinsic::isTernaryOp() const {
  switch (getIntrinsicID()) {
    default:
      return false;
    case Intrinsic::vp_compose:
    case Intrinsic::vp_select:
    case Intrinsic::vp_fma:
    case Intrinsic::experimental_constrained_fma:
      return true;
  }
}

VPIntrinsic::VPIntrinsicDesc
VPIntrinsic::GetVPDescForIntrinsic(unsigned IntrinsicID) {
  switch (IntrinsicID) {
  default:
    return VPIntrinsicDesc{Intrinsic::not_intrinsic, TypeTokenVec(), -1, -1};

  // llvm.experimental.constrained.*
  case Intrinsic::experimental_constrained_cos: return VPIntrinsicDesc{ Intrinsic::vp_constrained_cos, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_sin: return VPIntrinsicDesc{ Intrinsic::vp_constrained_sin, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_exp: return VPIntrinsicDesc{ Intrinsic::vp_constrained_exp, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_exp2: return VPIntrinsicDesc{ Intrinsic::vp_constrained_exp2, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_log: return VPIntrinsicDesc{ Intrinsic::vp_constrained_log, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_log2: return VPIntrinsicDesc{ Intrinsic::vp_constrained_log2, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_log10: return VPIntrinsicDesc{ Intrinsic::vp_constrained_log10, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_sqrt: return VPIntrinsicDesc{ Intrinsic::vp_constrained_sqrt, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_ceil: return VPIntrinsicDesc{ Intrinsic::vp_constrained_ceil, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_floor: return VPIntrinsicDesc{ Intrinsic::vp_constrained_floor, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_round: return VPIntrinsicDesc{ Intrinsic::vp_constrained_round, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_trunc: return VPIntrinsicDesc{ Intrinsic::vp_constrained_trunc, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_rint: return VPIntrinsicDesc{ Intrinsic::vp_constrained_rint, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;
  case Intrinsic::experimental_constrained_nearbyint: return VPIntrinsicDesc{ Intrinsic::vp_constrained_nearbyint, TypeTokenVec{VPTypeToken::Vector}, 3, 4}; break;

  case Intrinsic::experimental_constrained_fadd: return VPIntrinsicDesc{ Intrinsic::vp_constrained_fadd, TypeTokenVec{VPTypeToken::Vector}, 4, 5}; break;
  case Intrinsic::experimental_constrained_fsub: return VPIntrinsicDesc{ Intrinsic::vp_constrained_fsub, TypeTokenVec{VPTypeToken::Vector}, 4, 5}; break;
  case Intrinsic::experimental_constrained_fmul: return VPIntrinsicDesc{ Intrinsic::vp_constrained_fmul, TypeTokenVec{VPTypeToken::Vector}, 4, 5}; break;
  case Intrinsic::experimental_constrained_fdiv: return VPIntrinsicDesc{ Intrinsic::vp_constrained_fdiv, TypeTokenVec{VPTypeToken::Vector}, 4, 5}; break;
  case Intrinsic::experimental_constrained_frem: return VPIntrinsicDesc{ Intrinsic::vp_constrained_frem, TypeTokenVec{VPTypeToken::Vector}, 4, 5}; break;
  case Intrinsic::experimental_constrained_pow: return VPIntrinsicDesc{ Intrinsic::vp_constrained_pow, TypeTokenVec{VPTypeToken::Vector}, 4, 5}; break;
  case Intrinsic::experimental_constrained_powi: return VPIntrinsicDesc{ Intrinsic::vp_constrained_powi, TypeTokenVec{VPTypeToken::Vector}, 4, 5}; break;
  case Intrinsic::experimental_constrained_maxnum: return VPIntrinsicDesc{ Intrinsic::vp_constrained_maxnum, TypeTokenVec{VPTypeToken::Vector}, 4, 5}; break;
  case Intrinsic::experimental_constrained_minnum: return VPIntrinsicDesc{ Intrinsic::vp_constrained_minnum, TypeTokenVec{VPTypeToken::Vector}, 4, 5}; break;

  case Intrinsic::experimental_constrained_fma: return VPIntrinsicDesc{ Intrinsic::vp_constrained_fma, TypeTokenVec{VPTypeToken::Vector}, 5, 6}; break;
  }
}

VPIntrinsic::VPIntrinsicDesc
VPIntrinsic::GetVPIntrinsicDesc(unsigned OC) {
  switch (OC) {
  default:
    return VPIntrinsicDesc{Intrinsic::not_intrinsic, TypeTokenVec(), -1, -1};

    // fp unary
    case Instruction::FNeg: return VPIntrinsicDesc{ Intrinsic::vp_fneg, TypeTokenVec{VPTypeToken::Vector}, 1, 2}; break;

    // fp binary
    case Instruction::FAdd: return VPIntrinsicDesc{ Intrinsic::vp_fadd, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::FSub: return VPIntrinsicDesc{ Intrinsic::vp_fsub, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::FMul: return VPIntrinsicDesc{ Intrinsic::vp_fmul, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::FDiv: return VPIntrinsicDesc{ Intrinsic::vp_fdiv, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::FRem: return VPIntrinsicDesc{ Intrinsic::vp_frem, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;

    // sign-oblivious int
    case Instruction::Add:  return VPIntrinsicDesc{ Intrinsic::vp_add, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::Sub:  return VPIntrinsicDesc{ Intrinsic::vp_sub, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::Mul:  return VPIntrinsicDesc{ Intrinsic::vp_mul, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;

    // signed/unsigned int
    case Instruction::SDiv: return VPIntrinsicDesc{ Intrinsic::vp_sdiv, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::UDiv: return VPIntrinsicDesc{ Intrinsic::vp_udiv, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::SRem: return VPIntrinsicDesc{ Intrinsic::vp_srem, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::URem: return VPIntrinsicDesc{ Intrinsic::vp_urem, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;

    // logical
    case Instruction::Or:   return VPIntrinsicDesc{ Intrinsic::vp_or,  TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::And:  return VPIntrinsicDesc{ Intrinsic::vp_and, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::Xor:  return VPIntrinsicDesc{ Intrinsic::vp_xor, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;

    case Instruction::LShr: return VPIntrinsicDesc{ Intrinsic::vp_lshr, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::AShr: return VPIntrinsicDesc{ Intrinsic::vp_ashr, TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;
    case Instruction::Shl:  return VPIntrinsicDesc{ Intrinsic::vp_shl,  TypeTokenVec{VPTypeToken::Vector}, 2, 3}; break;

    // comparison
    case Instruction::ICmp:
    case Instruction::FCmp:
      return VPIntrinsicDesc{ Intrinsic::vp_cmp, TypeTokenVec{VPTypeToken::Mask, VPTypeToken::Vector}, 2, 3}; break;
  }
}

VPIntrinsic::ShortTypeVec
VPIntrinsic::EncodeTypeTokens(VPIntrinsic::TypeTokenVec TTVec, Type & VectorTy, Type & ScalarTy) {
  ShortTypeVec STV;

  for (auto Token : TTVec) {
    switch (Token) {
    default:
      llvm_unreachable("unsupported token"); // unsupported VPTypeToken

    case VPIntrinsic::VPTypeToken::Scalar: STV.push_back(&ScalarTy); break;
    case VPIntrinsic::VPTypeToken::Vector: STV.push_back(&VectorTy); break;
    case VPIntrinsic::VPTypeToken::Mask:
      auto NumElems = VectorTy.getVectorNumElements();
      auto MaskTy = VectorType::get(Type::getInt1Ty(VectorTy.getContext()), NumElems);
      STV.push_back(MaskTy); break;
    }
  }

  return STV;
}


bool ConstrainedFPIntrinsic::isUnaryOp() const {
  switch (getIntrinsicID()) {
    default:
      return false;
    case Intrinsic::experimental_constrained_fptrunc:
    case Intrinsic::experimental_constrained_fpext:
    case Intrinsic::experimental_constrained_sqrt:
    case Intrinsic::experimental_constrained_sin:
    case Intrinsic::experimental_constrained_cos:
    case Intrinsic::experimental_constrained_exp:
    case Intrinsic::experimental_constrained_exp2:
    case Intrinsic::experimental_constrained_log:
    case Intrinsic::experimental_constrained_log10:
    case Intrinsic::experimental_constrained_log2:
    case Intrinsic::experimental_constrained_rint:
    case Intrinsic::experimental_constrained_nearbyint:
    case Intrinsic::experimental_constrained_ceil:
    case Intrinsic::experimental_constrained_floor:
    case Intrinsic::experimental_constrained_round:
    case Intrinsic::experimental_constrained_trunc:
      return true;
  }
}

bool ConstrainedFPIntrinsic::isTernaryOp() const {
  switch (getIntrinsicID()) {
    default:
      return false;
    case Intrinsic::experimental_constrained_fma:
      return true;
  }
}

Instruction::BinaryOps BinaryOpIntrinsic::getBinaryOp() const {
  switch (getIntrinsicID()) {
    case Intrinsic::uadd_with_overflow:
    case Intrinsic::sadd_with_overflow:
    case Intrinsic::uadd_sat:
    case Intrinsic::sadd_sat:
      return Instruction::Add;
    case Intrinsic::usub_with_overflow:
    case Intrinsic::ssub_with_overflow:
    case Intrinsic::usub_sat:
    case Intrinsic::ssub_sat:
      return Instruction::Sub;
    case Intrinsic::umul_with_overflow:
    case Intrinsic::smul_with_overflow:
      return Instruction::Mul;
    default:
      llvm_unreachable("Invalid intrinsic");
  }
}

bool BinaryOpIntrinsic::isSigned() const {
  switch (getIntrinsicID()) {
    case Intrinsic::sadd_with_overflow:
    case Intrinsic::ssub_with_overflow:
    case Intrinsic::smul_with_overflow:
    case Intrinsic::sadd_sat:
    case Intrinsic::ssub_sat:
      return true;
    default:
      return false;
  }
}

unsigned BinaryOpIntrinsic::getNoWrapKind() const {
  if (isSigned())
    return OverflowingBinaryOperator::NoSignedWrap;
  else
    return OverflowingBinaryOperator::NoUnsignedWrap;
}
