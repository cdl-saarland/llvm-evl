//===-- llvm/PredicatedInst.h - Predication utility subclass --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines various classes for working with Predicated Instructions.
// Predicated instructions are either regular instructions or calls to
// Explicit Vector Length (EVL) intrinsics that have a mask and an explicit
// vector length argument.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_IR_PREDICATEDINST_H
#define LLVM_IR_PREDICATEDINST_H

#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Operator.h"
#include <cstddef>

namespace llvm {

class PredicatedInstruction : public User {
public:
  // The PredicatedInstruction class is intended to be used as a utility, and is never itself
  // instantiated.
  PredicatedInstruction() = delete;
  ~PredicatedInstruction() = delete;

  void *operator new(size_t s) = delete;

  Value* getMask() const {
    auto thisEVL = dyn_cast<EVLIntrinsic>(this);
    if (!thisEVL) return nullptr;
    return thisEVL->getMask();
  }

  Value* getVectorLength() const {
    auto thisEVL = dyn_cast<EVLIntrinsic>(this);
    if (!thisEVL) return nullptr;
    return thisEVL->getVectorLength();
  }

  unsigned getOpcode() const {
    auto * EVLInst = dyn_cast<EVLIntrinsic>(this);
    if (EVLInst)
      return EVLInst->getFunctionalOpcode();
    return cast<Instruction>(this)->getOpcode();
  }

  static bool classof(const Instruction * I) { return isa<Instruction>(I); }
  static bool classof(const ConstantExpr * CE) { return false; }
  static bool classof(const Value *V) { return isa<Instruction>(V); }
};

class PredicatedOperator : public User {
public:
  // The PredicatedOperator class is intended to be used as a utility, and is never itself
  // instantiated.
  PredicatedOperator() = delete;
  ~PredicatedOperator() = delete;

  void *operator new(size_t s) = delete;

  /// Return the opcode for this Instruction or ConstantExpr.
  unsigned getOpcode() const {
    auto * EVLInst = dyn_cast<EVLIntrinsic>(this);
    if (EVLInst)
      return EVLInst->getFunctionalOpcode();
    if (const Instruction *I = dyn_cast<Instruction>(this))
      return I->getOpcode();
    return cast<ConstantExpr>(this)->getOpcode();
  }

  Value* getMask() const {
    auto thisEVL = dyn_cast<EVLIntrinsic>(this);
    if (!thisEVL) return nullptr;
    return thisEVL->getMask();
  }

  Value* getVectorLength() const {
    auto thisEVL = dyn_cast<EVLIntrinsic>(this);
    if (!thisEVL) return nullptr;
    return thisEVL->getVectorLength();
  }

  static bool classof(const Instruction * I) { return isa<EVLIntrinsic>(I) || isa<Operator>(I); }
  static bool classof(const ConstantExpr * CE) { return isa<Operator>(CE); }
  static bool classof(const Value *V) { return isa<EVLIntrinsic>(V) || isa<Operator>(V); }
};

class PredicatedBinaryOperator : public PredicatedOperator {
public:
  // The PredicatedBinaryOperator class is intended to be used as a utility, and is never itself
  // instantiated.
  PredicatedBinaryOperator() = delete;
  ~PredicatedBinaryOperator() = delete;

  void *operator new(size_t s) = delete;

  static bool classof(const Instruction * I) {
    if (isa<BinaryOperator>(I)) return true;
    auto EVLInst = dyn_cast<EVLIntrinsic>(I);
    bool ok = EVLInst && EVLInst->isBinaryOp();
    if (getenv("SDEBUG")) {
      errs() << "Matched PredBinOp? " << ok << " for " << *EVLInst << "\n";
    }
    return ok;
  }
  static bool classof(const ConstantExpr * CE) { return isa<BinaryOperator>(CE); }
  static bool classof(const Value *V) {
    auto * I = dyn_cast<Instruction>(V);
    if (I && classof(I)) return true;
    auto * CE = dyn_cast<ConstantExpr>(V);
    return CE && classof(CE);
  }
};

class PredicatedICmpInst : public PredicatedBinaryOperator {
public:
  // The Operator class is intended to be used as a utility, and is never itself
  // instantiated.
  PredicatedICmpInst() = delete;
  ~PredicatedICmpInst() = delete;

  void *operator new(size_t s) = delete;

  static bool classof(const Instruction * I) {
    if (isa<ICmpInst>(I)) return true;
    auto EVLInst = dyn_cast<EVLIntrinsic>(I);
    return EVLInst && EVLInst->getFunctionalOpcode() == Instruction::ICmp;
  }
  static bool classof(const ConstantExpr * CE) { return CE->getOpcode() == Instruction::ICmp; }
  static bool classof(const Value *V) {
    auto * I = dyn_cast<Instruction>(V);
    if (I && classof(I)) return true;
    auto * CE = dyn_cast<ConstantExpr>(V);
    return CE && classof(CE);
  }

  ICmpInst::Predicate getPredicate() const {
    auto * ICInst = dyn_cast<const ICmpInst>(this);
    if (ICInst) return ICInst->getPredicate();
    auto * CE = dyn_cast<const ConstantExpr>(this);
    if (CE) return static_cast<ICmpInst::Predicate>(CE->getPredicate());
    return static_cast<ICmpInst::Predicate>(cast<EVLIntrinsic>(this)->getCmpPredicate());
  }
};


class PredicatedFCmpInst : public PredicatedBinaryOperator {
public:
  // The Operator class is intended to be used as a utility, and is never itself
  // instantiated.
  PredicatedFCmpInst() = delete;
  ~PredicatedFCmpInst() = delete;

  void *operator new(size_t s) = delete;

  static bool classof(const Instruction * I) {
    if (isa<FCmpInst>(I)) return true;
    auto EVLInst = dyn_cast<EVLIntrinsic>(I);
    return EVLInst && EVLInst->getFunctionalOpcode() == Instruction::FCmp;
  }
  static bool classof(const ConstantExpr * CE) { return CE->getOpcode() == Instruction::FCmp; }
  static bool classof(const Value *V) {
    auto * I = dyn_cast<Instruction>(V);
    if (I && classof(I)) return true;
    return isa<ConstantExpr>(V);
  }

  FCmpInst::Predicate getPredicate() const {
    auto * FCInst = dyn_cast<const FCmpInst>(this);
    if (FCInst) return FCInst->getPredicate();
    auto * CE = dyn_cast<const ConstantExpr>(this);
    if (CE) return static_cast<FCmpInst::Predicate>(CE->getPredicate());
    return static_cast<FCmpInst::Predicate>(cast<EVLIntrinsic>(this)->getCmpPredicate());
  }
};


class PredicatedSelectInst : public PredicatedOperator {
public:
  // The Operator class is intended to be used as a utility, and is never itself
  // instantiated.
  PredicatedSelectInst() = delete;
  ~PredicatedSelectInst() = delete;

  void *operator new(size_t s) = delete;

  static bool classof(const Instruction * I) {
    if (isa<SelectInst>(I)) return true;
    auto EVLInst = dyn_cast<EVLIntrinsic>(I);
    return EVLInst && EVLInst->getFunctionalOpcode() == Instruction::Select;
  }
  static bool classof(const ConstantExpr * CE) { return CE->getOpcode() == Instruction::Select; }
  static bool classof(const Value *V) {
    auto * I = dyn_cast<Instruction>(V);
    if (I && classof(I)) return true;
    auto * CE = dyn_cast<ConstantExpr>(V);
    return CE && CE->getOpcode() == Instruction::Select;
  }

  const Value *getCondition() const { return getOperand(0); }
  const Value *getTrueValue() const { return getOperand(1); }
  const Value *getFalseValue() const { return getOperand(2); }
  Value *getCondition() { return getOperand(0); }
  Value *getTrueValue() { return getOperand(1); }
  Value *getFalseValue() { return getOperand(2); }

  void setCondition(Value *V) { setOperand(0, V); }
  void setTrueValue(Value *V) { setOperand(1, V); }
  void setFalseValue(Value *V) { setOperand(2, V); }
};



} // namespace llvm

#endif // LLVM_IR_PREDICATEDINST_H
