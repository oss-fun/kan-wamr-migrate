#ifndef _WASM_RESTORE_H
#define _WASM_RESTORE_H

void
init_restore(void *addr);
void
restore_char(Pool_Info *addr);
void
restore_charT(Pool_Info *addr);
void
restore_uint8(Pool_Info *addr);
void
restore_uint16(Pool_Info *addr);
void
restore_uint32(Pool_Info *addr);
void
restore_uint64(Pool_Info *addr);
void
restore_gc_heap_t(Pool_Info *addr);
void
restore_base_addr(Pool_Info *addr);
void
restore_WASIContext(Pool_Info *addr);
void
restore_WASMThreadArg(Pool_Info *addr);
void
restore_ExternRefMapNode(Pool_Info *addr);
void
restore_fd_table(Pool_Info *addr);
void
restore_fd_prestats(Pool_Info *addr);
void
restore_argv_environ_values(Pool_Info *addr);
void
restore_uvwasi_t(Pool_Info *addr);
void
restore_uvwasi_preopen_t(Pool_Info *addr);
void
restore_wasm_val_t(Pool_Info *addr);
void
restore_wasm_instance_t(Pool_Info *addr);
void
restore_wasm_engine_t(Pool_Info *addr);
void
restore_wasm_store_t(Pool_Info *addr);
void
restore_Vector(Pool_Info *addr);
void
restore_wasm_valtype_t(Pool_Info *addr);
void
restore_wasm_functype_t(Pool_Info *addr);
void
restore_wasm_valtype_vec_t(Pool_Info *addr);
void
restore_wasm_globaltype_t(Pool_Info *addr);
void
restore_wasm_tabletype_t(Pool_Info *addr);
void
restore_wasm_memorytype_t(Pool_Info *addr);
void
restore_wasm_importtype_t(Pool_Info *addr);
void
restore_wasm_byte_vec_t(Pool_Info *addr);
void
restore_wasm_exporttype_t(Pool_Info *addr);
void
restore_wasm_byte_vec_t(Pool_Info *addr);
void
restore_wasm_ref_t(Pool_Info *addr);
void
restore_wasm_frame_t(Pool_Info *addr);
void
restore_wasm_trap_t(Pool_Info *addr);
void
restore_wasm_byte_vec_t(Pool_Info *addr);
void
restore_wasm_foreign_t(Pool_Info *addr);
void
restore_wasm_module_ex_t(Pool_Info *addr);
void
restore_wasm_func_t(Pool_Info *addr);
void
restore_wasm_global_t(Pool_Info *addr);
void
restore_wasm_table_t(Pool_Info *addr);
void
restore_wasm_memory_t(Pool_Info *addr);
void
restore_wasm_store_vec_t(Pool_Info *addr);
void
restore_wasm_module_vec_t(Pool_Info *addr);
void
restore_wasm_instance_vec_t(Pool_Info *addr);
void
restore_wasm_extern_vec_t(Pool_Info *addr);

void
restore_WASMRegisteredModule(Pool_Info *addr);
void
restore_LoadingModule(Pool_Info *addr);

void
restore_WASMExecEnv(Pool_Info *addr);
void
restore_NativeSymbolsNode(Pool_Info *addr);
void
restore_WASMModuleCommon(Pool_Info *addr);
void
restore_WASMModuleInstanceCommon(Pool_Info *addr);
void
restore_WASMModuleMemConsumption(Pool_Info *addr);
void
restore_WASMModuleInstMemConsumption(Pool_Info *addr);
void
restore_WASMMemoryInstanceCommon(Pool_Info *addr);
void
restore_WASMSection(Pool_Info *addr);
void
restore_WASMCApiFrame(Pool_Info *addr);
void
restore_WASMSharedMemNode(Pool_Info *addr);
void
restore_WASMModule(Pool_Info *addr);
void
restore_WASMFunction(Pool_Info *addr);
void
restore_WASMFunctionT(Pool_Info *addr);
void
restore_WASMGlobal(Pool_Info *addr);
void
restore_WASMExport(Pool_Info *addr);
void
restore_V128(Pool_Info *addr);
void
restore_WASMValue(Pool_Info *addr);
void
restore_InitializerExpression(Pool_Info *addr);
void
restore_WASMType(Pool_Info *addr);
void
restore_WASMTypeT(Pool_Info *addr);
void
restore_WASMTable(Pool_Info *addr);
void
restore_WASMMemory(Pool_Info *addr);
void
restore_WASMTableImport(Pool_Info *addr);
void
restore_WASMMemoryImport(Pool_Info *addr);
void
restore_WASMFunctionImport(Pool_Info *addr);
void
restore_WASMGlobalImport(Pool_Info *addr);
void
restore_WASMImport(Pool_Info *addr);
void
restore_WASMTableSeg(Pool_Info *addr);
void
restore_WASMDataSeg(Pool_Info *addr);
void
restore_WASMDataSegT(Pool_Info *addr);

void
restore_BlockAddr(Pool_Info *addr);
void
restore_WASIArguments(Pool_Info *addr);
void
restore_StringNode(Pool_Info *addr);
void
restore_BlockType(Pool_Info *addr);
void
restore_WASMBranchBlock(Pool_Info *addr);
void
restore_WASMInterpFrame(Pool_Info *addr);
void
restore_BranchBlock(Pool_Info *addr);
void
restore_BranchBlockPatch(Pool_Info *addr);
void
restore_WASMLoaderContext(Pool_Info *addr);
void
restore_Const(Pool_Info *addr);
void
restore_WASMModuleInstance(Pool_Info *addr);
void
restore_WASMFunctionInstance(Pool_Info *addr);
void
restore_WASMMemoryInstance(Pool_Info *addr);
void
restore_WASMMemoryInstanceT(Pool_Info *addr);
void
restore_WASMTableInstance(Pool_Info *addr);
void
restore_WASMTableInstanceT(Pool_Info *addr);
void
restore_WASMGlobalInstance(Pool_Info *addr);
void
restore_WASMExportFuncInstance(Pool_Info *addr);
void
restore_WASMExportGlobInstance(Pool_Info *addr);
void
restore_WASMSubModInstNode(Pool_Info *addr);
void
restore_WASMRuntimeFrame(Pool_Info *addr);

void
restore_WASMOpcode(Pool_Info *addr);
void
restore_WASMMiscEXTOpcode(Pool_Info *addr);
void
restore_WASMSimdEXTOpcode(Pool_Info *addr);
void
restore_WASMAtomicEXTOpcode(Pool_Info *addr);

void
restore_HashMapElem(Pool_Info *addr);
void
restore_HashMap(Pool_Info *addr);
void
restore_timer_ctx_t(Pool_Info *addr);
void
restore_app_timer_t(Pool_Info *addr);
void
restore_bh_queue(Pool_Info *addr);
void
restore_bh_queue_node(Pool_Info *addr);
void
restore_AtomicWaitInfo(Pool_Info *addr);
void
restore_AtomicWaitNode(Pool_Info *addr);

void
restore_wasi_iovec_t(Pool_Info *addr);
void
restore_wasi_ciovec_t(Pool_Info *addr);

void
restore_ERROR(Pool_Info *addr);

#endif // _WASM_RESTORE_H