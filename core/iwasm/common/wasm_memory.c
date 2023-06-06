/*
 * Copyright (C) 2019 Intel Corporation.  All rights reserved.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 */
#include <stdlib.h>
#include "wasm_runtime_common.h"
#include "bh_platform.h"
#include "mem_alloc.h"
#include "ems/ems_gc_internal.h"
#include "wasm_memory.h"
#include "wasm_c_api_internal.h"
#include "wasm_exec_env.h"
#include "wasm_native.h"
#include "wasm_shared_memory.h"
#include "wasm_dump.h"
#include "wasm_restore.h"
#include "wasm_util.h"
#include "../interpreter/wasm_opcode.h"
#include "../interpreter/wasm_runtime.h"
#include "../interpreter/wasm_loader.h"
#include "../interpreter/wasm_interp.h"
#include "../interpreter/wasm.h"
#include "wasm_c_api.h"
#if WASM_ENABLE_LIBC_WASI != 0
#include "../libraries/libc-wasi/libc_wasi_wrapper.h"
#endif

typedef enum Memory_Mode {
    MEMORY_MODE_UNKNOWN = 0,
    MEMORY_MODE_POOL,
    MEMORY_MODE_ALLOCATOR
} Memory_Mode;

static Memory_Mode memory_mode = MEMORY_MODE_UNKNOWN;

static mem_allocator_t pool_allocator = NULL;

static void *(*malloc_func)(unsigned int size) = NULL;
static void *(*realloc_func)(void *ptr, unsigned int size) = NULL;
static void (*free_func)(void *ptr) = NULL;

static unsigned int global_pool_size;

#define MAX_LIST_SIZE 512 * 1024

static Pool_Info *root_info = NULL;
static Pool_Info *root_frame = NULL;

static void (*dump_data[ERRORT])(Pool_Info *addr);

static void
init_dump_func(void)
{
    dump_data[charT] = dump_char;
    dump_data[charTT] = dump_charT;
    dump_data[uint8T] = dump_uint8;
    dump_data[uint16T] = dump_uint16;
    dump_data[uint32T] = dump_uint32;
    dump_data[uint64T] = dump_uint64;
    dump_data[gc_heap_tT] = dump_gc_heap_t;

    dump_data[WASIContextT] = dump_WASIContext;
    dump_data[WASMThreadArgT] = dump_WASMThreadArg;
    dump_data[ExternRefMapNodeT] = dump_ExternRefMapNode;
    dump_data[fd_tableT] = dump_fd_table;
    dump_data[fd_prestatsT] = dump_fd_prestats;
    dump_data[argv_environ_valuesT] = dump_argv_environ_values;
    dump_data[uvwasi_tT] = dump_uvwasi_t;
    dump_data[uvwasi_preopen_tT] = dump_uvwasi_preopen_t;
    dump_data[wasi_iovec_tT] = dump_wasi_iovec_t;
    dump_data[wasi_ciovec_tT] = dump_wasi_ciovec_t;

    dump_data[wasm_val_tT] = dump_wasm_val_t;
    dump_data[wasm_instance_tT] = dump_wasm_instance_t;
    dump_data[wasm_engine_tT] = dump_wasm_engine_t;
    dump_data[wasm_store_tT] = dump_wasm_store_t;
    dump_data[VectorT] = dump_Vector;
    dump_data[wasm_valtype_tT] = dump_wasm_valtype_t;
    dump_data[wasm_functype_tT] = dump_wasm_functype_t;
    dump_data[wasm_valtype_vec_tT] = dump_wasm_valtype_vec_t;
    dump_data[wasm_globaltype_tT] = dump_wasm_globaltype_t;
    dump_data[wasm_tabletype_tT] = dump_wasm_tabletype_t;
    dump_data[wasm_memorytype_tT] = dump_wasm_memorytype_t;
    dump_data[wasm_importtype_tT] = dump_wasm_importtype_t;
    dump_data[wasm_byte_vec_tT] = dump_wasm_byte_vec_t;
    dump_data[wasm_exporttype_tT] = dump_wasm_exporttype_t;
    dump_data[wasm_ref_tT] = dump_wasm_ref_t;
    dump_data[wasm_frame_tT] = dump_wasm_frame_t;
    dump_data[wasm_trap_tT] = dump_wasm_trap_t;
    dump_data[wasm_foreign_tT] = dump_wasm_foreign_t;
    dump_data[wasm_module_ex_tT] = dump_wasm_module_ex_t;
    dump_data[wasm_func_tT] = dump_wasm_func_t;
    dump_data[wasm_global_tT] = dump_wasm_global_t;
    dump_data[wasm_table_tT] = dump_wasm_table_t;
    dump_data[wasm_memory_tT] = dump_wasm_memory_t;
    dump_data[wasm_store_vec_tT] = dump_wasm_store_vec_t;
    dump_data[wasm_module_vec_tT] = dump_wasm_module_vec_t;
    dump_data[wasm_instance_vec_tT] = dump_wasm_instance_vec_t;
    dump_data[wasm_extern_vec_tT] = dump_wasm_extern_vec_t;

    dump_data[WASMRegisteredModuleT] = dump_WASMRegisteredModule;
    dump_data[LoadingModuleT] = dump_LoadingModule;

    dump_data[WASMExecEnvT] = dump_WASMExecEnv;
    dump_data[NativeSymbolsNodeT] = dump_NativeSymbolsNode;
    dump_data[WASMModuleCommonT] = dump_WASMModuleCommon;
    dump_data[WASMModuleInstanceCommonT] = dump_WASMModuleInstanceCommon;
    dump_data[WASMModuleMemConsumptionT] = dump_WASMModuleMemConsumption;
    dump_data[WASMModuleInstMemConsumptionT] =
      dump_WASMModuleInstMemConsumption;
    dump_data[WASMMemoryInstanceCommonT] = dump_WASMMemoryInstanceCommon;
    dump_data[WASMSectionT] = dump_WASMSection;
    dump_data[WASMCApiFrameT] = dump_WASMCApiFrame;
    dump_data[WASMSharedMemNodeT] = dump_WASMSharedMemNode;
    dump_data[WASMModuleT] = dump_WASMModule;
    dump_data[WASMFunctionT] = dump_WASMFunction;
    dump_data[WASMFunctionTT] = dump_WASMFunctionT;
    dump_data[WASMGlobalT] = dump_WASMGlobal;
    dump_data[WASMExportT] = dump_WASMExport;
    dump_data[V128T] = dump_V128;
    dump_data[WASMValueT] = dump_WASMValue;
    dump_data[InitializerExpressionT] = dump_InitializerExpression;
    dump_data[WASMTypeT] = dump_WASMType;
    dump_data[WASMTypeTT] = dump_WASMTypeT;
    dump_data[WASMTableT] = dump_WASMTable;
    dump_data[WASMMemoryT] = dump_WASMMemory;
    dump_data[WASMTableImportT] = dump_WASMTableImport;
    dump_data[WASMMemoryImportT] = dump_WASMMemoryImport;
    dump_data[WASMFunctionImportT] = dump_WASMFunctionImport;
    dump_data[WASMGlobalImportT] = dump_WASMGlobalImport;
    dump_data[WASMImportT] = dump_WASMImport;
    dump_data[WASMTableSegT] = dump_WASMTableSeg;
    dump_data[WASMDataSegT] = dump_WASMDataSeg;
    dump_data[WASMDataSegTT] = dump_WASMDataSegT;
    dump_data[BlockAddrT] = dump_BlockAddr;
    dump_data[WASIArgumentsT] = dump_WASIArguments;
    dump_data[StringNodeT] = dump_StringNode;
    dump_data[BlockTypeT] = dump_BlockType;
    dump_data[WASMBranchBlockT] = dump_WASMBranchBlock;
    dump_data[WASMInterpFrameT] = dump_WASMInterpFrame;
    dump_data[BranchBlockT] = dump_BranchBlock;
    dump_data[BranchBlockPatchT] = dump_BranchBlockPatch;
    dump_data[WASMLoaderContextT] = dump_WASMLoaderContext;
    dump_data[ConstT] = dump_Const;
    dump_data[WASMModuleInstanceT] = dump_WASMModuleInstance;
    dump_data[WASMFunctionInstanceT] = dump_WASMFunctionInstance;
    dump_data[WASMMemoryInstanceT] = dump_WASMMemoryInstance;
    dump_data[WASMMemoryInstanceTT] = dump_WASMMemoryInstanceT;
    dump_data[WASMTableInstanceT] = dump_WASMTableInstance;
    dump_data[WASMTableInstanceTT] = dump_WASMTableInstanceT;
    dump_data[WASMGlobalInstanceT] = dump_WASMGlobalInstance;
    dump_data[WASMExportFuncInstanceT] = dump_WASMExportFuncInstance;

    dump_data[WASMExportGlobInstanceT] = dump_WASMExportGlobInstance;
    dump_data[WASMSubModInstNodeT] = dump_WASMSubModInstNode;
    dump_data[WASMRuntimeFrameT] = dump_WASMRuntimeFrame;
    dump_data[WASMOpcodeT] = dump_WASMOpcode;
    dump_data[WASMMiscEXTOpcodeT] = dump_WASMMiscEXTOpcode;
    dump_data[WASMSimdEXTOpcodeT] = dump_WASMSimdEXTOpcode;
    dump_data[WASMAtomicEXTOpcodeT] = dump_WASMAtomicEXTOpcode;

    dump_data[HashMapElemT] = dump_HashMapElem;
    dump_data[HashMapT] = dump_HashMap;
    dump_data[timer_ctx_tT] = dump_timer_ctx_t;
    dump_data[app_timer_tT] = dump_app_timer_t;
    dump_data[bh_queueT] = dump_bh_queue;
    dump_data[bh_queue_nodeT] = dump_bh_queue_node;

    dump_data[AtomicWaitInfoT] = dump_AtomicWaitInfo;
    dump_data[AtomicWaitNodeT] = dump_AtomicWaitNode;
}

#define CASE_INFOS(data_type)                                                 \
    case data_type##T:                                                        \
        for (i = 1; i < size; i++) {                                          \
            info = malloc(sizeof(Pool_Info));                                 \
            info->p_abs = addr - pool_allocator + i * sizeof(data_type);      \
            info->p_raw = addr + i * sizeof(data_type);                       \
            info->size = -1;                                                  \
            info->type = type;                                                \
            info->list = NULL;                                                \
            info->next = NULL;                                                \
            p->list = info;                                                   \
            p = info;                                                         \
        }                                                                     \
        break;

void *
get_base_addr(void)
{
    return pool_allocator;
}

Pool_Info *
get_raw(void *addr)
{
    Pool_Info *info = root_info, *p;
    if (addr == NULL) {
        return NULL;
    }
    while (info) {
        for (p = info; p != NULL; p = p->list) {
            if (p->p_raw == addr) {
                return p;
            }
        }
        info = info->next;
    }
    printf("error p_abs:%p\n", addr);
    exit(1);

    return NULL;
}

Pool_Info *
get_info(int p_abs)
{
    Pool_Info *info = root_info, *p;
    if (p_abs == -1) {
        return NULL;
    }
    while (info) {
        for (p = info; p != NULL; p = p->list) {
            if (p_abs == p->p_abs) {
                return p;
            }
        }
        info = info->next;
    }
    printf("error p_abs:%d\n", p_abs);
    exit(1);

    return NULL;
}

Pool_Info *
get_frame_info(int p_abs)
{
    Pool_Info *info = root_frame, *p;
    if (p_abs == -1) {
        return NULL;
    }
    while (info) {
        if (info->p_abs == p_abs) {
            return info;
        }
        info = info->next;
    }
    printf("error frame p_abs:%d\n", p_abs);
    exit(1);

    return NULL;
}

Pool_Info *
get_root_frame_info(void)
{
    return root_frame;
}

void
abs_translation(void)
{
    Pool_Info *info = root_info, *p;
    unsigned long _p;

    while (info) {
        p = info->list;
        while (p) {
            p->p_abs = (unsigned long)p->p_raw - (unsigned long)pool_allocator;
            p = p->list;
        }
        info->p_abs =
          (unsigned long)info->p_raw - (unsigned long)pool_allocator;
        info = info->next;
    }
}

void
restore_runtime(void)
{
    int i;
    Pool_Info *info, *p;
    FILE *fp;
    fp = fopen("pool_info.img", "rb");
    int p_abs;
    int type;
    int size;

    set_data_size();
    init_restore();

    while (!feof(fp)) {
        if (fread(&p_abs, sizeof(int), 1, fp) == 0) {
            break;
        }
        fread(&type, sizeof(int), 1, fp);
        fread(&size, sizeof(int), 1, fp);

        info = calloc(1, sizeof(Pool_Info));
        info->p_abs = p_abs;
        info->type = type;
        info->size = size;
        info->list = NULL;

        switch (type) {
            //offsetof
            case WASMExecEnvT:
            case WASMTypeT:
            case WASMTableInstanceT:
            case StringNodeT:
            case WASMFunctionT:
                if (data_size[type] + size == 0) {
                    printf("type:%d\n", type);
                    printf("%d:%d\n", data_size[type], size);
                    exit(1);
                }

                info->p_raw = wasm_runtime_malloc(data_size[type] + size);
                if (type == WASMExecEnvT) {
                    set_WASMExecEnv(info->p_raw);
                }
                info->next = root_info;
                root_info = info;
                break;

            case WASMInterpFrameT:
                if (root_frame == NULL) {
                    info->next = root_frame;
                    root_frame = info;
                }
                else {
                    Pool_Info *p = root_frame, *q = NULL;
                    while (p != NULL && info->p_abs > p->p_abs) {
                        q = p;
                        p = p->next;
                    }
                    if (q == NULL) {
                        info->next = root_frame;
                        root_frame = info;
                    }
                    else {
                        q->next = info;
                        info->next = p;
                    }
                }
                break;

            case gc_heap_tT:
            case WASIContextT:
            case fd_tableT:
            case fd_prestatsT:
            case argv_environ_valuesT:
            case WASI_FILE_T:
                free(info);
                break;

            case charT:
            case charTT:
            case uint8T:
            case uint16T:
            case uint32T:
            case uint64T:
            case WASMFunctionTT:
            case WASMTypeTT:
            case WASMDataSegTT:
            case WASMMemoryInstanceTT:
            case WASMTableInstanceTT:
                info->p_raw = wasm_runtime_malloc(data_size[type] * size);
                info->next = root_info;
                root_info = info;
                break;

            default:
                if (data_size[type] * size == 0) {
                    printf("default type:%d\n", type);
                    exit(1);
                }
                info->p_raw = wasm_runtime_malloc(data_size[type] * size);
                if (type == WASMModuleT) {
                    set_WASMModule(info->p_raw);
                }
                Pool_Info *q = info;
                for (i = 1; i < size; i++) {
                    p = calloc(1, sizeof(Pool_Info));
                    fread(&p_abs, sizeof(int), 1, fp);
                    p->p_abs = p_abs;
                    p->p_raw = info->p_raw + i * data_size[type];
                    p->size = -1;
                    p->type = type;
                    p->next = NULL;
                    p->list = NULL;
                    q->list = p;
                    q = p;
                }
                info->next = root_info;
                root_info = info;
                break;
        }
    }

    restore_internal();
    restore_frame_internal();
    info = root_frame;
    while (info->next) {
        info = info->next;
    }
    info->next = root_info;
    root_info = root_frame;
    //root_frame = NULL;
}

void
dump_runtime(void)
{
#if WASM_ENABLE_AOT != 0 || WASM_ENABLE_MULTI_MODULE != 0
    return;
#endif
    Pool_Info *info = root_info, *p;
    int i;
    FILE *fp;
    fp = fopen("pool_info.img", "wb");

    init_dump_func();
    init_dump(pool_allocator);

    while (info) {
        switch (info->type) {
            case gc_heap_tT:
            case WASIContextT:
            case WASI_FILE_T:
            case NativeSymbolsNodeT:
            case fd_prestatsT:
            case fd_tableT:
            case argv_environ_valuesT:
                info = info->next;
                continue;
            default:
                (*dump_data[info->type])(info);
        }
        //printf("dump p_abs:%d\n",info->p_abs);
        fwrite(&info->p_abs, sizeof(int), 1, fp);
        fwrite(&info->type, sizeof(int), 1, fp);
        fwrite(&info->size, sizeof(int), 1, fp);
        //printf("%d:%d:%d\n", info->p_abs, info->type, info->size);
        if (info->list != NULL) {
            p = info->list;
            for (i = 1; i < info->size; i++) {
                //printf("dump p_abs:%d\n",p->p_abs);
                fwrite(&p->p_abs, sizeof(int), 1, fp);
                p = p->list;
            }
            if (p != NULL) {
                printf("Dump list error\n");
                exit(1);
            }
        }

        info = info->next;
    }
    printf("Done\n");
    fclose(fp);
}

void
alloc_info(void *addr, Data_Type type)
{
    if (type == WASMModuleT) {
        set_WASMModule((WASMModule *)addr);
    }
    alloc_info_buf(addr, type, 1);
}

void
alloc_info_buf(void *addr, Data_Type type, size_t buf_size)
{
    if (!addr) {
        printf("error\n");
        exit(1);
    }
#ifdef __FREE_DEBUG
    printf("buf:[%p]:[%d]:[%ld]\n", addr, type, buf_size);
#endif
    Pool_Info *info = malloc(sizeof(Pool_Info));
    info->p_abs = addr - pool_allocator;
    info->p_raw = addr;
    info->size = buf_size;
    info->type = type;
    info->list = NULL;

    info->next = root_info;
    root_info = info;
}

void
alloc_info_ex(void *addr, Data_Type type, size_t size)
{
    if (!addr) {
        printf("error\n");
        exit(1);
    }
    if (type == WASMExecEnvT) {
        set_WASMExecEnv(addr);
    }

#ifdef __FREE_DEBUG
    printf("ex:[%p]:[%d]:[%ld]\n", addr, type, size);
#endif
    Pool_Info *info = malloc(sizeof(Pool_Info));
    info->p_abs = addr - pool_allocator;
    info->p_raw = addr;
    info->size = size;
    info->type = type;
    info->list = NULL;

    info->next = root_info;
    root_info = info;
}

void
alloc_infos(void *addr, Data_Type type, size_t size)
{
    int i;
    if (!addr) {
        printf("error\n");
        exit(1);
    }
#ifdef __FREE_DEBUG
    printf("s:[%p]:[%d]:[%ld]\n", addr, type, size);
#endif
    Pool_Info *info, *p;
    info = malloc(sizeof(Pool_Info));
    info->p_abs = addr - pool_allocator;
    info->p_raw = addr;
    info->size = size;
    info->type = type;
    info->list = NULL;
    info->next = root_info;
    root_info = info;
    p = root_info;

    switch (type) {
        CASE_INFOS(char)
        CASE_INFOS(uint8)
        CASE_INFOS(uint16)
        CASE_INFOS(uint32)
        CASE_INFOS(uint64)

        CASE_INFOS(gc_heap_t)

#if WASM_ENABLE_LIBC_WASI != 0
        CASE_INFOS(WASIContext)
        CASE_INFOS(WASIArguments)
        CASE_INFOS(wasi_iovec_t)
        CASE_INFOS(wasi_ciovec_t)
#endif
        //CASE_INFOS(WASMThreadArg)
        //CASE_INFOS(ExternRefMapNode)

        //CASE_INFOS(fd_table)
        //CASE_INFOS(fd_prestats)
        //CASE_INFOS(argv_environ_values)

        //CASE_INFOS(uvwasi_t)
        //CASE_INFOS(uvwasi_preopen_t)

        CASE_INFOS(wasm_val_t)
        CASE_INFOS(WASMExecEnv)

        CASE_INFOS(NativeSymbolsNode)

        CASE_INFOS(WASMModuleCommon)
        CASE_INFOS(WASMModuleInstanceCommon)
        CASE_INFOS(WASMModuleMemConsumption)
        CASE_INFOS(WASMModuleInstMemConsumption)
        CASE_INFOS(WASMMemoryInstanceCommon)
        CASE_INFOS(WASMSection)
        CASE_INFOS(WASMCApiFrame)

        CASE_INFOS(wasm_instance_t)
        CASE_INFOS(wasm_engine_t)
        CASE_INFOS(wasm_store_t)
        CASE_INFOS(Vector)
        CASE_INFOS(wasm_valtype_t)
        CASE_INFOS(wasm_functype_t)
        CASE_INFOS(wasm_valtype_vec_t)
        CASE_INFOS(wasm_globaltype_t)
        CASE_INFOS(wasm_tabletype_t)
        CASE_INFOS(wasm_memorytype_t)
        CASE_INFOS(wasm_importtype_t)
        CASE_INFOS(wasm_byte_vec_t)
        CASE_INFOS(wasm_exporttype_t)
        CASE_INFOS(wasm_ref_t)
        CASE_INFOS(wasm_frame_t)
        CASE_INFOS(wasm_trap_t)
        CASE_INFOS(wasm_foreign_t)
        CASE_INFOS(wasm_module_ex_t)
        CASE_INFOS(wasm_func_t)
        CASE_INFOS(wasm_global_t)
        CASE_INFOS(wasm_table_t)
        CASE_INFOS(wasm_memory_t)
        CASE_INFOS(wasm_store_vec_t)
        CASE_INFOS(wasm_module_vec_t)
        CASE_INFOS(wasm_instance_vec_t)
        CASE_INFOS(wasm_extern_vec_t)

        CASE_INFOS(WASMSharedMemNode)

        CASE_INFOS(WASMModule)
        CASE_INFOS(WASMFunction)
        CASE_INFOS(WASMGlobal)
        CASE_INFOS(WASMExport)
        CASE_INFOS(V128)
        CASE_INFOS(WASMValue)
        CASE_INFOS(InitializerExpression)
        CASE_INFOS(WASMType)
        CASE_INFOS(WASMTable)
        CASE_INFOS(WASMMemory)
        CASE_INFOS(WASMTableImport)
        CASE_INFOS(WASMMemoryImport)
        CASE_INFOS(WASMFunctionImport)
        CASE_INFOS(WASMGlobalImport)
        CASE_INFOS(WASMImport)
        CASE_INFOS(WASMTableSeg)
        CASE_INFOS(WASMDataSeg)
        CASE_INFOS(BlockAddr)

        CASE_INFOS(StringNode)
        CASE_INFOS(BlockType)
        CASE_INFOS(WASMBranchBlock)

        CASE_INFOS(WASMInterpFrame)

        CASE_INFOS(BranchBlock)
        CASE_INFOS(WASMLoaderContext)
        CASE_INFOS(Const)

        CASE_INFOS(WASMModuleInstance)

        CASE_INFOS(WASMFunctionInstance)
        CASE_INFOS(WASMMemoryInstance)
        CASE_INFOS(WASMTableInstance)
        CASE_INFOS(WASMGlobalInstance)
        CASE_INFOS(WASMExportFuncInstance)

        CASE_INFOS(WASMRuntimeFrame)
        CASE_INFOS(WASMOpcode)
        CASE_INFOS(WASMMiscEXTOpcode)
        CASE_INFOS(WASMSimdEXTOpcode)
        CASE_INFOS(WASMAtomicEXTOpcode)
        CASE_INFOS(HashMapElem)
        CASE_INFOS(HashMap)
        CASE_INFOS(timer_ctx_t)
        CASE_INFOS(app_timer_t)
        CASE_INFOS(bh_queue)
        CASE_INFOS(bh_queue_node)
        CASE_INFOS(AtomicWaitInfo)
        CASE_INFOS(AtomicWaitNode)
#if WASM_ENABLE_FAST_INTERP != 0
        CASE_INFOS(BranchBlockPatch)
#endif

#if WASM_ENABLE_MULTI_MODULE != 0
        CASE_INFOS(WASMRegisteredModule)
        CASE_INFOS(LoadingModule)
        CASE_INFOS(WASMExportGlobInstance)
        CASE_INFOS(WASMSubModInstNode)
#endif
        default:
            break;
    }
}

void
free_info(void *addr)
{
#if WASM_ENABLE_AOT != 0 || WASM_ENABLE_MULTI_MODULE != 0
    return;
#endif
    Pool_Info *info, *prev, *p;

    info = root_info;
    prev = NULL;
    while (info) {
        if (info->p_raw == addr) {
            while (info->list) {
                p = info->list->list;
                free(info->list);
                info->list = p;
            }

            if (prev == NULL) {
                root_info = info->next;
            }
            else {
                prev->next = info->next;
            }
            //printf("free:%d\n",info->type);
            free(info);
            return;
        }
        prev = info;
        info = info->next;
    }

    info = root_frame;
    prev = NULL;
    while (info) {
        if (info->p_raw == addr) {
            while (info->list) {
                p = info->list->list;
                free(info->list);
                info->list = p;
            }

            if (prev == NULL) {
                root_frame = info->next;
            }
            else {
                prev->next = info->next;
            }
            //printf("free:%d\n",info->type);
            free(info);
            return;
        }
        prev = info;
        info = info->next;
    }

    printf("free error[%p]\n", addr);

    info = root_info;
    prev = NULL;
    while (info) {
        p = info->list;
        while (p) {
            if (p == addr) {
                printf("found\n");
                return;
            }
            p = p->list;
        }

        prev = info;
        info = info->next;
    }
    exit(1);
}

static bool
wasm_memory_init_with_pool(void *mem, unsigned int bytes)
{
    mem_allocator_t _allocator = mem_allocator_create(mem, bytes);

    if (_allocator) {
        memory_mode = MEMORY_MODE_POOL;
        pool_allocator = _allocator;
        global_pool_size = bytes;
        return true;
    }
    LOG_ERROR("Init memory with pool (%p, %u) failed.\n", mem, bytes);
    return false;
}

static bool
wasm_memory_init_with_allocator(void *_malloc_func,
                                void *_realloc_func,
                                void *_free_func)
{
    if (_malloc_func && _free_func && _malloc_func != _free_func) {
        memory_mode = MEMORY_MODE_ALLOCATOR;
        malloc_func = _malloc_func;
        realloc_func = _realloc_func;
        free_func = _free_func;
        return true;
    }
    LOG_ERROR("Init memory with allocator (%p, %p, %p) failed.\n",
              _malloc_func, _realloc_func, _free_func);
    return false;
}

bool
wasm_runtime_memory_init(mem_alloc_type_t mem_alloc_type,
                         const MemAllocOption *alloc_option)
{
    if (mem_alloc_type == Alloc_With_Pool)
        return wasm_memory_init_with_pool(alloc_option->pool.heap_buf,
                                          alloc_option->pool.heap_size);
    else if (mem_alloc_type == Alloc_With_Allocator)
        return wasm_memory_init_with_allocator(
          alloc_option->allocator.malloc_func,
          alloc_option->allocator.realloc_func,
          alloc_option->allocator.free_func);
    else if (mem_alloc_type == Alloc_With_System_Allocator)
        return wasm_memory_init_with_allocator(os_malloc, os_realloc, os_free);
    else
        return false;
}

void
wasm_runtime_memory_destroy()
{
    if (memory_mode == MEMORY_MODE_POOL)
        mem_allocator_destroy(pool_allocator);
    memory_mode = MEMORY_MODE_UNKNOWN;
}

unsigned
wasm_runtime_memory_pool_size()
{
    if (memory_mode == MEMORY_MODE_POOL)
        return global_pool_size;
    else
        return 1 * BH_GB;
}

static inline void *
wasm_runtime_malloc_internal(unsigned int size)
{
    if (memory_mode == MEMORY_MODE_UNKNOWN) {
        LOG_WARNING(
          "wasm_runtime_malloc failed: memory hasn't been initialize.\n");
        return NULL;
    }
    else if (memory_mode == MEMORY_MODE_POOL) {
        return mem_allocator_malloc(pool_allocator, size);
    }
    else {
        return malloc_func(size);
    }
}

static inline void *
wasm_runtime_realloc_internal(void *ptr, unsigned int size)
{
    if (memory_mode == MEMORY_MODE_UNKNOWN) {
        LOG_WARNING(
          "wasm_runtime_realloc failed: memory hasn't been initialize.\n");
        return NULL;
    }
    else if (memory_mode == MEMORY_MODE_POOL) {
        return mem_allocator_realloc(pool_allocator, ptr, size);
    }
    else {
        if (realloc_func)
            return realloc_func(ptr, size);
        else
            return NULL;
    }
}

static inline void
wasm_runtime_free_internal(void *ptr)
{
    if (!ptr) {
        LOG_WARNING("warning: wasm_runtime_free with NULL pointer\n");
        return;
    }

    if (memory_mode == MEMORY_MODE_UNKNOWN) {
        LOG_WARNING("warning: wasm_runtime_free failed: "
                    "memory hasn't been initialize.\n");
    }
    else if (memory_mode == MEMORY_MODE_POOL) {
        mem_allocator_free(pool_allocator, ptr);
    }
    else {
        free_func(ptr);
    }
}

void *
wasm_runtime_malloc(unsigned int size)
{
    if (size == 0) {
        LOG_WARNING("warning: wasm_runtime_malloc with size zero\n");
        /* At lease alloc 1 byte to avoid malloc failed */
        size = 1;
    }

    return wasm_runtime_malloc_internal(size);
}

void *
wasm_runtime_realloc(void *ptr, unsigned int size)
{
    void *ret = wasm_runtime_realloc_internal(ptr, size);

    if (ret) {
        printf("realloc\n");
        free_info(ptr);
    }
    return ret;
}

void
wasm_runtime_free(void *ptr)
{
    wasm_runtime_free_internal(ptr);
    free_info(ptr);
}
