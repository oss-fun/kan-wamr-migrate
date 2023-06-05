/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */

#ifndef _WASM_MEMORY_H
#define _WASM_MEMORY_H

#include "bh_common.h"
#include "../include/wasm_export.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum Data_Type {
    charT,
    charTT,
    uint8T,
    uint16T,
    uint32T,
    uint64T,

    gc_heap_tT,
    base_addrT,
    WASIContextT,
    WASMThreadArgT,
    ExternRefMapNodeT,

    fd_tableT,
    fd_prestatsT,
    argv_environ_valuesT,

    uvwasi_tT,
    uvwasi_preopen_tT,

    //wasm_export.h
    wasm_val_tT,

    //wasm_c_api_internal.h
    wasm_instance_tT,
    wasm_engine_tT,
    wasm_store_tT,
    VectorT,
    wasm_valtype_tT,
    wasm_functype_tT,
    wasm_valtype_vec_tT,
    wasm_globaltype_tT,
    wasm_tabletype_tT,
    wasm_memorytype_tT,
    wasm_importtype_tT,
    wasm_byte_vec_tT,
    wasm_exporttype_tT,
    wasm_ref_tT,
    wasm_frame_tT,
    wasm_trap_tT,
    wasm_foreign_tT,
    wasm_module_ex_tT,
    wasm_func_tT,
    wasm_global_tT,
    wasm_table_tT,
    wasm_memory_tT,
    wasm_store_vec_tT,
    wasm_module_vec_tT,
    wasm_instance_vec_tT,
    wasm_extern_vec_tT,

    //wasm_exec_env.h
    WASMExecEnvT,

    //wasm_native.c
    NativeSymbolsNodeT,

    //wasm_runtime_common.h
    WASMModuleCommonT,
    WASMModuleInstanceCommonT,
    WASMModuleMemConsumptionT,
    WASMModuleInstMemConsumptionT,
    WASMMemoryInstanceCommonT,
    WASMSectionT,
    WASMCApiFrameT,
    WASMRegisteredModuleT,
    LoadingModuleT,

    //wasm_shared_memory.h
    WASMSharedMemNodeT,

    //wasm.h
    WASMModuleT,
    WASMFunctionT,
    WASMFunctionTT,
    WASMGlobalT,
    WASMExportT,
    V128T,
    WASMValueT,
    InitializerExpressionT,
    WASMTypeT,
    WASMTypeTT,
    WASMTableT,
    WASMMemoryT,
    WASMTableImportT,
    WASMMemoryImportT,
    WASMFunctionImportT,
    WASMGlobalImportT,
    WASMImportT,
    WASMTableSegT,
    WASMDataSegT,
    WASMDataSegTT,
    BlockAddrT,
    WASIArgumentsT,
    StringNodeT,
    BlockTypeT,
    WASMBranchBlockT,

    //wasm_interp.h
    WASMInterpFrameT,

    //wasm_loader.c
    BranchBlockT,
    BranchBlockPatchT,
    WASMLoaderContextT,
    ConstT,

    //wasm_runtime.h
    WASMModuleInstanceT,
    WASMFunctionInstanceT,
    WASMMemoryInstanceT,
    WASMMemoryInstanceTT,
    WASMTableInstanceT,
    WASMTableInstanceTT,
    WASMGlobalInstanceT,
    WASMExportFuncInstanceT,
    WASMExportGlobInstanceT,
    WASMSubModInstNodeT,
    WASMRuntimeFrameT, // == WASMInterpFrame

    //wasm_opcode.h
    WASMOpcodeT,
    WASMMiscEXTOpcodeT,
    WASMSimdEXTOpcodeT,
    WASMAtomicEXTOpcodeT,
    HashMapElemT,
    HashMapT,
    timer_ctx_tT,
    app_timer_tT,
    bh_queueT,
    bh_queue_nodeT,

    AtomicWaitInfoT,
    AtomicWaitNodeT,

    wasi_iovec_tT,
    wasi_ciovec_tT,
    WASI_FILE_T,
    ERRORT
} Data_Type;

typedef struct Pool_Info {
    int p_abs;
    void *p_raw;
    int size;
    Data_Type type;
    struct Pool_Info *list;
    struct Pool_Info *next;
} Pool_Info;

void *
get_base_addr(void);

Pool_Info *
get_raw(void *addr);

Pool_Info *
get_info(int p_abs);

Pool_Info *
get_frame_info(int p_abs);

Pool_Info *
get_root_frame_info(void);

void
abs_translation(void);

void
restore_runtime(void);
void
dump_runtime(void);
void
alloc_info(void *addr, Data_Type type);
void
alloc_info_buf(void *addr, Data_Type type, size_t buf_size);
void
alloc_info_ex(void *addr, Data_Type type, size_t size);
void
alloc_infos(void *addr, Data_Type type, size_t size);
void
free_info(void *addr);

bool
wasm_runtime_memory_init(mem_alloc_type_t mem_alloc_type,
                         const MemAllocOption *alloc_option);

void
wasm_runtime_memory_destroy();

#ifdef __cplusplus
}
#endif

#endif /* end of _WASM_MEMORY_H */

