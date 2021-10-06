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
    BlockAddrT,
    WASIArgumentsT,
    StringNodeT,
    BlockTypeT,
    WASMBranchBlockT,

    //wasm_interp.h
    WASMInterpFrameT,

    //wasm_loader.c
    BranchBlockT,
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
    WASMRuntimeFrameT, // == WASMInterpFrame

    //wasm_opcode.h
    WASMOpcodeT,
    WASMMiscEXTOpcodeT,
    WASMSimdEXTOpcodeT,
    WASMAtomicEXTOpcodeT,
} Data_Type;

typedef struct Pool_Info {
    unsigned int p_abs;
    void *p_raw;
    Data_Type type;
} Pool_Info;
void
alloc_info(void *addr, Data_Type type);
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
