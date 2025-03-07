/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _WASM_INTERP_H
#define _WASM_INTERP_H

#include "wasm.h"
#include "wasm_runtime.h"
#include "../common/wasm_exec_env.h"

#ifdef __cplusplus
extern "C" {
#endif

struct WASMModuleInstance;
struct WASMFunctionInstance;
struct WASMExecEnv;

typedef struct WASMInterpFrame {
  /* The frame of the caller that are calling the current function. */
  struct WASMInterpFrame *prev_frame;

  /* The current WASM function. */
  struct WASMFunctionInstance *function;

  /* Instruction pointer of the bytecode array.  */
  uint8 *ip;

#if WASM_ENABLE_PERF_PROFILING != 0
  uint64 time_started;
#endif

#if WASM_ENABLE_FAST_INTERP != 0
  /* return offset of the first return value of current frame.
    the callee will put return values here continuously */
  uint32 ret_offset;
  uint32 *lp;
  uint32 operand[1];
#else
  /* Operand stack top pointer of the current frame.  The bottom of
     the stack is the next cell after the last local variable.  */
  uint32 *sp_bottom;
  uint32 *sp_boundary;
  uint32 *sp;

  WASMBranchBlock *csp_bottom;
  WASMBranchBlock *csp_boundary;
  WASMBranchBlock *csp;

  /* Frame data, the layout is:
     lp: param_cell_count + local_cell_count
     sp_bottom to sp_boundary: stack of data
     csp_bottom to csp_boundary: stack of block
     ref to frame end: data types of local vairables and stack data
     */
  uint32 lp[1];
#endif
} WASMInterpFrame;

/**
 * Calculate the size of interpreter area of frame of a function.
 *
 * @param all_cell_num number of all cells including local variables
 * and the working stack slots
 *
 * @return the size of interpreter area of the frame
 */
static inline unsigned
wasm_interp_interp_frame_size(unsigned all_cell_num)
{
  return align_uint((uint32)offsetof(WASMInterpFrame, lp)
                    + all_cell_num * 5, 4);
}

void
wasm_interp_call_wasm(struct WASMModuleInstance *module_inst,
                      struct WASMExecEnv *exec_env,
                      struct WASMFunctionInstance *function,
                      uint32 argc, uint32 argv[]);

bool
wasm_interp_restore(uint32 argc, uint32 argv[]);

WASMInterpFrame *
wasm_interp_alloc_frame(WASMExecEnv *exec_env,
                        uint32 size,
                        WASMInterpFrame *prev_frame);

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_INTERP_H */
