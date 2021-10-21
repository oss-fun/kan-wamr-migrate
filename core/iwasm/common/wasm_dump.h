#ifndef WASM_DUMP_H
#define WASM_DUMP_H


void
init_dump(void *addr);

void
dump_char(Pool_Info *addr);
void
dump_charT(Pool_Info *addr);
void
dump_uint8(Pool_Info *addr);
void
dump_uint16(Pool_Info *addr);
void
dump_uint32(Pool_Info *addr);
void
dump_uint64(Pool_Info *addr);
void
dump_gc_heap_t(Pool_Info *addr);
void
dump_base_addr(Pool_Info *addr);
void
dump_WASIContext(Pool_Info *addr);
void
dump_WASMThreadArg(Pool_Info *addr);
void
dump_ExternRefMapNode(Pool_Info *addr);
void
dump_fd_table(Pool_Info *addr);
void
dump_fd_prestats(Pool_Info *addr);
void
dump_argv_environ_values(Pool_Info *addr);
void
dump_uvwasi_t(Pool_Info *addr);
void
dump_uvwasi_preopen_t(Pool_Info *addr);
void
dump_wasm_val_t(Pool_Info *addr);
void
dump_wasm_instance_t(Pool_Info *addr);
void
dump_wasm_engine_t(Pool_Info *addr);
void
dump_wasm_store_t(Pool_Info *addr);
void
dump_Vector(Pool_Info *addr);
void
dump_wasm_valtype_t(Pool_Info *addr);
void
dump_wasm_functype_t(Pool_Info *addr);
void
dump_wasm_valtype_vec_t(Pool_Info *addr);
void
dump_wasm_globaltype_t(Pool_Info *addr);
void
dump_wasm_tabletype_t(Pool_Info *addr);
void
dump_wasm_memorytype_t(Pool_Info *addr);
void
dump_wasm_importtype_t(Pool_Info *addr);
void
dump_wasm_byte_vec_t(Pool_Info *addr);
void
dump_wasm_exporttype_t(Pool_Info *addr);
void
dump_wasm_byte_vec_t(Pool_Info *addr);
void
dump_wasm_ref_t(Pool_Info *addr);
void
dump_wasm_frame_t(Pool_Info *addr);
void
dump_wasm_trap_t(Pool_Info *addr);
void
dump_wasm_byte_vec_t(Pool_Info *addr);
void
dump_wasm_foreign_t(Pool_Info *addr);
void
dump_wasm_module_ex_t(Pool_Info *addr);
void
dump_wasm_func_t(Pool_Info *addr);
void
dump_wasm_global_t(Pool_Info *addr);
void
dump_wasm_table_t(Pool_Info *addr);
void
dump_wasm_memory_t(Pool_Info *addr);
void
dump_WASMRegisteredModule(Pool_Info *addr);
void
dump_LoadingModule(Pool_Info *addr);

void
dump_WASMExecEnv(Pool_Info *addr);
void
dump_NativeSymbolsNode(Pool_Info *addr);
void
dump_WASMModuleCommon(Pool_Info *addr);
void
dump_WASMModuleInstanceCommon(Pool_Info *addr);
void
dump_WASMModuleMemConsumption(Pool_Info *addr);
void
dump_WASMModuleInstMemConsumption(Pool_Info *addr);
void
dump_WASMMemoryInstanceCommon(Pool_Info *addr);
void
dump_WASMSection(Pool_Info *addr);
void
dump_WASMCApiFrame(Pool_Info *addr);
void
dump_WASMSharedMemNode(Pool_Info *addr);
void
dump_WASMModule(Pool_Info *addr);
void
dump_WASMFunction(Pool_Info *addr);
void
dump_WASMFunctionT(Pool_Info *addr);
void
dump_WASMGlobal(Pool_Info *addr);
void
dump_WASMExport(Pool_Info *addr);
void
dump_V128(Pool_Info *addr);
void
dump_WASMValue(Pool_Info *addr);
void
dump_InitializerExpression(Pool_Info *addr);
void
dump_WASMType(Pool_Info *addr);
void
dump_WASMTypeT(Pool_Info *addr);
void
dump_WASMTable(Pool_Info *addr);
void
dump_WASMMemory(Pool_Info *addr);
void
dump_WASMTableImport(Pool_Info *addr);
void
dump_WASMMemoryImport(Pool_Info *addr);
void
dump_WASMFunctionImport(Pool_Info *addr);
void
dump_WASMGlobalImport(Pool_Info *addr);
void
dump_WASMImport(Pool_Info *addr);
void
dump_WASMTableSeg(Pool_Info *addr);
void
dump_WASMDataSeg(Pool_Info *addr);
void
dump_WASMDataSegT(Pool_Info *addr);

void
dump_BlockAddr(Pool_Info *addr);
void
dump_WASIArguments(Pool_Info *addr);
void
dump_StringNode(Pool_Info *addr);
void
dump_BlockType(Pool_Info *addr);
void
dump_WASMBranchBlock(Pool_Info *addr);
void
dump_WASMInterpFrame(Pool_Info *addr);
void
dump_BranchBlock(Pool_Info *addr);
void
dump_BranchBlockPatch(Pool_Info *addr);
void
dump_WASMLoaderContext(Pool_Info *addr);
void
dump_Const(Pool_Info *addr);
void
dump_WASMModuleInstance(Pool_Info *addr);
void
dump_WASMFunctionInstance(Pool_Info *addr);
void
dump_WASMMemoryInstance(Pool_Info *addr);
void
dump_WASMMemoryInstanceT(Pool_Info *addr);
void
dump_WASMTableInstance(Pool_Info *addr);
void
dump_WASMTableInstanceT(Pool_Info *addr);
void
dump_WASMGlobalInstance(Pool_Info *addr);
void
dump_WASMExportFuncInstance(Pool_Info *addr);
void
dump_WASMRuntimeFrame(Pool_Info *addr);
void
dump_WASMOpcode(Pool_Info *addr);
void
dump_WASMMiscEXTOpcode(Pool_Info *addr);
void
dump_WASMSimdEXTOpcode(Pool_Info *addr);
void
dump_WASMAtomicEXTOpcode(Pool_Info *addr);
void dump_HashMapElem(Pool_Info* addr);
void dump_HashMap(Pool_Info *addr);
void dump_timer_ctx_t(Pool_Info *addr);
void dump_app_timer_t(Pool_Info *addr);
void dump_bh_queue(Pool_Info *addr);
void dump_bh_queue_node(Pool_Info *addr);
void
dump_ERRO(Pool_Info *addr);

#endif // WASM_DUMP_H