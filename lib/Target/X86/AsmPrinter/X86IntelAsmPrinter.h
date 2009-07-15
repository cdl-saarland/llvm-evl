//===-- X86IntelAsmPrinter.h - Convert X86 LLVM code to Intel assembly ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Intel assembly code printer class.
//
//===----------------------------------------------------------------------===//

#ifndef X86INTELASMPRINTER_H
#define X86INTELASMPRINTER_H

#include "../X86.h"
#include "../X86MachineFunctionInfo.h"
#include "../X86TargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/FormattedStream.h"

namespace llvm {

struct VISIBILITY_HIDDEN X86IntelAsmPrinter : public AsmPrinter {
  explicit X86IntelAsmPrinter(formatted_raw_ostream &O, X86TargetMachine &TM,
                              const TargetAsmInfo *T, bool V)
    : AsmPrinter(O, TM, T, V) {}

  virtual const char *getPassName() const {
    return "X86 Intel-Style Assembly Printer";
  }

  /// printInstruction - This method is automatically generated by tablegen
  /// from the instruction set description.  This method returns true if the
  /// machine instruction was sufficiently described to print it, otherwise it
  /// returns false.
  bool printInstruction(const MachineInstr *MI);

  // This method is used by the tablegen'erated instruction printer.
  void printOperand(const MachineInstr *MI, unsigned OpNo,
                    const char *Modifier = 0) {
    const MachineOperand &MO = MI->getOperand(OpNo);
    if (MO.isReg()) {
      assert(TargetRegisterInfo::isPhysicalRegister(MO.getReg()) &&
             "Not physreg??");
      O << TM.getRegisterInfo()->get(MO.getReg()).Name;  // Capitalized names
    } else {
      printOp(MO, Modifier);
    }
  }
  
  void print_pcrel_imm(const MachineInstr *MI, unsigned OpNo);


  void printi8mem(const MachineInstr *MI, unsigned OpNo) {
    O << "BYTE PTR ";
    printMemReference(MI, OpNo);
  }
  void printi16mem(const MachineInstr *MI, unsigned OpNo) {
    O << "WORD PTR ";
    printMemReference(MI, OpNo);
  }
  void printi32mem(const MachineInstr *MI, unsigned OpNo) {
    O << "DWORD PTR ";
    printMemReference(MI, OpNo);
  }
  void printi64mem(const MachineInstr *MI, unsigned OpNo) {
    O << "QWORD PTR ";
    printMemReference(MI, OpNo);
  }
  void printi128mem(const MachineInstr *MI, unsigned OpNo) {
    O << "XMMWORD PTR ";
    printMemReference(MI, OpNo);
  }
  void printi256mem(const MachineInstr *MI, unsigned OpNo) {
    O << "YMMWORD PTR ";
    printMemReference(MI, OpNo);
  }
  void printf32mem(const MachineInstr *MI, unsigned OpNo) {
    O << "DWORD PTR ";
    printMemReference(MI, OpNo);
  }
  void printf64mem(const MachineInstr *MI, unsigned OpNo) {
    O << "QWORD PTR ";
    printMemReference(MI, OpNo);
  }
  void printf80mem(const MachineInstr *MI, unsigned OpNo) {
    O << "XWORD PTR ";
    printMemReference(MI, OpNo);
  }
  void printf128mem(const MachineInstr *MI, unsigned OpNo) {
    O << "XMMWORD PTR ";
    printMemReference(MI, OpNo);
  }
  void printf256mem(const MachineInstr *MI, unsigned OpNo) {
    O << "YMMWORD PTR ";
    printMemReference(MI, OpNo);
  }
  void printlea32mem(const MachineInstr *MI, unsigned OpNo) {
    O << "DWORD PTR ";
    printLeaMemReference(MI, OpNo);
  }
  void printlea64mem(const MachineInstr *MI, unsigned OpNo) {
    O << "QWORD PTR ";
    printLeaMemReference(MI, OpNo);
  }
  void printlea64_32mem(const MachineInstr *MI, unsigned OpNo) {
    O << "QWORD PTR ";
    printLeaMemReference(MI, OpNo, "subreg64");
  }

  bool printAsmMRegister(const MachineOperand &MO, const char Mode);
  bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                       unsigned AsmVariant, const char *ExtraCode);
  bool PrintAsmMemoryOperand(const MachineInstr *MI, unsigned OpNo,
                             unsigned AsmVariant, const char *ExtraCode);
  void printMachineInstruction(const MachineInstr *MI);
  void printOp(const MachineOperand &MO, const char *Modifier = 0);
  void printSSECC(const MachineInstr *MI, unsigned Op);
  void printMemReference(const MachineInstr *MI, unsigned Op,
                         const char *Modifier=NULL);
  void printLeaMemReference(const MachineInstr *MI, unsigned Op,
                            const char *Modifier=NULL);
  void printPICJumpTableSetLabel(unsigned uid,
                                 const MachineBasicBlock *MBB) const;
  void printPICJumpTableSetLabel(unsigned uid, unsigned uid2,
                                 const MachineBasicBlock *MBB) const {
    AsmPrinter::printPICJumpTableSetLabel(uid, uid2, MBB);
  }
  void printPICLabel(const MachineInstr *MI, unsigned Op);
  bool runOnMachineFunction(MachineFunction &F);
  bool doInitialization(Module &M);
  bool doFinalization(Module &M);

  // We have to propagate some information about MachineFunction to
  // AsmPrinter. It's ok, when we're printing the function, since we have
  // access to MachineFunction and can get the appropriate MachineFunctionInfo.
  // Unfortunately, this is not possible when we're printing reference to
  // Function (e.g. calling it and so on). Even more, there is no way to get the
  // corresponding MachineFunctions: it can even be not created at all. That's
  // why we should use additional structure, when we're collecting all necessary
  // information.
  //
  // This structure is using e.g. for name decoration for stdcall & fastcall'ed
  // function, since we have to use arguments' size for decoration.
  typedef std::map<const Function*, X86MachineFunctionInfo> FMFInfoMap;
  FMFInfoMap FunctionInfoMap;

  void decorateName(std::string& Name, const GlobalValue* GV);

  virtual void EmitString(const ConstantArray *CVA) const;

  // Necessary for dllexport support
  StringSet<> DLLExportedFns, DLLExportedGVs;
};

} // end namespace llvm

#endif
