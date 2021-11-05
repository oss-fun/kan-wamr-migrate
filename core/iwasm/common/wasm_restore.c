#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "wasm_runtime_common.h"
#include "../../shared/utils/bh_platform.h"
#include "mem_alloc.h"
#include "../../shared/mem-alloc/ems/ems_gc_internal.h"
#include "wasm_memory.h"
#include "wasm_restore.h"
#include "../interpreter/wasm_interp.h"
#include "../interpreter/wasm_loader.h"
#include "../interpreter/wasm_runtime.h"

static FILE *fp, *gp;
static unsigned long base;
static WASMModule *_module_static;
static WASMModuleInstance *_module_inst_static;
static WASMExecEnv *_exec_env_static;

#define HEADER_RESTORE_BUF(Type) Type *node = (Type *)addr->p_raw

#define HEADER_RESTORE(Type)                                                  \
    for (Type *node = (Type *)addr->p_raw; addr != NULL; addr = addr->list,   \
              node = (addr != NULL) ? (Type *)addr->p_raw : NULL)

#define RESTORE_PTR(attr)                                                     \
    do {                                                                      \
        Pool_Info *_info;                                                     \
        int _p_abs;                                                           \
        fread(&_p_abs, sizeof(int), 1, fp);                                   \
        if (_p_abs == -1) {                                                   \
            attr = NULL;                                                      \
        }                                                                     \
        else {                                                                \
            _info = get_info(_p_abs);                                         \
            attr = _info->p_raw;                                              \
        }                                                                     \
    } while (0)

#define RESTORE_FRAME_PTR(attr)                                               \
    do {                                                                      \
        Pool_Info *_info;                                                     \
        int _p_abs;                                                           \
        fread(&_p_abs, sizeof(int), 1, gp);                                   \
        if (_p_abs == -1) {                                                   \
            attr = NULL;                                                      \
        }                                                                     \
        else {                                                                \
            _info = get_info(_p_abs);                                         \
            attr = _info->p_raw;                                              \
        }                                                                     \
    } while (0)

static void (*restore_data[ERRORT])(Pool_Info *addr);

void
init_restore(void)
{
    if ((fp = fopen("dump.img", "rb")) == NULL) {
        printf("file open error\n");
        exit(1);
    }
    if ((gp = fopen("frame.img", "rb")) == NULL) {
        printf("file open error\n");
        exit(1);
    }

    restore_data[charT] = restore_char;
    restore_data[charTT] = restore_charT;
    restore_data[uint8T] = restore_uint8;
    restore_data[uint16T] = restore_uint16;
    restore_data[uint32T] = restore_uint32;
    restore_data[uint64T] = restore_uint64;
    restore_data[gc_heap_tT] = restore_gc_heap_t;

    restore_data[WASIContextT] = restore_WASIContext;
    restore_data[WASMThreadArgT] = restore_WASMThreadArg;
    restore_data[ExternRefMapNodeT] = restore_ExternRefMapNode;
    restore_data[fd_tableT] = restore_fd_table;
    restore_data[fd_prestatsT] = restore_fd_prestats;
    restore_data[argv_environ_valuesT] = restore_argv_environ_values;
    restore_data[uvwasi_tT] = restore_uvwasi_t;
    restore_data[uvwasi_preopen_tT] = restore_uvwasi_preopen_t;
    restore_data[wasi_iovec_tT] = restore_wasi_iovec_t;
    restore_data[wasi_ciovec_tT] = restore_wasi_ciovec_t;

    restore_data[wasm_val_tT] = restore_wasm_val_t;
    restore_data[wasm_instance_tT] = restore_wasm_instance_t;
    restore_data[wasm_engine_tT] = restore_wasm_engine_t;
    restore_data[wasm_store_tT] = restore_wasm_store_t;
    restore_data[VectorT] = restore_Vector;
    restore_data[wasm_valtype_tT] = restore_wasm_valtype_t;
    restore_data[wasm_functype_tT] = restore_wasm_functype_t;
    restore_data[wasm_valtype_vec_tT] = restore_wasm_valtype_vec_t;
    restore_data[wasm_globaltype_tT] = restore_wasm_globaltype_t;
    restore_data[wasm_tabletype_tT] = restore_wasm_tabletype_t;
    restore_data[wasm_memorytype_tT] = restore_wasm_memorytype_t;
    restore_data[wasm_importtype_tT] = restore_wasm_importtype_t;
    restore_data[wasm_byte_vec_tT] = restore_wasm_byte_vec_t;
    restore_data[wasm_exporttype_tT] = restore_wasm_exporttype_t;
    restore_data[wasm_ref_tT] = restore_wasm_ref_t;
    restore_data[wasm_frame_tT] = restore_wasm_frame_t;
    restore_data[wasm_trap_tT] = restore_wasm_trap_t;
    restore_data[wasm_foreign_tT] = restore_wasm_foreign_t;
    restore_data[wasm_module_ex_tT] = restore_wasm_module_ex_t;
    restore_data[wasm_func_tT] = restore_wasm_func_t;
    restore_data[wasm_global_tT] = restore_wasm_global_t;
    restore_data[wasm_table_tT] = restore_wasm_table_t;
    restore_data[wasm_memory_tT] = restore_wasm_memory_t;
    restore_data[wasm_store_vec_tT] = restore_wasm_store_vec_t;
    restore_data[wasm_module_vec_tT] = restore_wasm_module_vec_t;
    restore_data[wasm_instance_vec_tT] = restore_wasm_instance_vec_t;
    restore_data[wasm_extern_vec_tT] = restore_wasm_extern_vec_t;

    restore_data[WASMRegisteredModuleT] = restore_WASMRegisteredModule;
    restore_data[LoadingModuleT] = restore_LoadingModule;

    restore_data[WASMExecEnvT] = restore_WASMExecEnv;
    restore_data[NativeSymbolsNodeT] = restore_NativeSymbolsNode;
    restore_data[WASMModuleCommonT] = restore_WASMModuleCommon;
    restore_data[WASMModuleInstanceCommonT] = restore_WASMModuleInstanceCommon;
    restore_data[WASMModuleMemConsumptionT] = restore_WASMModuleMemConsumption;
    restore_data[WASMModuleInstMemConsumptionT] =
      restore_WASMModuleInstMemConsumption;
    restore_data[WASMMemoryInstanceCommonT] = restore_WASMMemoryInstanceCommon;
    restore_data[WASMSectionT] = restore_WASMSection;
    restore_data[WASMCApiFrameT] = restore_WASMCApiFrame;
    restore_data[WASMSharedMemNodeT] = restore_WASMSharedMemNode;
    restore_data[WASMModuleT] = restore_WASMModule;
    restore_data[WASMFunctionT] = restore_WASMFunction;
    restore_data[WASMFunctionTT] = restore_WASMFunctionT;
    restore_data[WASMGlobalT] = restore_WASMGlobal;
    restore_data[WASMExportT] = restore_WASMExport;
    restore_data[V128T] = restore_V128;
    restore_data[WASMValueT] = restore_WASMValue;
    restore_data[InitializerExpressionT] = restore_InitializerExpression;
    restore_data[WASMTypeT] = restore_WASMType;
    restore_data[WASMTypeTT] = restore_WASMTypeT;
    restore_data[WASMTableT] = restore_WASMTable;
    restore_data[WASMMemoryT] = restore_WASMMemory;
    restore_data[WASMTableImportT] = restore_WASMTableImport;
    restore_data[WASMMemoryImportT] = restore_WASMMemoryImport;
    restore_data[WASMFunctionImportT] = restore_WASMFunctionImport;
    restore_data[WASMGlobalImportT] = restore_WASMGlobalImport;
    restore_data[WASMImportT] = restore_WASMImport;
    restore_data[WASMTableSegT] = restore_WASMTableSeg;
    restore_data[WASMDataSegT] = restore_WASMDataSeg;
    restore_data[WASMDataSegTT] = restore_WASMDataSegT;
    restore_data[BlockAddrT] = restore_BlockAddr;
    restore_data[WASIArgumentsT] = restore_WASIArguments;
    restore_data[StringNodeT] = restore_StringNode;
    restore_data[BlockTypeT] = restore_BlockType;
    restore_data[WASMBranchBlockT] = restore_WASMBranchBlock;
    restore_data[WASMInterpFrameT] = restore_WASMInterpFrame;
    restore_data[BranchBlockT] = restore_BranchBlock;
    restore_data[BranchBlockPatchT] = restore_BranchBlockPatch;
    restore_data[WASMLoaderContextT] = restore_WASMLoaderContext;
    restore_data[ConstT] = restore_Const;
    restore_data[WASMModuleInstanceT] = restore_WASMModuleInstance;
    restore_data[WASMFunctionInstanceT] = restore_WASMFunctionInstance;
    restore_data[WASMMemoryInstanceT] = restore_WASMMemoryInstance;
    restore_data[WASMMemoryInstanceTT] = restore_WASMMemoryInstanceT;
    restore_data[WASMTableInstanceT] = restore_WASMTableInstance;
    restore_data[WASMTableInstanceTT] = restore_WASMTableInstanceT;
    restore_data[WASMGlobalInstanceT] = restore_WASMGlobalInstance;
    restore_data[WASMExportFuncInstanceT] = restore_WASMExportFuncInstance;

    restore_data[WASMExportGlobInstanceT] = restore_WASMExportGlobInstance;
    restore_data[WASMSubModInstNodeT] = restore_WASMSubModInstNode;
    restore_data[WASMRuntimeFrameT] = restore_WASMRuntimeFrame;
    restore_data[WASMOpcodeT] = restore_WASMOpcode;
    restore_data[WASMMiscEXTOpcodeT] = restore_WASMMiscEXTOpcode;
    restore_data[WASMSimdEXTOpcodeT] = restore_WASMSimdEXTOpcode;
    restore_data[WASMAtomicEXTOpcodeT] = restore_WASMAtomicEXTOpcode;

    restore_data[HashMapElemT] = restore_HashMapElem;
    restore_data[HashMapT] = restore_HashMap;
    restore_data[timer_ctx_tT] = restore_timer_ctx_t;
    restore_data[app_timer_tT] = restore_app_timer_t;
    restore_data[bh_queueT] = restore_bh_queue;
    restore_data[bh_queue_nodeT] = restore_bh_queue_node;

    restore_data[AtomicWaitInfoT] = restore_AtomicWaitInfo;
    restore_data[AtomicWaitNodeT] = restore_AtomicWaitNode;
}

void
restore_internal(void)
{
    Pool_Info *info;
    int type;
    int size;
    int p_abs;
    printf("restore_internal\n");
    while (!feof(fp)) {
        if ((fread(&type, sizeof(int), 1, fp)) == 0) {
            break;
        }
        fread(&size, sizeof(int), 1, fp);
        fread(&p_abs, sizeof(int), 1, fp);

        info = get_info(p_abs);
        (*restore_data[type])(info);
    }
    printf("end restore_internal\n");
}

void
restore_frame_internal(void)
{
    int i;
    Pool_Info *info;
    WASMInterpFrame *frame, *prev = NULL;
    int type;
    int size;
    int p_abs;
    char error_buf[128];

    info = get_root_frame_info();

    while (info) {
        frame = wasm_interp_alloc_frame(_exec_env_static, info->size, prev);
        wasm_exec_env_set_cur_frame(_exec_env_static, frame);
        prev = frame;
        info->p_raw = frame;
        info = info->next;
    }

    while (!feof(gp)) {
        if ((fread(&type, sizeof(int), 1, gp)) == 0) {
            break;
        }
        fread(&size, sizeof(int), 1, gp);
        fread(&p_abs, sizeof(int), 1, gp);
        info = get_frame_info(p_abs);
        restore_WASMInterpFrame(info);
    }

    if (!wasm_runtime_init_wasi(
          (WASMModuleInstanceCommon *)_module_inst_static,
          _module_static->wasi_args.dir_list,
          _module_static->wasi_args.dir_count,
          _module_static->wasi_args.map_dir_list,
          _module_static->wasi_args.map_dir_count,
          _module_static->wasi_args.env, _module_static->wasi_args.env_count,
          _module_static->wasi_args.argv, _module_static->wasi_args.argc,
          _module_static->wasi_args.stdio[0],
          _module_static->wasi_args.stdio[1],
          _module_static->wasi_args.stdio[2], error_buf, sizeof(error_buf))) {
        printf("error init wasi\n");
    }
    else {
        printf("init wasi\n");
    }

    WASMImport *import = _module_static->import_functions;
    for (i = 0; i < _module_static->import_function_count; i++, import++) {
        const char *linked_signature = NULL;
        void *linked_attachment = NULL;
        bool linked_call_conv_raw = false;
        bool is_native_symbol = false;

        WASMFunction *linked_func = wasm_native_resolve_symbol(
          import->u.names.module_name, import->u.names.field_name,
          import->u.function.func_type, &linked_signature, &linked_attachment,
          &linked_call_conv_raw);

        if (linked_func) {
            is_native_symbol = true;
        }else{
            printf("error linked\n");
        }
        WASMFunctionImport *function = &import->u.function;
        function->func_ptr_linked = is_native_symbol ? linked_func : NULL;
        function->signature = linked_signature;
        function->attachment = linked_attachment;
        function->call_conv_raw = linked_call_conv_raw;
    }
    printf("resolve native\n");
}

void
restore_char(Pool_Info *addr) //buf
{
    fread((char *)addr->p_raw, sizeof(char), addr->size, fp);
}

void
restore_charT(Pool_Info *addr)
{
    int i, p_abs;
    Pool_Info *info;

    for (i = 0; i < addr->size; i++) {
        RESTORE_PTR(((char **)addr->p_raw)[i]);
    }
}

void
restore_uint8(Pool_Info *addr)
{
    fread((uint8 *)addr->p_raw, sizeof(uint8), addr->size, fp);
}
void
restore_uint16(Pool_Info *addr)
{
    fread((uint16 *)addr->p_raw, sizeof(uint16), addr->size, fp);
}

void
restore_uint32(Pool_Info *addr)
{
    fread((uint32 *)addr->p_raw, sizeof(uint32), addr->size, fp);
}

void
restore_uint64(Pool_Info *addr)
{
    fread((uint64 *)addr->p_raw, sizeof(uint64), addr->size, fp);
}

void
restore_gc_heap_t(Pool_Info *addr)
{
    int i;
    printf("skip gc_heap_t\n");
    return;
    HEADER_RESTORE(gc_heap_t)
    {
        fwrite(&addr->p_abs, sizeof(int), 1, fp);

        //gc_handle_t heap_id; ==void*
        RESTORE_PTR(node->heap_id);

        //gc_uint8 *base_addr;
        RESTORE_PTR(node->base_addr);

        //gc_size_t current_size; ==uint32
        fwrite(&node->current_size, sizeof(uint32), 1, fp);

        //hmu_normal_list_t kfc_normal_list[HMU_NORMAL_NODE_CNT];
        for (i = 0; i < HMU_NORMAL_NODE_CNT; i++) {
            RESTORE_PTR(node->kfc_normal_list[i].next);
        }

        //hmu_tree_node_t kfc_tree_root;
        fwrite(&node->kfc_tree_root.hmu_header, sizeof(gc_uint32), 1, fp);
        fwrite(&node->kfc_tree_root.size, sizeof(gc_size_t), 1, fp);
        RESTORE_PTR(node->kfc_tree_root.left);
        RESTORE_PTR(node->kfc_tree_root.right);
        RESTORE_PTR(node->kfc_tree_root.parent);

        //bool is_heap_corrupted;
        fwrite(&node->is_heap_corrupted, sizeof(bool), 1, fp);

        //gc_size_t init_size;
        fwrite(&node->init_size, sizeof(gc_size_t), 1, fp);
        //gc_size_t highmark_size;
        fwrite(&node->highmark_size, sizeof(gc_size_t), 1, fp);
        //gc_size_t total_free_size;
        fwrite(&node->total_free_size, sizeof(gc_size_t), 1, fp);
    }
}

void
restore_base_addr(Pool_Info *addr)
{
    // skip
}

void
restore_WASIContext(Pool_Info *addr)
{
    printf("skip WASIContext\n");
    return;
#if WASM_ENABLE_LIBC_WASI != 0
    HEADER_RESTORE(WASIContext)
    {
        fwrite(&addr->p_abs, sizeof(int), 1, fp);

        //struct fd_table *curfds;
        //struct fd_prestats *prestats;
        //struct argv_environ_values *argv_environ;
        {
            //   const char *argv_buf;
            RESTORE_PTR(node->argv_environ->argv_buf);
            //   size_t argv_buf_size;
            fwrite(&node->argv_environ->argv_buf_size, sizeof(uint64), 1, fp);
            //   char **argv_list;
            RESTORE_PTR(node->argv_environ->argv_list);
            //   size_t argc;
            fwrite(&node->argv_environ->argc, sizeof(uint64), 1, fp);
            //   char *environ_buf;
            RESTORE_PTR(node->argv_environ->environ_buf);
            //   size_t environ_buf_size;
            fwrite(&node->argv_environ->environ_buf_size, sizeof(uint64), 1,
                   fp);
            //   char **environ_list;
            RESTORE_PTR(node->argv_environ->environ_list);
            //   size_t environ_count;
            fwrite(&node->argv_environ->environ_count, sizeof(uint64), 1, fp);
        }

        //char *argv_buf;
        RESTORE_PTR(node->argv_buf);
        //char **argv_list;
        RESTORE_PTR(node->argv_list);
        //char *env_buf;
        RESTORE_PTR(node->env_buf);
        //char **env_list;
        RESTORE_PTR(node->env_list);
    }
#endif
}

void
restore_WASMThreadArg(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
    /*
    HEADER_RESTORE(WASMThreadArg);

    //WASMExecEnv *new_exec_env;
    RESTORE_PTR(node->new_exec_env);
    //wasm_thread_callback_t callback;==function pointer
    RESTORE_PTR(node->callback);
    //void *arg;
    RESTORE_PTR(node->arg);
    */
}

void
restore_ExternRefMapNode(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}

void
restore_fd_table(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
    // skip
    //struct rwlock lock;==pthread_rwlock_t object;
    //struct fd_entry *entries;
    //size_t size;
    //size_t used;
}
void
restore_fd_prestats(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
    // skip
    //struct rwlock lock;==pthread_rwlock_t object;
    //struct fd_prestat *prestats;
    //size_t size;
    //size_t used;
}
void
restore_argv_environ_values(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
    //const char *argv_buf;
    //size_t argv_buf_size;
    //char **argv_list;
    //size_t argc;
    //char *environ_buf;
    //size_t environ_buf_size;
    //char **environ_list;
    //size_t environ_count;
}

void
restore_uvwasi_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_uvwasi_preopen_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}

void
restore_wasm_val_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
    HEADER_RESTORE(wasm_val_t)
    {
        fwrite(&addr->p_abs, sizeof(int), 1, fp);

        //wasm_valkind_t kind;==uint8_t
        fwrite(&node->kind, sizeof(uint8_t), 1, fp);

        /*union {
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
    struct wasm_ref_t* ref;
    } of;*/
        switch (node->kind) {
            case WASM_I32:
            case WASM_I64:
            case WASM_F32:
            case WASM_F64:
                fwrite(&node->of, sizeof(int64_t), 1, fp);
                break;
            case WASM_ANYREF:
            case WASM_FUNCREF:
                RESTORE_PTR(node->of.ref);
                break;
            default:
                break;
        }
    }
}

/*
    *wasm-c-api
*/

void
restore_wasm_instance_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_engine_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_store_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_Vector(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_valtype_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_functype_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_valtype_vec_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_globaltype_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_tabletype_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_memorytype_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_importtype_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_byte_vec_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_exporttype_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_ref_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_frame_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_trap_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_foreign_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_module_ex_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_func_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_global_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_table_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_memory_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}

void
restore_wasm_store_vec_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_module_vec_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_instance_vec_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_wasm_extern_vec_t(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_WASMRegisteredModule(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_LoadingModule(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
/* 
    *end wasm-c-api
*/

void
restore_WASMExecEnv(Pool_Info *addr)
{
    int i, j, k, p_abs;
    Pool_Info *info;
    HEADER_RESTORE_BUF(WASMExecEnv);

    _exec_env_static = node;

    //struct WASMExecEnv *next;
    RESTORE_PTR(node->next);

    //struct WASMExecEnv *prev;
    RESTORE_PTR(node->prev);

    //struct WASMModuleInstanceCommon *module_inst;
    RESTORE_PTR(node->module_inst);

    //uint8 *native_stack_boundary;
    //RESTORE_PTR(node->native_stack_boundary);

    /*
    union {
        uint32 flags;
        uintptr_t __padding__;
    } suspend_flags;*/

    /*
    union {
        uint32 boundary;
        uintptr_t __padding__;
    } aux_stack_boundary;*/

    /*
    union {
        uint32 bottom;
        uintptr_t __padding__;
    } aux_stack_bottom;*/

    //void *attachment; probably always==NULL
    RESTORE_PTR(node->attachment);

    //void *user_data;??
    //RESTORE_PTR(node->user_data);

    //struct WASMInterpFrame *cur_frame;
    //RESTORE_PTR(node->cur_frame);

    //korp_tid handle;

#if WASM_ENABLE_INTERP != 0 && WASM_ENABLE_FAST_INTERP == 0
    //WASMModuleInstance *module_inst = (WASMModuleInstance *)node->module_inst;
    //BlockAddr block_addr_cache[BLOCK_ADDR_CACHE_SIZE][BLOCK_ADDR_CONFLICT_SIZE];
    for (i = 0; i < BLOCK_ADDR_CACHE_SIZE; i++) {
        for (j = 0; j < BLOCK_ADDR_CONFLICT_SIZE; j++) {
            uint8 *code;
            uint32 ip;

            fread(&p_abs, sizeof(int), 1, fp);
            if (p_abs == -1) { // code is null
                node->block_addr_cache[i][j].start_addr = NULL;
                node->block_addr_cache[i][j].else_addr = NULL;
                node->block_addr_cache[i][j].end_addr = NULL;
                continue;
            }
            info = get_info(p_abs);
            code = info->p_raw;

            // const uint8 *start_addr;
            //ip = start_addr - function->code;
            fread(&ip, sizeof(uint32), 1, fp);
            node->block_addr_cache[i][j].start_addr = ip + code;

            // uint8 *else_addr;
            //ip = else_addr - function->code;
            fread(&ip, sizeof(uint32), 1, fp);
            node->block_addr_cache[i][j].else_addr = ip + code;

            // uint8 *end_addr;
            //ip = end_addr - function->code;
            fread(&ip, sizeof(uint32), 1, fp);
            node->block_addr_cache[i][j].end_addr = ip + code;
        }
    }
#endif
    /*
#ifdef OS_ENABLE_HW_BOUND_CHECK
    WASMJmpBuf *jmpbuf_stack_top;
#endif
*/

    //uint32 wasm_stack_size;
    fread(&node->wasm_stack_size, sizeof(uint32), 1, fp);
    /*
    union {
        uint64 __make_it_8_byte_aligned_;

        struct {
            uint8 *top_boundary;

            uint8 *top;

            uint8 bottom[1];
        } s;
    } wasm_stack;*/
    node->wasm_stack.s.top = node->wasm_stack.s.bottom;
    node->wasm_stack.s.top_boundary =
      node->wasm_stack.s.bottom + node->wasm_stack_size;
    //RESTORE_PTR(node->wasm_stack.s.top_boundary);
    //RESTORE_PTR(node->wasm_stack.s.top);
    //fwrite(node->wasm_stack.s.bottom, sizeof(uint8), addr->size, fp);
}

void
restore_NativeSymbolsNode(Pool_Info *addr)
{
    printf("skip NativeSymbolNode\n");
    return;
    HEADER_RESTORE(NativeSymbolsNode)
    {
        fwrite(&addr->p_abs, sizeof(int), 1, fp);

        //struct NativeSymbolsNode *next;
        RESTORE_PTR(node->next);
        //const char *module_name; 外部

        //NativeSymbol *native_symbols; 付け替え

        //uint32 n_native_symbols; 付け替え

        //bool call_conv_raw; wasm_runtime_full_initからだとfalse
    }
}

void
restore_WASMModuleCommon(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
    return;
    HEADER_RESTORE_BUF(WASMModuleCommon);

    //uint32 module_type;
    fwrite(&node->module_type, sizeof(uint32), 1, fp);
    //uint8 module_data[1];
    fwrite(node->module_data, sizeof(uint8), 1, fp);
}
void
restore_WASMModuleInstanceCommon(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
    HEADER_RESTORE_BUF(WASMModuleInstanceCommon);

    //uint32 module_type;
    fwrite(&node->module_type, sizeof(uint32), 1, fp);
    //uint8 module_inst_data[1];
    fwrite(node->module_inst_data, sizeof(uint8), 1, fp);
}

void
restore_WASMModuleMemConsumption(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(WASMModuleMemConsumption)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        //uint32 total_size;
        fread(&node->total_size, sizeof(uint32), 1, fp);
        //uint32 module_struct_size;
        fread(&node->module_struct_size, sizeof(uint32), 1, fp);
        //uint32 types_size;
        fread(&node->types_size, sizeof(uint32), 1, fp);
        //uint32 imports_size;
        fread(&node->imports_size, sizeof(uint32), 1, fp);
        //uint32 functions_size;
        fread(&node->functions_size, sizeof(uint32), 1, fp);
        //uint32 tables_size;
        fread(&node->tables_size, sizeof(uint32), 1, fp);
        //uint32 memories_size;
        fread(&node->memories_size, sizeof(uint32), 1, fp);
        //uint32 globals_size;
        fread(&node->globals_size, sizeof(uint32), 1, fp);
        //uint32 exports_size;
        fread(&node->exports_size, sizeof(uint32), 1, fp);
        //uint32 table_segs_size;
        fread(&node->table_segs_size, sizeof(uint32), 1, fp);
        //uint32 data_segs_size;
        fread(&node->data_segs_size, sizeof(uint32), 1, fp);
        //uint32 const_strs_size;
        fread(&node->const_strs_size, sizeof(uint32), 1, fp);
        /*
#if WASM_ENABLE_AOT != 0
    uint32 aot_code_size;
#endif*/
    }
}

void
restore_WASMModuleInstMemConsumption(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(WASMModuleInstMemConsumption)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        //uint32 total_size;
        fread(&node->total_size, sizeof(uint32), 1, fp);
        //uint32 module_inst_struct_size;
        fread(&node->module_inst_struct_size, sizeof(uint32), 1, fp);
        //uint32 memories_size;
        fread(&node->memories_size, sizeof(uint32), 1, fp);
        //uint32 app_heap_size;
        fread(&node->app_heap_size, sizeof(uint32), 1, fp);
        //uint32 tables_size;
        fread(&node->tables_size, sizeof(uint32), 1, fp);
        //uint32 globals_size;
        fread(&node->globals_size, sizeof(uint32), 1, fp);
        //uint32 functions_size;
        fread(&node->functions_size, sizeof(uint32), 1, fp);
        //uint32 exports_size;
        fread(&node->exports_size, sizeof(uint32), 1, fp);
    }
}
void
restore_WASMMemoryInstanceCommon(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
    HEADER_RESTORE_BUF(WASMMemoryInstanceCommon);

    //uint32 module_type;
    fwrite(&node->module_type, sizeof(uint32), 1, fp);
    //uint8 memory_inst_data[1];
    fwrite(node->memory_inst_data, sizeof(uint8), 1, fp);
}
void
restore_WASMSection(Pool_Info *addr) //==wasm_section_t
{
    int p_abs;
    Pool_Info *info;
    HEADER_RESTORE(WASMSection)
    {
        // printf("WASMSection\n");
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        //struct wasm_section_t *next;
        RESTORE_PTR(node->next);

        //int section_type;
        fread(&node->section_type, sizeof(int), 1, fp);

        //uint8_t *section_body; 外部wasmファイル => loader_mallocでuint8を確保
        RESTORE_PTR(node->section_body);

        //uint32_t section_body_size;
        fread(&node->section_body_size, sizeof(uint32_t), 1, fp);
    }
}
void
restore_WASMCApiFrame(Pool_Info *addr) //==wasm_frame_t
{
    printf("unsupported:%d\n", addr->type);
    exit(1);
    HEADER_RESTORE(WASMCApiFrame)
    {
        fwrite(&addr->p_abs, sizeof(int), 1, fp);

        //void *instance;
        RESTORE_PTR(node->instance);
        //uint32 module_offset;
        fwrite(&node->module_offset, sizeof(uint32), 1, fp);
        //uint32 func_index;
        fwrite(&node->func_index, sizeof(uint32), 1, fp);
        //uint32 func_offset;
        fwrite(&node->func_offset, sizeof(uint32), 1, fp);
    }
}

void
restore_WASMSharedMemNode(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}

void
restore_WASMModule(Pool_Info *addr) // 要チェック
{
    int p_abs;
    Pool_Info *info;
    _module_static = addr->p_raw;
    HEADER_RESTORE(WASMModule)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        //uint32 module_type;
        fread(&node->module_type, sizeof(uint32), 1, fp);
        //uint32 type_count;
        fread(&node->type_count, sizeof(uint32), 1, fp);
        //uint32 import_count;
        fread(&node->import_count, sizeof(uint32), 1, fp);
        //uint32 function_count;
        fread(&node->function_count, sizeof(uint32), 1, fp);
        //uint32 table_count;
        fread(&node->table_count, sizeof(uint32), 1, fp);
        //uint32 memory_count;
        fread(&node->memory_count, sizeof(uint32), 1, fp);
        //uint32 global_count;
        fread(&node->global_count, sizeof(uint32), 1, fp);
        //uint32 export_count;
        fread(&node->export_count, sizeof(uint32), 1, fp);
        //uint32 table_seg_count;
        fread(&node->table_seg_count, sizeof(uint32), 1, fp);
        //uint32 data_seg_count;
        fread(&node->data_seg_count, sizeof(uint32), 1, fp);

        //uint32 import_function_count;
        fread(&node->import_function_count, sizeof(uint32), 1, fp);
        //uint32 import_table_count;
        fread(&node->import_table_count, sizeof(uint32), 1, fp);
        //uint32 import_memory_count;
        fread(&node->import_memory_count, sizeof(uint32), 1, fp);
        //uint32 import_global_count;
        fread(&node->import_global_count, sizeof(uint32), 1, fp);

        //WASMImport *import_functions;
        RESTORE_PTR(node->import_functions);
        //WASMImport *import_tables;
        RESTORE_PTR(node->import_tables);
        //WASMImport *import_memories;
        RESTORE_PTR(node->import_memories);
        //WASMImport *import_globals;
        RESTORE_PTR(node->import_globals);

        //WASMType **types;
        RESTORE_PTR(node->types);
        //WASMImport *imports;
        RESTORE_PTR(node->imports);
        //WASMFunction **functions;
        RESTORE_PTR(node->functions);
        //WASMTable *tables;
        RESTORE_PTR(node->tables);
        // WASMMemory *memories;
        RESTORE_PTR(node->memories);
        // WASMGlobal *globals;
        RESTORE_PTR(node->globals);
        // WASMExport *exports;
        RESTORE_PTR(node->exports);
        // WASMTableSeg *table_segments;
        RESTORE_PTR(node->table_segments);
        // WASMDataSeg **data_segments;
        RESTORE_PTR(node->data_segments);

        // uint32 start_function;
        fread(&node->start_function, sizeof(uint32), 1, fp);

        //uint32 aux_data_end_global_index;
        fread(&node->aux_data_end_global_index, sizeof(uint32), 1, fp);
        //uint32 aux_data_end;
        fread(&node->aux_data_end, sizeof(uint32), 1, fp);

        //uint32 aux_heap_base_global_index;
        fread(&node->aux_heap_base_global_index, sizeof(uint32), 1, fp);
        //uint32 aux_heap_base;
        fread(&node->aux_heap_base, sizeof(uint32), 1, fp);

        //uint32 aux_stack_top_global_index;
        fread(&node->aux_stack_top_global_index, sizeof(uint32), 1, fp);
        //uint32 aux_stack_bottom;
        fread(&node->aux_stack_bottom, sizeof(uint32), 1, fp);
        //uint32 aux_stack_size;
        fread(&node->aux_stack_size, sizeof(uint32), 1, fp);

        //uint32 malloc_function;
        fread(&node->malloc_function, sizeof(uint32), 1, fp);
        //uint32 free_function;
        fread(&node->free_function, sizeof(uint32), 1, fp);

        //uint32 retain_function;
        fread(&node->retain_function, sizeof(uint32), 1, fp);

        //bool possible_memory_grow;
        fread(&node->possible_memory_grow, sizeof(bool), 1, fp);

        //StringList const_str_list;
        RESTORE_PTR(node->const_str_list);

#if WASM_ENABLE_LIBC_WASI != 0
        // init_wasi(node);

        //WASIArguments wasi_args;
        /*{
            // const char **dir_list;
            RESTORE_PTR(node->wasi_args.dir_list);
            // uint32 dir_count;
            fread(&node->wasi_args.dir_count, sizeof(uint32), 1, fp);
            // const char **map_dir_list;
            RESTORE_PTR(node->wasi_args.map_dir_list);
            // uint32 map_dir_count;
            fread(&node->wasi_args.dir_count, sizeof(uint32), 1, fp);
            // const char **env;
            RESTORE_PTR(node->wasi_args.env);
            // uint32 env_count;
            fread(&node->wasi_args.env_count, sizeof(uint32), 1, fp);
            // char **argv;
            RESTORE_PTR(node->wasi_args.argv);
            // uint32 argc;
            fread(&node->wasi_args.argc, sizeof(uint32), 1, fp);
            // int stdio[3];
            fread(node->wasi_args.stdio, sizeof(int), 3, fp);
        }*/
        //bool is_wasi_module;
        fread(&node->is_wasi_module, sizeof(bool), 1, fp);
#endif
    }
}

void
restore_WASMFunction(Pool_Info *addr) //要チェック
{
    HEADER_RESTORE_BUF(WASMFunction);

#if WASM_ENABLE_CUSTOM_NAME_SECTION != 0
    char *field_name;
#endif
    // WASMType *func_type;
    RESTORE_PTR(node->func_type);

    // uint32 local_count;
    fread(&node->local_count, sizeof(uint32), 1, fp);

    // uint8 *local_types;
    //RESTORE_PTR(node->local_types);
    node->local_types = (uint8 *)node + sizeof(WASMFunction);

    // uint16 param_cell_num;
    fread(&node->param_cell_num, sizeof(uint16), 1, fp);
    // uint16 ret_cell_num;
    fread(&node->ret_cell_num, sizeof(uint16), 1, fp);
    // uint16 local_cell_num;
    fread(&node->local_cell_num, sizeof(uint16), 1, fp);
    // uint16 *local_offsets; probably Ok
    RESTORE_PTR(node->local_offsets);

    //uint32 max_stack_cell_num;
    fread(&node->max_stack_cell_num, sizeof(uint32), 1, fp);
    //uint32 max_block_num;
    fread(&node->max_block_num, sizeof(uint32), 1, fp);

    //bool has_op_memory_grow;
    fread(&node->has_op_memory_grow, sizeof(bool), 1, fp);
    //bool has_op_func_call;
    fread(&node->has_op_func_call, sizeof(bool), 1, fp);
    // uint32 code_size;
    fread(&node->code_size, sizeof(uint32), 1, fp);
    // uint8 *code; //loader_mallocでcodeをcopy
    RESTORE_PTR(node->code);

    // +local_count // sizeof(WASMFunction) + (uint64)local_count
    fread(node->local_types, sizeof(uint8), node->local_count, fp);
}

void
restore_WASMFunctionT(Pool_Info *addr)
{
    int i, p_abs;
    Pool_Info *info;

    for (i = 0; i < addr->size; i++) {
        RESTORE_PTR(((WASMFunction **)addr->p_raw)[i]);
    }
}

void
restore_WASMGlobal(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(WASMGlobal)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        //uint8 type;
        fread(&node->type, sizeof(uint8), 1, fp);
        //bool is_mutable;
        fread(&node->is_mutable, sizeof(bool), 1, fp);
        //InitializerExpression init_expr;
        {
            //uint8 init_expr_type;
            fread(&node->init_expr.init_expr_type, sizeof(uint8), 1, fp);
            //union WASMValue u;
            {
                switch (node->init_expr.init_expr_type) {
                    /*
                int32 i32;
                uint32 u32;
                int64 i64;
                uint64 u64;
                float32 f32;
                float64 f64;
                */
                    case INIT_EXPR_TYPE_I32_CONST:
                    case INIT_EXPR_TYPE_I64_CONST:
                    case INIT_EXPR_TYPE_F32_CONST:
                    case INIT_EXPR_TYPE_F64_CONST:
                        fread(&node->init_expr.u.i64, sizeof(uint64), 1, fp);
                        break;

                    //V128 v128;
                    case INIT_EXPR_TYPE_V128_CONST:
                        /*
                        int8 i8x16[16];
                        int16 i16x8[8];
                        int32 i32x8[4];
                        int64 i64x2[2];
                        float32 f32x4[4];
                        float64 f64x2[2];
                        */
                        fread(&node->init_expr.u.v128.f32x4, sizeof(float64),
                              2, fp);
                        break;

                    //uint32 ref_index;
                    case INIT_EXPR_TYPE_FUNCREF_CONST:
                    case INIT_EXPR_TYPE_REFNULL_CONST:
                        fread(&node->init_expr.u.ref_index, sizeof(uint32), 1,
                              fp);
                        break;

                    //uint32 global_index;
                    case INIT_EXPR_TYPE_GET_GLOBAL:
                        fread(&node->init_expr.u.global_index, sizeof(uint32),
                              1, fp);

                        break;

                    //uintptr_t addr;
                    default:
                        fread(&node->init_expr.u.addr, sizeof(uintptr_t), 1,
                              fp);
                        break;
                }
            }
        }
    }
}

void
restore_WASMExport(Pool_Info *addr)
{
    int p_abs;
    StringNode *str_node;
    HEADER_RESTORE(WASMExport)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        //char *name; stringnodeの途中を指す
        RESTORE_PTR(str_node);
        node->name = (uint8 *)str_node + sizeof(StringNode);

        //uint8 kind;
        fread(&node->kind, sizeof(uint8), 1, fp);
        //uint32 index;
        fread(&node->index, sizeof(uint32), 1, fp);
    }
}

void
restore_V128(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(V128)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        fread(&node->f64x2, sizeof(float64), 2, fp);
    }
}

void
restore_WASMValue(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(WASMValue)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }
        fread(&node->v128.f32x4, sizeof(float64), 2, fp);
    }
}
void
restore_InitializerExpression(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(InitializerExpression)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        //uint8 init_expr_type;
        fread(&node->init_expr_type, sizeof(uint8), 1, fp);
        //union WASMValue u;
        {
            switch (node->init_expr_type) {
                /*
                int32 i32;
                uint32 u32;
                int64 i64;
                uint64 u64;
                float32 f32;
                float64 f64;
                */
                case INIT_EXPR_TYPE_I32_CONST:
                case INIT_EXPR_TYPE_I64_CONST:
                case INIT_EXPR_TYPE_F32_CONST:
                case INIT_EXPR_TYPE_F64_CONST:
                    fread(&node->u.i64, sizeof(uint64), 1, fp);
                    break;

                //V128 v128;
                case INIT_EXPR_TYPE_V128_CONST:
                    /*
                        int8 i8x16[16];
                        int16 i16x8[8];
                        int32 i32x8[4];
                        int64 i64x2[2];
                        float32 f32x4[4];
                        float64 f64x2[2];
                        */
                    fread(&node->u.v128.f32x4, sizeof(float64), 2, fp);
                    break;

                //uint32 ref_index;
                case INIT_EXPR_TYPE_FUNCREF_CONST:
                case INIT_EXPR_TYPE_REFNULL_CONST:
                    fread(&node->u.ref_index, sizeof(uint32), 1, fp);
                    break;

                //uint32 global_index;
                case INIT_EXPR_TYPE_GET_GLOBAL:
                    fread(&node->u.global_index, sizeof(uint32), 1, fp);
                    break;

                //uintptr_t addr;
                default:
                    fread(&node->u.addr, sizeof(uintptr_t), 1, fp);
                    break;
            }
        }
    }
}

void
restore_WASMType(Pool_Info *addr)
{
    HEADER_RESTORE_BUF(WASMType);

    // uint16 param_count;
    fread(&node->param_count, sizeof(uint16), 1, fp);
    // uint16 result_count;
    fread(&node->result_count, sizeof(uint16), 1, fp);
    // uint16 param_cell_num;
    fread(&node->param_cell_num, sizeof(uint16), 1, fp);
    // uint16 ret_cell_num;
    fread(&node->ret_cell_num, sizeof(uint16), 1, fp);
    // uint8 types[1]; offsetof(WASMType, types) + sizeof(uint8) * (uint64)(param_count + result_count);
    fread(node->types, sizeof(uint8), node->param_count + node->result_count,
          fp);
}

void
restore_WASMTypeT(Pool_Info *addr)
{
    int i, p_abs;
    Pool_Info *info;

    for (i = 0; i < addr->size; i++) {
        RESTORE_PTR(((WASMType **)addr->p_raw)[i]);
    }
}

void
restore_WASMTable(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(WASMTable)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        //uint8 elem_type;
        fread(&node->elem_type, sizeof(uint8), 1, fp);
        //uint32 flags;
        fread(&node->flags, sizeof(uint32), 1, fp);
        //uint32 init_size;
        fread(&node->init_size, sizeof(uint32), 1, fp);
        // uint32 max_size;
        fread(&node->max_size, sizeof(uint32), 1, fp);
        // bool possible_grow;
        fread(&node->possible_grow, sizeof(bool), 1, fp);
    }
}
void
restore_WASMMemory(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(WASMMemory)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        // uint32 flags;
        fread(&node->flags, sizeof(uint32), 1, fp);
        // uint32 num_bytes_per_page;
        fread(&node->num_bytes_per_page, sizeof(uint32), 1, fp);
        // uint32 init_page_count;
        fread(&node->init_page_count, sizeof(uint32), 1, fp);
        // uint32 max_page_count;
        fread(&node->max_page_count, sizeof(uint32), 1, fp);
    }
}

/*
for (i = 0; i < addr->size; i++, node = (*)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof();
        fwrite(&p_abs, sizeof(int), 1, fp);
*/
void
restore_WASMTableImport(Pool_Info *addr)
{
    // maybe skip
}
void
restore_WASMMemoryImport(Pool_Info *addr)
{
    // maybe skip
}
void
restore_WASMFunctionImport(Pool_Info *addr)
{
    // maybe skip
}
void
restore_WASMGlobalImport(Pool_Info *addr)
{
    // maybe skip
}

void
restore_WASMImport(Pool_Info *addr)
{
    int p_abs,i;
    StringNode *str_node;
    HEADER_RESTORE(WASMImport)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        // uint8 kind;
        fread(&node->kind, sizeof(uint8), 1, fp);

        // node->u.names.module_name
        RESTORE_PTR(str_node);
        node->u.names.module_name = ((char *)str_node) + sizeof(StringNode);

        // node->u.names.field_name
        RESTORE_PTR(str_node);
        node->u.names.field_name = ((char *)str_node) + sizeof(StringNode);

        /*union {
            WASMFunctionImport function;
            WASMTableImport table;
            WASMMemoryImport memory;
            WASMGlobalImport global;
            struct {
                char *module_name;
                char *field_name;
            } names;
        } u;*/
        switch (node->kind) {
            case IMPORT_KIND_FUNC:
                // WASMType *func_type;
                RESTORE_PTR(node->u.function.func_type);

                // void *func_ptr_linked;
                /* restore時に再設定 */

                // const char *signature;
                //RESTORE_PTR(node->u.function.signature); // 要チェック

                // void *attachment;
                //RESTORE_PTR(node->u.function.attachment); // 要チェック

                // bool call_conv_raw;
                fread(&node->u.function.call_conv_raw, sizeof(bool), 1, fp);

                //bool call_conv_wasm_c_api;
                fread(&node->u.function.call_conv_wasm_c_api, sizeof(bool), 1,
                      fp);
                //bool wasm_c_api_with_env;
                fread(&node->u.function.wasm_c_api_with_env, sizeof(bool), 1,
                      fp);
                break;

            case IMPORT_KIND_TABLE:
                // uint8 elem_type;
                fread(&node->u.table.elem_type, sizeof(uint8), 1, fp);
                // uint32 flags;
                fread(&node->u.table.flags, sizeof(uint32), 1, fp);
                // uint32 init_size;
                fread(&node->u.table.init_size, sizeof(uint32), 1, fp);
                // uint32 max_size;
                fread(&node->u.table.max_size, sizeof(uint32), 1, fp);
                // bool possible_grow;
                fread(&node->u.table.possible_grow, sizeof(bool), 1, fp);
                break;

            case IMPORT_KIND_MEMORY:
                // uint32 flags;
                fread(&node->u.memory.flags, sizeof(uint32), 1, fp);
                // uint32 num_bytes_per_page;
                fread(&node->u.memory.num_bytes_per_page, sizeof(uint32), 1,
                      fp);
                // uint32 init_page_count;
                fread(&node->u.memory.init_page_count, sizeof(uint32), 1, fp);
                // uint32 max_page_count;
                fread(&node->u.memory.max_page_count, sizeof(uint32), 1, fp);
                break;

            case IMPORT_KIND_GLOBAL:
                // uint8 type;
                fread(&node->u.global.type, sizeof(uint8), 1, fp);
                // bool is_mutable;
                fread(&node->u.global.is_mutable, sizeof(bool), 1, fp);
                // WASMValue global_data_linked; /* global data after linked =>???*/
                fread(&node->u.global.global_data_linked.v128.f32x4,
                      sizeof(float64), 2, fp);
                // bool is_linked;
                fread(&node->u.global.is_linked, sizeof(bool), 1, fp);
                break;

            default:
                break;
        }
    }
}

void
restore_WASMTableSeg(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(WASMTableSeg)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        // uint32 mode;
        fread(&node->mode, sizeof(uint32), 1, fp);
        // uint32 elem_type;
        fread(&node->elem_type, sizeof(uint32), 1, fp);
        // bool is_dropped;
        fread(&node->is_dropped, sizeof(bool), 1, fp);
        // uint32 table_index;
        fread(&node->table_index, sizeof(uint32), 1, fp);

        // InitializerExpression base_offset;
        {
            //uint8 init_expr_type;
            fread(&node->base_offset.init_expr_type, sizeof(uint8), 1, fp);
            //union WASMValue u;
            {
                switch (node->base_offset.init_expr_type) {
                    /*
                int32 i32;
                uint32 u32;
                int64 i64;
                uint64 u64;
                float32 f32;
                float64 f64;
                */
                    case INIT_EXPR_TYPE_I32_CONST:
                    case INIT_EXPR_TYPE_I64_CONST:
                    case INIT_EXPR_TYPE_F32_CONST:
                    case INIT_EXPR_TYPE_F64_CONST:
                        fread(&node->base_offset.u.i64, sizeof(uint64), 1, fp);
                        break;

                    //V128 v128;
                    case INIT_EXPR_TYPE_V128_CONST:
                        /*
                        int8 i8x16[16];
                        int16 i16x8[8];
                        int32 i32x8[4];
                        int64 i64x2[2];
                        float32 f32x4[4];
                        float64 f64x2[2];
                        */
                        fread(&node->base_offset.u.v128.f32x4, sizeof(float64),
                              2, fp);
                        break;

                    //uint32 ref_index;
                    case INIT_EXPR_TYPE_FUNCREF_CONST:
                    case INIT_EXPR_TYPE_REFNULL_CONST:
                        fread(&node->base_offset.u.ref_index, sizeof(uint32),
                              1, fp);
                        break;

                    //uint32 global_index;
                    case INIT_EXPR_TYPE_GET_GLOBAL:
                        fread(&node->base_offset.u.global_index,
                              sizeof(uint32), 1, fp);
                        break;

                    //uintptr_t addr;
                    default:
                        fread(&node->base_offset.u.addr, sizeof(uintptr_t), 1,
                              fp);
                        break;
                }
            }
        }

        // uint32 function_count;
        fread(&node->function_count, sizeof(uint32), 1, fp);
        // uint32 *func_indexes;
        RESTORE_PTR(node->func_indexes);
    }
}

void
restore_WASMDataSeg(Pool_Info *addr)
{
    int p_abs;
    printf("WASMDataSeg\n");
    HEADER_RESTORE(WASMDataSeg)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        // uint32 memory_index;
        fread(&node->memory_index, sizeof(uint32), 1, fp);
        // InitializerExpression base_offset;
        {
            //uint8 init_expr_type;
            fread(&node->base_offset.init_expr_type, sizeof(uint8), 1, fp);
            //union WASMValue u;
            {
                switch (node->base_offset.init_expr_type) {
                    /*
                int32 i32;
                uint32 u32;
                int64 i64;
                uint64 u64;
                float32 f32;
                float64 f64;
                */
                    case INIT_EXPR_TYPE_I32_CONST:
                    case INIT_EXPR_TYPE_I64_CONST:
                    case INIT_EXPR_TYPE_F32_CONST:
                    case INIT_EXPR_TYPE_F64_CONST:
                        fread(&node->base_offset.u.i64, sizeof(uint64), 1, fp);
                        break;

                    //V128 v128;
                    case INIT_EXPR_TYPE_V128_CONST:
                        /*
                        int8 i8x16[16];
                        int16 i16x8[8];
                        int32 i32x8[4];
                        int64 i64x2[2];
                        float32 f32x4[4];
                        float64 f64x2[2];
                        */
                        fread(&node->base_offset.u.v128.f32x4, sizeof(float64),
                              2, fp);
                        break;

                    //uint32 ref_index;
                    case INIT_EXPR_TYPE_FUNCREF_CONST:
                    case INIT_EXPR_TYPE_REFNULL_CONST:
                        fread(&node->base_offset.u.ref_index, sizeof(uint32),
                              1, fp);
                        break;

                    //uint32 global_index;
                    case INIT_EXPR_TYPE_GET_GLOBAL:
                        fread(&node->base_offset.u.global_index,
                              sizeof(uint32), 1, fp);
                        break;

                    //uintptr_t addr;
                    default:
                        fread(&node->base_offset.u.addr, sizeof(uintptr_t), 1,
                              fp);
                        break;
                }
            }
        }

        // uint32 data_length;
        fread(&node->data_length, sizeof(uint32), 1, fp);
#if WASM_ENABLE_BULK_MEMORY != 0
        bool is_passive;
#endif
        // uint8 *data;
        RESTORE_PTR(node->data);
    }
}

void
restore_WASMDataSegT(Pool_Info *addr)
{
    int i, p_abs;
    Pool_Info *info;

    for (i = 0; i < addr->size; i++) {
        RESTORE_PTR(((WASMDataSeg **)addr->p_raw)[i]);
    }
}
void
restore_BlockAddr(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
    exit(1);
}
void
restore_WASIArguments(Pool_Info *addr)
{
    // maybe skip
    printf("unsupported:%d\n", addr->type);
    exit(1);
    const char **dir_list;
    uint32 dir_count;
    const char **map_dir_list;
    uint32 map_dir_count;
    const char **env;
    uint32 env_count;
    char **argv;
    uint32 argc;
    int stdio[3];
}

void
restore_StringNode(Pool_Info *addr)
{
    int size;
    HEADER_RESTORE_BUF(StringNode);

    //size
    fread(&size, sizeof(int), 1, fp);
    if (addr->size != size) {
        printf("StringNode size error\n");
        exit(1);
    }
    //struct StringNode *next;
    RESTORE_PTR(node->next);

    node->str = ((char *)node) + sizeof(StringNode);
    //char *str;
    fread(node->str, sizeof(char), size, fp);
}
void
restore_BlockType(Pool_Info *addr)
{
    // skip
}
void
restore_WASMBranchBlock(Pool_Info *addr) //要チェック アロケートはされない
{
    printf("skip WASMBranchBlock\n");
    HEADER_RESTORE(WASMBranchBlock)
    {
        fwrite(&addr->p_abs, sizeof(int), 1, fp);

        // uint8 *begin_addr;
        RESTORE_PTR(node->begin_addr);
        // uint8 *target_addr;
        RESTORE_PTR(node->target_addr);
        // uint32 *frame_sp;
        RESTORE_PTR(node->frame_sp);
        // uint32 cell_num;
        fwrite(&node->cell_num, sizeof(uint32), 1, fp);
    }
}
void
restore_BranchBlockPatch(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}

void
restore_WASMInterpFrame(Pool_Info *addr)
{
    HEADER_RESTORE_BUF(WASMInterpFrame);

    //struct WASMInterpFrame *prev_frame;
    //RESTORE_FRAME_PTR(node->prev_frame);

    //struct WASMFunctionInstance *function;
    RESTORE_FRAME_PTR(node->function);

    //uint8 *ip;
    if (node->function != NULL) {
        uint8 *code = wasm_get_func_code(node->function);
        //RESTORE_FRAME_PTR(code); // codeバッファの抽象アドレス
        uint64 ip; // = node->ip - code;
        fread(&ip, sizeof(uint64), 1, gp);
        node->ip = ip + code;
    }
    else {
        return;
    }
#if WASM_ENABLE_FAST_INTERP != 0
    uint32 ret_offset;
    uint32 *lp;
    uint32 operand[1];
#else

    // uint32 *sp_bottom; == node->lp + node->function->param_cell_num + node->function->local_cell_num
    // DUMP_PTR(node->sp_bottom);
    node->sp_bottom = node->lp + node->function->param_cell_num
                      + node->function->local_cell_num;

    // uint32 *sp_boundary; == sp_bottom + node->function->u.func->max_stack_cell_num
    // DUMP_PTR(node->sp_boundary);
    node->sp_boundary =
      node->sp_bottom + node->function->u.func->max_stack_cell_num;

    // uint32 *sp;
    //DUMP_PTR(node->sp);
    uint64 sp; // = node->sp - node->sp_bottom;
    fread(&sp, sizeof(uint64), 1, gp);
    node->sp = node->sp_bottom + sp;


    node->csp = node->csp_bottom = node->sp_boundary;
    node->csp_boundary =
      node->csp_bottom + node->function->u.func->max_block_num;

    uint32 bb_num;
    fread(&bb_num, sizeof(uint32), 1, gp);

    for (int i = 0; i < bb_num; i++) {
        // uint8 *begin_addr; frame_ip
        int64 ip; // = bb->begin_addr - wasm_get_func_code(node->function);
        fread(&ip, sizeof(int64), 1, gp);
        if(ip==-1){
            node->csp->begin_addr = NULL;
        }
        else {
            node->csp->begin_addr = ip + wasm_get_func_code(node->function);
        }

        // uint8 *target_addr; frame_ip
        //ip = bb->target_addr - wasm_get_func_code(node->function);
        fread(&ip, sizeof(int64), 1, gp);
        if(ip==-1){
            node->csp->target_addr = NULL;
        }
        else {
        node->csp->target_addr = ip + wasm_get_func_code(node->function);
        }

        // uint32 *frame_sp;
        //sp = bb->frame_sp - node->sp_bottom;
        fread(&ip, sizeof(int64), 1, gp);
        if(ip==-1){
            node->csp->frame_sp = NULL;
        }
        else {
        node->csp->frame_sp = ip + node->sp_bottom;
        }

        // uint32 cell_num;
        fread(&node->csp->cell_num, sizeof(uint32), 1, gp);
        node->csp++;
    }

    /* Frame data, the layout is:
     lp: param_cell_count + local_cell_count
     sp_bottom to sp_boundary: stack of data
     csp_bottom to csp_boundary: stack of block
     ref to frame end: data types of local vairables and stack data
     */
    //uint32 lp[1]; 環境依存のサイズが微妙align_uint(cell_num) 要チェック
    /*fwrite(node->lp, sizeof(uint32),
           addr->size - (uint32)offsetof(WASMInterpFrame, lp), fp);*/
    fread(node->lp, sizeof(uint32), node->function->param_cell_num, gp);
    fread(node->lp + node->function->param_cell_num, sizeof(uint32),
          node->function->local_cell_num, gp);
    fread(node->sp_bottom, sizeof(uint32),
          node->function->u.func->max_stack_cell_num, gp);
#endif
}

void
restore_BranchBlock(Pool_Info *addr)
{
    printf("BranchBlock\n");
    exit(1);
    //HEADER_RESTORE(BranchBlock);

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
}
void
restore_WASMLoaderContext(Pool_Info *addr)
{
    printf("WASMLoaderContext\n");
    exit(1);
    // skip
}
void
restore_Const(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(Const)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        // WASMValue value;
        fread(&node->value.v128.f32x4, sizeof(float64), 2, fp);
        // uint16 slot_index;
        fread(&node->slot_index, sizeof(uint16), 1, fp);
        // uint8 value_type;
        fread(&node->value_type, sizeof(uint8), 1, fp);
    }
}

void
restore_WASMModuleInstance(Pool_Info *addr)
{
    int p_abs;
    _module_inst_static = addr->p_raw;
    HEADER_RESTORE(WASMModuleInstance)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        // uint32 module_type;
        fread(&node->module_type, sizeof(uint32), 1, fp);

        // uint32 memory_count;
        fread(&node->memory_count, sizeof(uint32), 1, fp);
        // uint32 table_count;
        fread(&node->table_count, sizeof(uint32), 1, fp);
        // uint32 global_count;
        fread(&node->global_count, sizeof(uint32), 1, fp);
        // uint32 function_count;
        fread(&node->function_count, sizeof(uint32), 1, fp);

        // uint32 export_func_count;
        fread(&node->export_func_count, sizeof(uint32), 1, fp);

        // WASMMemoryInstance **memories;
        RESTORE_PTR(node->memories);
        // WASMTableInstance **tables;
        RESTORE_PTR(node->tables);
        // WASMGlobalInstance *globals;
        RESTORE_PTR(node->globals);
        // WASMFunctionInstance *functions;
        RESTORE_PTR(node->functions);

        // WASMExportFuncInstance *export_functions;
        RESTORE_PTR(node->export_functions);

        // WASMMemoryInstance *default_memory;
        RESTORE_PTR(node->default_memory);
        // WASMTableInstance *default_table;
        RESTORE_PTR(node->default_table);
        // uint8 *global_data;
        RESTORE_PTR(node->global_data);

        // WASMFunctionInstance *start_function;
        RESTORE_PTR(node->start_function);
        // WASMFunctionInstance *malloc_function;
        RESTORE_PTR(node->malloc_function);
        // WASMFunctionInstance *free_function;
        RESTORE_PTR(node->free_function);
        // WASMFunctionInstance *retain_function;
        RESTORE_PTR(node->retain_function);

        // WASMModule *module;
        RESTORE_PTR(node->module);

#if WASM_ENABLE_LIBC_WASI != 0
        //WASIContext *wasi_ctx;
        //RESTORE_PTR(node->wasi_ctx);
#endif

        // WASMExecEnv *exec_env_singleton;
        RESTORE_PTR(node->exec_env_singleton);

        // uint32 temp_ret;
        fread(&node->temp_ret, sizeof(uint32), 1, fp);
        // uint32 llvm_stack;
        fread(&node->llvm_stack, sizeof(uint32), 1, fp);

        // uint32 default_wasm_stack_size;
        fread(&node->default_wasm_stack_size, sizeof(uint32), 1, fp);

        /* The exception buffer of wasm interpreter for current thread. */
        // char cur_exception[128];
        fread(node->cur_exception, sizeof(char), 128, fp);

        /* The custom data that can be set/get by
     * wasm_set_custom_data/wasm_get_custom_data */
        // void *custom_data;
        RESTORE_PTR(node->custom_data);
    }
}

void
restore_WASMFunctionInstance(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(WASMFunctionInstance)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        // bool is_import_func;
        fread(&node->is_import_func, sizeof(bool), 1, fp);
        // uint16 param_count;
        fread(&node->param_count, sizeof(uint16), 1, fp);
        // uint16 local_count;
        fread(&node->local_count, sizeof(uint16), 1, fp);
        // uint16 param_cell_num;
        fread(&node->param_cell_num, sizeof(uint16), 1, fp);
        // uint16 ret_cell_num;
        fread(&node->ret_cell_num, sizeof(uint16), 1, fp);
        // uint16 local_cell_num;
        fread(&node->local_cell_num, sizeof(uint16), 1, fp);
#if WASM_ENABLE_FAST_INTERP != 0
        /* cell num of consts */
        uint16 const_cell_num;
#endif
        /*union {
            WASMFunctionImport *func_import;
            WASMFunction *func;
        } u;*/
        if (node->is_import_func) {
            WASMImport *import;
            WASMType *type;
            // WASMFunctionImport *func_import;
            RESTORE_PTR(import);
            node->u.func_import = &import->u.function;

            // uint8 *param_types; == type->types
            RESTORE_PTR(type);
            node->param_types = type->types;
        }
        else {
            RESTORE_PTR(node->u.func);

            // uint16 *local_offsets;
            RESTORE_PTR(node->local_offsets);

            // uint8 *param_types; == type->types
            WASMType *type;
            RESTORE_PTR(type);
            node->param_types = type->types;

            // uint8 *local_types; == (uint8*)func + sizeof(WASMFunction)
            node->local_types = ((uint8 *)node->u.func) + sizeof(WASMFunction);
        }
    }
}

void
restore_WASMMemoryInstance(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(WASMMemoryInstance)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        // uint32 module_type;
        fread(&node->module_type, sizeof(uint32), 1, fp);
        // bool is_shared;
        fread(&node->is_shared, sizeof(bool), 1, fp);
        // uint32 num_bytes_per_page;
        fread(&node->num_bytes_per_page, sizeof(uint32), 1, fp);
        // uint32 cur_page_count;
        fread(&node->cur_page_count, sizeof(uint32), 1, fp);
        // uint32 max_page_count;
        fread(&node->max_page_count, sizeof(uint32), 1, fp);

        // heapを使うプログラムはスキップ
        // uint8 *heap_data;
        //RESTORE_PTR(node->heap_data);
        // uint8 *heap_data_end;
        //RESTORE_PTR(node->heap_data_end);
        // void *heap_handle;
        //RESTORE_PTR(node->heap_handle);

        //uint8 *memory_data_end; memory_data + memory_data_size => heap_data_end

        /* Memory data begin address, the layout is: memory data + heap data
       Note: when memory is re-allocated, the heap data and memory data
             must be copied to new memory also. */
        //uint8 *memory_data;
        RESTORE_PTR(node->memory_data);
        node->memory_data_end =
          node->memory_data
          + (uint64)node->num_bytes_per_page * node->cur_page_count;
    }
}
void
restore_WASMMemoryInstanceT(Pool_Info *addr)
{
    int i, p_abs;
    Pool_Info *info;

    for (i = 0; i < addr->size; i++) {
        RESTORE_PTR(((WASMMemoryInstance **)addr->p_raw)[i]);
    }
}
void
restore_WASMTableInstance(Pool_Info *addr)
{
    HEADER_RESTORE_BUF(WASMTableInstance);

    // uint8 elem_type;
    fread(&node->elem_type, sizeof(uint8), 1, fp);
    // uint32 cur_size;
    fread(&node->cur_size, sizeof(uint32), 1, fp);
    // uint32 max_size;
    fread(&node->max_size, sizeof(uint32), 1, fp);
    // uint8 base_addr[1];
    fread(node->base_addr, sizeof(uint32), addr->size / sizeof(uint32), fp);
}

void
restore_WASMTableInstanceT(Pool_Info *addr)
{
    int i, p_abs;
    Pool_Info *info;

    for (i = 0; i < addr->size; i++) {
        RESTORE_PTR(((WASMTableInstance **)addr->p_raw)[i]);
    }
}

void
restore_WASMGlobalInstance(Pool_Info *addr)
{
    int p_abs;
    HEADER_RESTORE(WASMGlobalInstance)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d\n", addr->type);
            exit(1);
        }

        // uint8 type;
        fread(&node->type, sizeof(uint8), 1, fp);
        // bool is_mutable;
        fread(&node->is_mutable, sizeof(bool), 1, fp);
        // uint32 data_offset;
        fread(&node->data_offset, sizeof(uint32), 1, fp);
        // WASMValue initial_value;
        switch (node->type) {
            /*
                int32 i32;
                uint32 u32;
                int64 i64;
                uint64 u64;
                float32 f32;
                float64 f64;
                */
            case INIT_EXPR_TYPE_I32_CONST:
            case INIT_EXPR_TYPE_I64_CONST:
            case INIT_EXPR_TYPE_F32_CONST:
            case INIT_EXPR_TYPE_F64_CONST:
                fread(&node->initial_value.i64, sizeof(uint64), 1, fp);
                break;

            //V128 v128;
            case INIT_EXPR_TYPE_V128_CONST:
                /*
                        int8 i8x16[16];
                        int16 i16x8[8];
                        int32 i32x8[4];
                        int64 i64x2[2];
                        float32 f32x4[4];
                        float64 f64x2[2];
                        */
                fread(&node->initial_value.v128.f32x4, sizeof(float64), 2, fp);
                break;

            //uint32 ref_index;
            case INIT_EXPR_TYPE_FUNCREF_CONST:
            case INIT_EXPR_TYPE_REFNULL_CONST:
                fwrite(&node->initial_value.ref_index, sizeof(uint32), 1, fp);
                break;

            //uint32 global_index;
            case INIT_EXPR_TYPE_GET_GLOBAL:
                fread(&node->initial_value.global_index, sizeof(uint32), 1,
                      fp);
                break;

            //uintptr_t addr;
            default:
                fread(&node->initial_value.addr, sizeof(uintptr_t), 1, fp);
                break;
        }
    }
}

void
restore_WASMExportFuncInstance(Pool_Info *addr)
{
    int p_abs;
    StringNode *str_node;
    HEADER_RESTORE(WASMExportFuncInstance)
    {
        fread(&p_abs, sizeof(int), 1, fp);
        if (addr->p_abs != p_abs) {
            printf("no match p_abs:%d:%d!=%d\n", addr->type, addr->p_abs,
                   p_abs);
            exit(1);
        }

        //     char *name;
        RESTORE_PTR(str_node);
        node->name = ((char *)str_node) + sizeof(StringNode);

        //     WASMFunctionInstance *function;
        RESTORE_PTR(node->function);
    }
}
void
restore_WASMExportGlobInstance(Pool_Info *addr)
{
    printf("WASMExportGlobInstance\n");
}
void
restore_WASMSubModInstNode(Pool_Info *addr)
{
    printf("WASMSubModInstNode\n");
}

void
restore_WASMRuntimeFrame(Pool_Info *addr)
{
    printf("WASMRuntimeFrame\n");
}

void
restore_WASMOpcode(Pool_Info *addr)
{
    printf("WASMOpcode\n");
    // enum
}
void
restore_WASMMiscEXTOpcode(Pool_Info *addr)
{
    // enum
}
void
restore_WASMSimdEXTOpcode(Pool_Info *addr)
{
    // enum
}
void
restore_WASMAtomicEXTOpcode(Pool_Info *addr)
{
    // enum
}

void
restore_HashMapElem(Pool_Info *addr)
{
    printf("HashMapElem\n");
}
void
restore_HashMap(Pool_Info *addr)
{
    printf("HashMap\n");
}
void
restore_timer_ctx_t(Pool_Info *addr)
{
    printf("timer_ctx_t\n");
}
void
restore_app_timer_t(Pool_Info *addr)
{
    printf("app_timer_t\n");
}
void
restore_bh_queue(Pool_Info *addr)
{
    printf("bh_queue\n");
}
void
restore_bh_queue_node(Pool_Info *addr)
{
    printf("bh_queue_node\n");
}
void
restore_AtomicWaitInfo(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}
void
restore_AtomicWaitNode(Pool_Info *addr)
{
    printf("unsupported:%d\n", addr->type);
}

void
restore_wasi_iovec_t(Pool_Info *addr)
{
    printf("wasi_iovec_t\n");
}
void
restore_wasi_ciovec_t(Pool_Info *addr)
{
    printf("wasi_ciovec_t\n");
}
