#include <llvm/IR/VPBuilder.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/PredicatedInst.h>
#include <llvm/ADT/SmallVector.h>

namespace {
  using namespace llvm;
  using ShortTypeVec = VPIntrinsic::ShortTypeVec;
  using ShortValueVec = SmallVector<Value*, 4>;
}

namespace llvm {

Module &
VPBuilder::getModule() const {
  return *Builder.GetInsertBlock()->getParent()->getParent();
}

Value&
VPBuilder::GetMaskForType(VectorType & VecTy) {
  if (Mask) return *Mask;

  auto * boolTy = Builder.getInt1Ty();
  auto * maskTy = VectorType::get(boolTy, StaticVectorLength);
  return *ConstantInt::getAllOnesValue(maskTy);
}

Value&
VPBuilder::GetEVLForType(VectorType & VecTy) {
  if (ExplicitVectorLength) return *ExplicitVectorLength;

  auto * intTy = Builder.getInt32Ty();
  return *ConstantInt::get(intTy, StaticVectorLength);
}

Value*
VPBuilder::CreateVectorCopy(Instruction & Inst, ValArray VecOpArray) {

  auto oc = Inst.getOpcode();

  auto evlDesc = VPIntrinsic::GetVPIntrinsicDesc(oc);
  if (evlDesc.ID == Intrinsic::not_intrinsic) {
    return nullptr;
  }

  if ((oc <= Instruction::BinaryOpsEnd) &&
      (oc >= Instruction::BinaryOpsBegin)) {

    assert(VecOpArray.size() == 2);
    Value & FirstOp = *VecOpArray[0];
    Value & SndOp = *VecOpArray[1];

    // Fetch the VP intrinsic
    auto & VecTy = cast<VectorType>(*FirstOp.getType());
    assert((evlDesc.MaskPos == 2) && (evlDesc.EVLPos == 3));

    auto & VPCall =
      cast<Instruction>(*PredicatedBinaryOperator::Create(&getModule(), &GetMaskForType(VecTy), &GetEVLForType(VecTy), static_cast<Instruction::BinaryOps>(oc), &FirstOp, &SndOp));
    Builder.Insert(&VPCall);

    // transfer fast math flags
    if (isa<FPMathOperator>(Inst)) {
      VPCall.copyFastMathFlags(Inst.getFastMathFlags());
    }

    return &VPCall;
  }

  if ((oc <= Instruction::UnaryOpsBegin) &&
      (oc >= Instruction::UnaryOpsEnd)) {
    assert(VecOpArray.size() == 1);
    Value & FirstOp = *VecOpArray[0];

    // Fetch the VP intrinsic
    auto & VecTy = cast<VectorType>(*FirstOp.getType());
    auto & ScalarTy = *VecTy.getVectorElementType();
    auto * Func = Intrinsic::getDeclaration(&getModule(), evlDesc.ID, VPIntrinsic::EncodeTypeTokens(evlDesc.typeTokens, VecTy, ScalarTy));

    assert((evlDesc.MaskPos == 1) && (evlDesc.EVLPos == 2));

    // Materialize the Call
    ShortValueVec Args{&FirstOp, &GetMaskForType(VecTy), &GetEVLForType(VecTy)};

    auto & VPCall = *Builder.CreateCall(Func, Args);

    // transfer fast math flags
    if (isa<FPMathOperator>(Inst)) {
      cast<CallInst>(VPCall).copyFastMathFlags(Inst.getFastMathFlags());
    }

    return &VPCall;
  }

  switch (oc) {
    default:
      return nullptr;

    case Instruction::FCmp:
    case Instruction::ICmp: {
      assert(VecOpArray.size() == 2);
      Value & FirstOp = *VecOpArray[0];
      Value & SndOp = *VecOpArray[1];

      // Fetch the VP intrinsic
      auto & VecTy = cast<VectorType>(*FirstOp.getType());
      auto & ScalarTy = *VecTy.getVectorElementType();
      auto * Func = Intrinsic::getDeclaration(&getModule(), evlDesc.ID, VPIntrinsic::EncodeTypeTokens(evlDesc.typeTokens, VecTy, ScalarTy));

      assert((evlDesc.MaskPos == 2) && (evlDesc.EVLPos == 3));

      // encode comparison predicate as MD
      uint8_t RawPred = cast<CmpInst>(Inst).getPredicate();
      auto Int8Ty = Builder.getInt8Ty();
      auto PredArg = ConstantInt::get(Int8Ty, RawPred, false);

      // Materialize the Call
      ShortValueVec Args{&FirstOp, &SndOp, &GetMaskForType(VecTy), &GetEVLForType(VecTy), PredArg};

      return Builder.CreateCall(Func, Args);
    }

     case Instruction::Select: {
      assert(VecOpArray.size() == 2);
      Value & MaskOp = *VecOpArray[0];
      Value & OnTrueOp = *VecOpArray[1];
      Value & OnFalseOp = *VecOpArray[2];

      // Fetch the VP intrinsic
      auto & VecTy = cast<VectorType>(*OnTrueOp.getType());
      auto & ScalarTy = *VecTy.getVectorElementType();

      auto * Func = Intrinsic::getDeclaration(&getModule(), evlDesc.ID, VPIntrinsic::EncodeTypeTokens(evlDesc.typeTokens, VecTy, ScalarTy));

      assert((evlDesc.MaskPos == 2) && (evlDesc.EVLPos == 3));

      // Materialize the Call
      ShortValueVec Args{&OnTrueOp, &OnFalseOp, &MaskOp, &GetEVLForType(VecTy)};

      return Builder.CreateCall(Func, Args);
    }
  }
}

VectorType&
VPBuilder::getVectorType(Type &ElementTy) {
  return *VectorType::get(&ElementTy, StaticVectorLength);
}

Value&
VPBuilder::CreateContiguousStore(Value & Val, Value & Pointer, unsigned Alignment) {
  auto & VecTy = cast<VectorType>(*Val.getType());
  auto * StoreFunc = Intrinsic::getDeclaration(&getModule(), Intrinsic::vp_store, {Val.getType(), Pointer.getType()});
  ShortValueVec Args{&Val, &Pointer, &GetMaskForType(VecTy), &GetEVLForType(VecTy)};
  CallInst &StoreCall = *Builder.CreateCall(StoreFunc, Args);
  if (Alignment) StoreCall.addParamAttr(1, Attribute::getWithAlignment(getContext(), Alignment));
  return StoreCall;
}

Value&
VPBuilder::CreateContiguousLoad(Value & Pointer, unsigned Alignment) {
  auto & PointerTy = cast<PointerType>(*Pointer.getType());
  auto & VecTy = getVectorType(*PointerTy.getPointerElementType());

  auto * LoadFunc = Intrinsic::getDeclaration(&getModule(), Intrinsic::vp_load, {&VecTy, &PointerTy});
  ShortValueVec Args{&Pointer, &GetMaskForType(VecTy), &GetEVLForType(VecTy)};
  CallInst &LoadCall= *Builder.CreateCall(LoadFunc, Args);
  if (Alignment) LoadCall.addParamAttr(1, Attribute::getWithAlignment(getContext(), Alignment));
  return LoadCall;
}

Value&
VPBuilder::CreateScatter(Value & Val, Value & PointerVec, unsigned Alignment) {
  auto & VecTy = cast<VectorType>(*Val.getType());
  auto * ScatterFunc = Intrinsic::getDeclaration(&getModule(), Intrinsic::vp_scatter, {Val.getType(), PointerVec.getType()});
  ShortValueVec Args{&Val, &PointerVec, &GetMaskForType(VecTy), &GetEVLForType(VecTy)};
  CallInst &ScatterCall = *Builder.CreateCall(ScatterFunc, Args);
  if (Alignment) ScatterCall.addParamAttr(1, Attribute::getWithAlignment(getContext(), Alignment));
  return ScatterCall;
}

Value&
VPBuilder::CreateGather(Value & PointerVec, unsigned Alignment) {
  auto & PointerVecTy = cast<VectorType>(*PointerVec.getType());
  auto & ElemTy = *cast<PointerType>(*PointerVecTy.getVectorElementType()).getPointerElementType();
  auto & VecTy = *VectorType::get(&ElemTy, PointerVecTy.getNumElements());
  auto * GatherFunc = Intrinsic::getDeclaration(&getModule(), Intrinsic::vp_gather, {&VecTy, &PointerVecTy});

  ShortValueVec Args{&PointerVec, &GetMaskForType(VecTy), &GetEVLForType(VecTy)};
  CallInst &GatherCall = *Builder.CreateCall(GatherFunc, Args);
  if (Alignment) GatherCall.addParamAttr(1, Attribute::getWithAlignment(getContext(), Alignment));
  return GatherCall;
}

} // namespace llvm
