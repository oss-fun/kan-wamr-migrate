/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */
#ifndef _WASM_LOADER_H
#define _WASM_LOADER_H

#include "wasm.h"
#include "bh_hashmap.h"
#include "../common/wasm_runtime_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#if WASM_ENABLE_FAST_INTERP != 0
typedef struct BranchBlockPatch {
    struct BranchBlockPatch *next;
    uint8 patch_type;
    uint8 *code_compiled;
} BranchBlockPatch;
#endif

typedef struct BranchBlock {
    uint8 label_type;
    BlockType block_type;
    uint8 *start_addr;
    uint8 *else_addr;
    uint8 *end_addr;
    uint32 stack_cell_num;
#if WASM_ENABLE_FAST_INTERP != 0
    uint16 dynamic_offset;
    uint8 *code_compiled;
    BranchBlockPatch *patch_list;
    /* This is used to save params frame_offset of of if block */
    int16 *param_frame_offsets;
#endif

    /* Indicate the operand stack is in polymorphic state.
     * If the opcode is one of unreachable/br/br_table/return, stack is marked
     * to polymorphic state until the block's 'end' opcode is processed.
     * If stack is in polymorphic state and stack is empty, instruction can
     * pop any type of value directly without decreasing stack top pointer
     * and stack cell num. */
    bool is_stack_polymorphic;
} BranchBlock;

typedef struct WASMLoaderContext {
    /* frame ref stack */
    uint8 *frame_ref;
    uint8 *frame_ref_bottom;
    uint8 *frame_ref_boundary;
    uint32 frame_ref_size;
    uint32 stack_cell_num;
    uint32 max_stack_cell_num;

    /* frame csp stack */
    BranchBlock *frame_csp;
    BranchBlock *frame_csp_bottom;
    BranchBlock *frame_csp_boundary;
    uint32 frame_csp_size;
    uint32 csp_num;
    uint32 max_csp_num;

#if WASM_ENABLE_FAST_INTERP != 0
    /* frame offset stack */
    int16 *frame_offset;
    int16 *frame_offset_bottom;
    int16 *frame_offset_boundary;
    uint32 frame_offset_size;
    int16 dynamic_offset;
    int16 start_dynamic_offset;
    int16 max_dynamic_offset;

    /* preserved local offset */
    int16 preserved_local_offset;

    /* const buffer */
    uint8 *const_buf;
    uint16 num_const;
    uint16 const_buf_size;
    uint16 const_cell_num;

    /* processed code */
    uint8 *p_code_compiled;
    uint8 *p_code_compiled_end;
    uint32 code_compiled_size;
#endif
} WASMLoaderContext;

typedef struct Const {
    WASMValue value;
    uint16 slot_index;
    uint8 value_type;
} Const;


/**
 * Load a WASM module from a specified byte buffer.
 *
 * @param buf the byte buffer which contains the WASM binary data
 * @param size the size of the buffer
 * @param error_buf output of the exception info
 * @param error_buf_size the size of the exception string
 *
 * @return return module loaded, NULL if failed
 */
WASMModule*
wasm_loader_load(const uint8 *buf, uint32 size, char *error_buf, uint32 error_buf_size);

/**
 * Load a WASM module from a specified WASM section list.
 *
 * @param section_list the section list which contains each section data
 * @param error_buf output of the exception info
 * @param error_buf_size the size of the exception string
 *
 * @return return WASM module loaded, NULL if failed
 */
WASMModule*
wasm_loader_load_from_sections(WASMSection *section_list,
                               char *error_buf, uint32 error_buf_size);

/**
 * Unload a WASM module.
 *
 * @param module the module to be unloaded
 */
void
wasm_loader_unload(WASMModule *module);

/**
 * Find address of related else opcode and end opcode of opcode block/loop/if
 * according to the start address of opcode.
 *
 * @param module the module to find
 * @param start_addr the next address of opcode block/loop/if
 * @param code_end_addr the end address of function code block
 * @param block_type the type of block, 0/1/2 denotes block/loop/if
 * @param p_else_addr returns the else addr if found
 * @param p_end_addr returns the end addr if found
 * @param error_buf returns the error log for this function
 * @param error_buf_size returns the error log string length
 *
 * @return true if success, false otherwise
 */
bool
wasm_loader_find_block_addr(BlockAddr *block_addr_cache,
                            const uint8 *start_addr,
                            const uint8 *code_end_addr,
                            uint8 block_type,
                            uint8 **p_else_addr,
                            uint8 **p_end_addr);

#if WASM_ENABLE_REF_TYPES != 0
void
wasm_set_ref_types_flag(bool enable);

bool
wasm_get_ref_types_flag();
#endif

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_LOADER_H */
