#pragma once


#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "mem_alloc.h"
#include "wasm_memory.h"
#include "wasm_c_api_internal.h"
#include "wasm_exec_env.h"
#include "wasm_native.h"
#include "wasm_shared_memory.h"
#include "wasm_dump.h"
#include "wasm_restore.h"
#include "../interpreter/wasm_opcode.h"
#include "../interpreter/wasm_runtime.h"
#include "../interpreter/wasm_loader.h"
#include "../interpreter/wasm_interp.h"
#include "../interpreter/wasm.h"
#include "wasm_c_api.h"
#include "wasm_runtime_common.h"
#include "../../shared/utils/bh_platform.h"
#include "../../shared/mem-alloc/ems/ems_gc_internal.h"
#if WASM_ENABLE_LIBC_WASI != 0
#include "../libraries/libc-wasi/libc_wasi_wrapper.h"
#endif

static size_t data_size[ERRORT];

void
set_data_size(void)
{
    data_size[charT] = sizeof(char);
    data_size[charTT] = sizeof(char*);
    data_size[uint8T] = sizeof(uint8);
    data_size[uint16T] = sizeof(uint16);
    data_size[uint32T] = sizeof(uint32);
    data_size[uint64T] = sizeof(uint64);
    data_size[gc_heap_tT] = sizeof(gc_heap_t);
    data_size[WASIContextT] = sizeof(WASIContext);
    //data_size[WASMThreadArgT] = sizeof(WASMThreadArg);
    //data_size[ExternRefMapNodeT] = sizeof(ExternRefMapNode);
    //data_size[fd_tableT] = sizeof(fd_table);
    //data_size[fd_prestatsT] = sizeof(fd_prestats);
    //data_size[argv_environ_valuesT] = sizeof(argv_environ_values);
    //data_size[uvwasi_tT] = sizeof(uvwasi_t);
    //data_size[uvwasi_preopen_tT] = sizeof(uvwasi_preopen_t);
    data_size[wasm_val_tT] = sizeof(wasm_val_t);
    data_size[wasm_instance_tT] = sizeof(wasm_instance_t);
    data_size[wasm_engine_tT] = sizeof(wasm_engine_t);
    data_size[wasm_store_tT] = sizeof(wasm_store_t);
    data_size[VectorT] = sizeof(Vector);
    data_size[wasm_valtype_tT] = sizeof(wasm_valtype_t);
    data_size[wasm_functype_tT] = sizeof(wasm_functype_t);
    data_size[wasm_valtype_vec_tT] = sizeof(wasm_valtype_vec_t);
    data_size[wasm_globaltype_tT] = sizeof(wasm_globaltype_t);
    data_size[wasm_tabletype_tT] = sizeof(wasm_tabletype_t);
    data_size[wasm_memorytype_tT] = sizeof(wasm_memorytype_t);
    data_size[wasm_importtype_tT] = sizeof(wasm_importtype_t);
    data_size[wasm_byte_vec_tT] = sizeof(wasm_byte_vec_t);
    data_size[wasm_exporttype_tT] = sizeof(wasm_exporttype_t);
    data_size[wasm_ref_tT] = sizeof(wasm_ref_t);
    data_size[wasm_frame_tT] = sizeof(wasm_frame_t);
    data_size[wasm_trap_tT] = sizeof(wasm_trap_t);
    data_size[wasm_foreign_tT] = sizeof(wasm_foreign_t);
    data_size[wasm_module_ex_tT] = sizeof(wasm_module_ex_t);
    data_size[wasm_func_tT] = sizeof(wasm_func_t);
    data_size[wasm_global_tT] = sizeof(wasm_global_t);
    data_size[wasm_table_tT] = sizeof(wasm_table_t);
    data_size[wasm_memory_tT] = sizeof(wasm_memory_t);
    data_size[wasm_store_vec_tT] = sizeof(wasm_store_vec_t);
    data_size[wasm_module_vec_tT] = sizeof(wasm_module_vec_t);
    data_size[wasm_instance_vec_tT] = sizeof(wasm_instance_vec_t);
    data_size[wasm_extern_vec_tT] = sizeof(wasm_extern_vec_t);
    data_size[WASMExecEnvT] = offsetof(WASMExecEnv, wasm_stack.s.bottom); // + stack_size
    data_size[NativeSymbolsNodeT] = sizeof(NativeSymbolsNode);
    data_size[WASMModuleCommonT] = sizeof(WASMModuleCommon);
    data_size[WASMModuleInstanceCommonT] = sizeof(WASMModuleInstanceCommon);
    data_size[WASMModuleMemConsumptionT] = sizeof(WASMModuleMemConsumption);
    data_size[WASMModuleInstMemConsumptionT] =
      sizeof(WASMModuleInstMemConsumption);
    data_size[WASMMemoryInstanceCommonT] = sizeof(WASMMemoryInstanceCommon);
    data_size[WASMSectionT] = sizeof(WASMSection);
    data_size[WASMCApiFrameT] = sizeof(WASMCApiFrame);
    //data_size[WASMRegisteredModuleT] = sizeof(WASMRegisteredModule);
    //data_size[LoadingModuleT] = sizeof(LoadingModule);
    data_size[WASMSharedMemNodeT] = sizeof(WASMSharedMemNode);
    data_size[WASMModuleT] = sizeof(WASMModule);
    data_size[WASMFunctionT] = sizeof(WASMFunction); // + (uint64)local_count
    data_size[WASMFunctionTT] = sizeof(WASMFunction*);
    data_size[WASMGlobalT] = sizeof(WASMGlobal);
    data_size[WASMExportT] = sizeof(WASMExport);
    data_size[V128T] = sizeof(V128);
    data_size[WASMValueT] = sizeof(WASMValue);
    data_size[InitializerExpressionT] = sizeof(InitializerExpression);
    data_size[WASMTypeT] = offsetof(WASMType, types); // + (uint64)(param_count + result_count)
    data_size[WASMTypeTT] = sizeof(WASMType*);
    data_size[WASMTableT] = sizeof(WASMTable);
    data_size[WASMMemoryT] = sizeof(WASMMemory);
    data_size[WASMTableImportT] = sizeof(WASMTableImport);
    data_size[WASMMemoryImportT] = sizeof(WASMMemoryImport);
    data_size[WASMFunctionImportT] = sizeof(WASMFunctionImport);
    data_size[WASMGlobalImportT] = sizeof(WASMGlobalImport);
    data_size[WASMImportT] = sizeof(WASMImport);
    data_size[WASMTableSegT] = sizeof(WASMTableSeg);
    data_size[WASMDataSegT] = sizeof(WASMDataSeg);
    data_size[WASMDataSegTT] = sizeof(WASMDataSeg*);
    data_size[BlockAddrT] = sizeof(BlockAddr);
    data_size[WASIArgumentsT] = sizeof(WASIArguments);
    data_size[StringNodeT] = sizeof(StringNode); // + len + 1
    data_size[BlockTypeT] = sizeof(BlockType);
    data_size[WASMBranchBlockT] = sizeof(WASMBranchBlock);
    data_size[WASMInterpFrameT] = offsetof(WASMInterpFrame,lp); // + all_cell_num*5 => align_uint
    data_size[BranchBlockT] = sizeof(BranchBlock);
    //data_size[BranchBlockPatchT] = sizeof(BranchBlockPatch);
    data_size[WASMLoaderContextT] = sizeof(WASMLoaderContext);
    data_size[ConstT] = sizeof(Const);
    data_size[WASMModuleInstanceT] = sizeof(WASMModuleInstance);
    data_size[WASMFunctionInstanceT] = sizeof(WASMFunctionInstance);
    data_size[WASMMemoryInstanceT] = sizeof(WASMMemoryInstance);
    data_size[WASMMemoryInstanceTT] = sizeof(WASMMemoryInstance*);
    data_size[WASMTableInstanceT] = offsetof(WASMTableInstance, base_addr); // + (uint64)max_size_fixed * sizeof(uint32) 
    data_size[WASMTableInstanceTT] = sizeof(WASMTableInstance*);
    data_size[WASMGlobalInstanceT] = sizeof(WASMGlobalInstance);
    data_size[WASMExportFuncInstanceT] = sizeof(WASMExportFuncInstance);
    //data_size[WASMExportGlobInstanceT] = sizeof(WASMExportGlobInstance);
    //data_size[WASMSubModInstNodeT] = sizeof(WASMSubModInstNode);
    data_size[WASMRuntimeFrameT] = sizeof(WASMRuntimeFrame);
    data_size[WASMOpcodeT] = sizeof(WASMOpcode);
    data_size[WASMMiscEXTOpcodeT] = sizeof(WASMMiscEXTOpcode);
    data_size[WASMSimdEXTOpcodeT] = sizeof(WASMSimdEXTOpcode);
    data_size[WASMAtomicEXTOpcodeT] = sizeof(WASMAtomicEXTOpcode);
    data_size[HashMapElemT] = sizeof(HashMapElem);
    data_size[HashMapT] = sizeof(HashMap);
    data_size[timer_ctx_tT] = sizeof(timer_ctx_t);
    data_size[app_timer_tT] = sizeof(app_timer_t);
    data_size[bh_queueT] = sizeof(bh_queue);
    data_size[bh_queue_nodeT] = sizeof(bh_queue_node);
    data_size[AtomicWaitInfoT] = sizeof(AtomicWaitInfo);
    data_size[AtomicWaitNodeT] = sizeof(AtomicWaitNode);
    data_size[wasi_iovec_tT] = sizeof(wasi_iovec_t);
    data_size[wasi_ciovec_tT] = sizeof(wasi_ciovec_t);
}