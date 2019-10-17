==========================
Vector Predication Roadmap
==========================

.. contents:: Table of Contents
  :depth: 3
  :local:

Motivation
==========

This proposal defines a roadmap towards native vector predication in LLVM, specifically for vector instructions with a mask and/or an explicit vector length.
LLVM currently has no target-independent means to model predicated vector instructions for modern SIMD ISAs such as AVX512, ARM SVE, the RISC-V V extension and NEC SX-Aurora.
Only some predicated vector operations, such as masked loads and stores are available through intrinsics [MaskedIR]_.

The Explicit Vector Length extension
====================================

The Explicit Vector Length (EVL) extension [EvlRFC]_ can be a first step towards native vector predication.
The EVL prototype in this patch demonstrates the following concepts:

- Predicated vector intrinsics with an explicit mask and vector length parameter on IR level.
- First-class predicated SDNodes on ISel level. Mask and vector length are value operands.
- An incremental strategy to generalize PatternMatch/InstCombine/InstSimplify and DAGCombiner to work on both regular instructions and EVL intrinsics.
- DAGCombiner example: FMA fusion.
- InstCombine/InstSimplify example: FSub pattern re-writes.
- Early experiments on the LNT test suite (Clang static release, O3 -ffast-math) indicate that compile time on non-EVL IR is not affected by the API abstractions in PatternMatch, etc.

Roadmap
=======

Drawing from the EVL prototype, we propose the following roadmap towards native vector predication in LLVM:


1. IR-level EVL intrinsics 
-----------------------------------------

- There is a consensus on the semantics/instruction set of EVL.
- EVL intrinsics and attributes are available on IR level.
- TTI has capability flags for EVL (``supportsEVL()``?, ``haveActiveVectorLength()``?).

Result: EVL usable for IR-level vectorizers (LV, VPlan, RegionVectorizer), potential integration in Clang with builtins.

2. CodeGen support
------------------

- EVL intrinsics translate to first-class SDNodes (``llvm.evl.fdiv.* -> evl_fdiv``). 
- EVL legalization (legalize explicit vector length to mask (AVX512), legalize EVL SDNodes to pre-existing ones (SSE, NEON)).

Result: Backend development based on EVL SDNodes.

3. Lift InstSimplify/InstCombine/DAGCombiner to EVL
------------------------------------------------

- Introduce PredicatedInstruction, PredicatedBinaryOperator, .. helper classes that match standard vector IR and EVL intrinsics.
- Add a matcher context to PatternMatch and context-aware IR Builder APIs.
- Incrementally lift DAGCombiner to work on EVL SDNodes as well as on regular vector instructions.
- Incrementally lift InstCombine/InstSimplify to operate on EVL as well as regular IR instructions.

Result: Optimization of EVL intrinsics on par with standard vector instructions.

4. Deprecate llvm.masked.* / llvm.experimental.reduce.*
-------------------------------------------------------

- Modernize llvm.masked.* / llvm.experimental.reduce* by translating to EVL.
- DCE transitional APIs.

Result: EVL has superseded earlier vector intrinsics.

5. Predicated IR Instructions
---------------------------------------

- Vector instructions have an optional mask and vector length parameter. These lower to EVL SDNodes (from Stage 2).
- Phase out EVL intrinsics, only keeping those that are not equivalent to vectorized scalar instructions (reduce,  shuffles, ..)
- InstCombine/InstSimplify expect predication in regular Instructions (Stage (3) has laid the groundwork). 

Result: Native vector predication in IR.

References
==========

.. [MaskedIR] `llvm.masked.*` intrinsics, https://llvm.org/docs/LangRef.html#masked-vector-load-and-store-intrinsics
.. [EvlRFC] Explicit Vector Length RFC, https://reviews.llvm.org/D53613
