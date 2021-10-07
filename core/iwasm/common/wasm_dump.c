#include "wasm_runtime_common.h"
#include "bh_platform.h"
#include "mem_alloc.h"
#include "wasm_memory.h"
#include "wasm_dump.h"


void dump_char(void *addr);
void dump_charT(void *addr);
void dump_uint8(void *addr);
void dump_uint16(void *addr);
void dump_uint32(void *addr);
void dump_uint64(void *addr);

void dump_gc_heap_t(void *addr);
void dump_base_addr(void *addr);
void dump_WASIContext(void *addr);
void dump_WASMThreadArg(void *addr);
void dump_ExternRefMapNode(void *addr);

void dump_fd_table(void *addr);
void dump_fd_prestats(void *addr);
void dump_argv_environ_values(void *addr);

void dump_uvwasi_t(void *addr);
void dump_uvwasi_preopen_t(void *addr);

void dump_wasm_val_t(void *addr);

void dump_WASMExecEnv(void *addr);

void dump_NativeSymbolsNode(void *addr);

void dump_WASMModuleCommon(void *addr);
void dump_WASMModuleInstanceCommon(void *addr);
void dump_WASMModuleMemConsumption(void *addr);
void dump_WASMModuleInstMemConsumption(void *addr);
void dump_WASMMemoryInstanceCommon(void *addr);
void dump_WASMSection(void *addr);
void dump_WASMCApiFrame(void *addr);

void dump_WASMSharedMemNode(void *addr);

void dump_WASMModule(void *addr);
void dump_WASMFunction(void *addr);
void dump_WASMFunctionT(void *addr);
void dump_WASMGlobal(void *addr);
void dump_WASMExport(void *addr);
void dump_V128(void *addr);
void dump_WASMValue(void *addr);
void dump_InitializerExpression(void *addr);
void dump_WASMType(void *addr);
void dump_WASMTypeT(void *addr);
void dump_WASMTable(void *addr);
void dump_WASMMemory(void *addr);
void dump_WASMTableImport(void *addr);
void dump_WASMMemoryImport(void *addr);
void dump_WASMFunctionImport(void *addr);
void dump_WASMGlobalImport(void *addr);
void dump_WASMImport(void *addr);
void dump_WASMTableSeg(void *addr);
void dump_WASMDataSeg(void *addr);
void dump_BlockAddr(void *addr);
void dump_WASIArguments(void *addr);
void dump_StringNode(void *addr);
void dump_BlockType(void *addr);
void dump_WASMBranchBlock(void *addr);

void dump_WASMInterpFrame(void *addr);

void dump_BranchBlock(void *addr);
void dump_WASMLoaderContext(void *addr);
void dump_Const(void *addr);

void dump_WASMModuleInstance(void *addr);
void dump_WASMFunctionInstance(void *addr);
void dump_WASMMemoryInstance(void *addr);
void dump_WASMMemoryInstanceT(void *addr);
void dump_WASMTableInstance(void *addr);
void dump_WASMTableInstanceT(void *addr);
void dump_WASMGlobalInstance(void *addr);
void dump_WASMExportFuncInstance(void *addr);
void dump_WASMRuntimeFrame(void *addr);

void dump_WASMOpcode(void *addr);
void dump_WASMMiscEXTOpcode(void *addr);
void dump_WASMSimdEXTOpcode(void *addr);
void dump_WASMAtomicEXTOpcode(void *addr);















