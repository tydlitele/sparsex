/* -*- C++ -*-
 *
 * jit.h -- Just In Time compilation routines.
 *
 * Copyright (C) 2009-2011, Computing Systems Laboratory (CSLab), NTUA.
 * Copyright (C) 2009-2011, Kornilios Kourtis
 * Copyright (C) 2009-2011, Vasileios Karakasis
 * Copyright (C) 2010-2011, Theodors Goudouvas
 * All rights reserved.
 *
 * This file is distributed under the BSD License. See LICENSE.txt for details.
 */
#ifndef CSX_JIT_H__
#define CSX_JIT_H__

#include "csx.h"

#include "llvm/Type.h"
#include "llvm/Module.h"
#include "llvm/LLVMContext.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Function.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

#include "llvm_jit_help.h"


/**
 *  Implementation Notes
 *
 *  CsxJit generates spmv versions for given CSX matrices.
 *  Each CsxJit corresponds to a different thread, with a differnt CSX matrix.
 *
 *  CsxJit uses the LLVM library, to programmatically generate the spmv code.
 *  Because we don't want to generate all code from scratch, we use templates
 *  written in C and compiled into LLVM bytecode.
 *
 *  An spmv template is loaded at run-time and contains:
 *   - various helper functions
 *     (e.g., print_yxv(), fail(), align_ptr(), ...)
 *   - a template function called csx_spmv_template.
 *
 *  The code generated by:
 *   - create a function for each thread by cloning the template function
 *   - use annotations to mark specific values inside the template function
 *   - use special hooks (dummy functions), that are overwritten with
 *     with matrix-specific (generated) code.
 */

/**
 *  In the current implementation we use one LLVM Module for all CsxJits.
 *  This saves us some space, but doesn't allow us to generate code in parallel.
 *  The shared state is managed externally with the following functions:
 */
void CsxJitInitGlobal(void); ///> Initialize global stated -- called first.
void CsxJitOptmize(void);    ///> Optimize module -- called before JITting.

using namespace llvm;

namespace csx {

/**
 *  Code generator for a CSX matrix (one-per-thread)
 */
class CsxJit {

private:
    ///< Main state.
    CsxManager  *CsxMg;
    Module      *M;
    IRBuilder<> *Bld;

    ///< Helper functions that are loaded from the template.
    Function
      *UlGet,    ///< Extract a variable-length unsigned long from encoded data.
      *FailF,    ///< A function to indicate that something went wrong.
      *PrintYXV, ///< Print a triplet of Y, X, Value.
      *AlignF,   ///< Align a pointer to a specific boundary.
      *TestBitF, ///< Test if a bit is set.
      *SpmvF;    ///< A copied instance of the template's spmv function.

    ///< Commonly used Constants.
    Value *Zero8, *Zero32, *Zero64, *One8, *One32, *One64, *Three64;

    /**
     *  Values for handling the template function's local variables
     *  Note: We load and store into the variables stack addresses,
     *        We leave it to optimization to promote mem to regsiters.
     */
    Value
       *Xptr,      ///< X:     X array
       *Yptr,      ///< Y:     Y array
       *Vptr,      ///< V:     V array
       *YrPtr,     ///< Yr:    current partial value of y
       *CtlPtr,    ///< Ctl:   Ctl array (encoded index information)
       *MyxPtr,    ///< MyX:   current position in X (pointer)
       *YindxPtr,  ///< Yindx: Y index
       *SizePtr,   ///< Size:  size of current unit (taken from Ctl)
       *FlagsPtr;  ///< Flags: flags of current unit (taken from Ctl)


public:
    CsxJit(CsxManager *CsxMg, unsigned int id = 0);
    void GenCode(std::ostream &log);
    void *GetSpmvFn();

private:
    LLVMContext &GetLLVMCtx(void);

    void DoPrint(Value *Myx=NULL, Value *Yindx=NULL);
    void DoIncV();
    void DoMul(Value *Myx=NULL, Value *Yindx=NULL);
    void DoStoreYr();
    void DoOp(Value *MyX=NULL, Value *Yindx=NULL);

    void DoNewRowHook();
    void DoBodyHook(std::ostream &os);

    void DoDeltaAddMyx(int delta_bytes);

    void DeltaCase(BasicBlock *BB,          // case
                   BasicBlock *BB_lentry,   // loop entry
                   BasicBlock *BB_lbody,    // loop body
                   BasicBlock *BB_exit,     // final exit
                   int delta_bytes);

    void HorizCase(BasicBlock *BB,
                   BasicBlock *BB_lbody,
                   BasicBlock *BB_lexit,
                   BasicBlock *BB_exit,
                   int delta_size);

    void VertCase(BasicBlock *BB,
                  BasicBlock *BB_lbody,
                  BasicBlock *BB_exit,
                  int delta_size);

    void DiagCase(BasicBlock *BB,
                  BasicBlock *BB_lbody,
                  BasicBlock *BB_exit,
                  int delta_size,
                  bool reversed);

    void BlockRowCaseRolled(BasicBlock *BB,
                            BasicBlock *BB_lbody,
                            BasicBlock *BB_exit,
                            int r, int c);

    void BlockColCaseRolled(BasicBlock *BB,
                            BasicBlock *BB_lbody,
                            BasicBlock *BB_exit,
                            int r, int c);

    void BlockRowCaseUnrolled(BasicBlock *BB,
                              BasicBlock *BB_exit,
                              int r, int c);

    void BlockColCaseUnrolled(BasicBlock *BB,
                              BasicBlock *BB_exit,
                              int r, int c);

};

} // end csx namespace

#endif // CSX_JIT_H__

// vim:expandtab:tabstop=8:shiftwidth=4:softtabstop=4
