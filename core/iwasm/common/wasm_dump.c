#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "wasm_runtime_common.h"
#include "../../shared/utils/bh_platform.h"
#include "mem_alloc.h"
#include "../../shared/mem-alloc/ems/ems_gc_internal.h"
#include "wasm_memory.h"
#include "wasm_dump.h"

static FILE *fp;
static unsigned long base;

#define HEADER(type)                                                          \
    type *node = (type *)addr->p_raw;                                         \
    fputc(type##T, fp);                                                       \
    fwrite(&addr->size, sizeof(size_t), 1, fp)

#define DUMP_PTR(attr)                                                        \
    do {                                                                      \
        unsigned long p = (unsigned long)attr;                                \
        p -= base;                                                            \
        fwrite(&p, sizeof(int), 1, fp);                                       \
    } while (0)

void
init_dump(void *addr)
{
    fp = fopen("dump.img", "wb");
    base = (unsigned long)addr;
}

void
dump_char(Pool_Info *addr)
{
    int i;
    fputc(charT, fp);
    fwrite(&addr->size, sizeof(size_t), 1, fp);
    for (i = 0; i < addr->size; i++) {
        int p_abs = addr->p_abs + i * sizeof(char);
        fwrite(&p_abs, sizeof(int), 1, fp);
        fwrite((char *)addr->p_raw + i, sizeof(char), 1, fp);
    }
}

void
dump_charT(Pool_Info *addr)
{
    int i;
    // skip
    fputc(charTT, fp);
    fwrite(&addr->size, sizeof(size_t), 1, fp);
    for (i = 0; i < addr->size; i++) {
        int p_abs = addr->p_abs + i * sizeof(char *);
        fwrite(&p_abs, sizeof(int), 1, fp);
        DUMP_PTR(addr->p_raw + i);
    }
}
void
dump_uint8(Pool_Info *addr)
{
    int i;
    fputc(uint8T, fp);
    fwrite(&addr->size, sizeof(size_t), 1, fp);
    for (i = 0; i < addr->size; i++) {
        int p_abs = addr->p_abs + i * sizeof(uint8);
        fwrite(&p_abs, sizeof(int), 1, fp);
        fwrite((uint8 *)addr->p_raw + i, sizeof(uint8), 1, fp);
    }
}
void
dump_uint16(Pool_Info *addr)
{
    int i;
    fputc(uint16T, fp);
    fwrite(&addr->size, sizeof(size_t), 1, fp);
    for (i = 0; i < addr->size; i++) {
        int p_abs = addr->p_abs + i * sizeof(uint16);
        fwrite(&p_abs, sizeof(int), 1, fp);
        fwrite((uint16 *)addr->p_raw + i, sizeof(uint16), 1, fp);
    }
}

void
dump_uint32(Pool_Info *addr)
{
    int i;
    fputc(uint32T, fp);
    fwrite(&addr->p_abs, sizeof(int), 1, fp);
    fwrite(&addr->size, sizeof(size_t), 1, fp);
    for (i = 0; i < addr->size; i++) {
        int p_abs = addr->p_abs + i * sizeof(uint32);
        fwrite(&p_abs, sizeof(int), 1, fp);
        fwrite((uint32 *)addr->p_raw + i, sizeof(uint32), 1, fp);
    }
}

void
dump_uint64(Pool_Info *addr)
{
    int i;
    fputc(uint64T, fp);
    fwrite(&addr->p_abs, sizeof(int), 1, fp);
    fwrite(&addr->size, sizeof(size_t), 1, fp);
    for (i = 0; i < addr->size; i++) {
        int p_abs = addr->p_abs + i * sizeof(uint64);
        fwrite(&p_abs, sizeof(int), 1, fp);
        fwrite((uint64 *)addr->p_raw + i, sizeof(uint64), 1, fp);
    }
}

void
dump_gc_heap_t(Pool_Info *addr)
{
    int i;
    HEADER(gc_heap_t);

    for (i = 0; i < addr->size; i++, node = (gc_heap_t *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(gc_heap_t);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //gc_handle_t heap_id; ==void*
        DUMP_PTR(node->heap_id);

        //gc_uint8 *base_addr;
        DUMP_PTR(node->base_addr);

        //gc_size_t current_size; ==uint32
        fwrite(&node->current_size, sizeof(uint32), 1, fp);

        //hmu_normal_list_t kfc_normal_list[HMU_NORMAL_NODE_CNT];
        for (i = 0; i < HMU_NORMAL_NODE_CNT; i++) {
            DUMP_PTR(node->kfc_normal_list[i].next);
        }

        //hmu_tree_node_t kfc_tree_root;
        fwrite(&node->kfc_tree_root.hmu_header, sizeof(gc_uint32), 1, fp);
        fwrite(&node->kfc_tree_root.size, sizeof(gc_size_t), 1, fp);
        DUMP_PTR(node->kfc_tree_root.left);
        DUMP_PTR(node->kfc_tree_root.right);
        DUMP_PTR(node->kfc_tree_root.parent);

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
dump_base_addr(Pool_Info *addr)
{
    // skip
}

void
dump_WASIContext(Pool_Info *addr)
{
    // skip
    //struct fd_table *curfds;
    //struct fd_prestats *prestats;
    //struct argv_environ_values *argv_environ;
    //char *argv_buf;
    //char **argv_list;
    //char *env_buf;
    //char **env_list;
}

void
dump_WASMThreadArg(Pool_Info *addr)
{
    /*
    HEADER(WASMThreadArg);

    //WASMExecEnv *new_exec_env;
    DUMP_PTR(node->new_exec_env);
    //wasm_thread_callback_t callback;==function pointer
    DUMP_PTR(node->callback);
    //void *arg;
    DUMP_PTR(node->arg);
    */
}

void
dump_ExternRefMapNode(Pool_Info *addr)
{
    // skip
}

void
dump_fd_table(Pool_Info *addr)
{
    // skip
    //struct rwlock lock;==pthread_rwlock_t object;
    //struct fd_entry *entries;
    //size_t size;
    //size_t used;
}
void
dump_fd_prestats(Pool_Info *addr)
{
    // skip
    //struct rwlock lock;==pthread_rwlock_t object;
    //struct fd_prestat *prestats;
    //size_t size;
    //size_t used;
}
void
dump_argv_environ_values(Pool_Info *addr)
{
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
dump_uvwasi_t(Pool_Info *addr)
{
    // skip
}
void
dump_uvwasi_preopen_t(Pool_Info *addr)
{
    // skip
}

void
dump_wasm_val_t(Pool_Info *addr)
{
    int i;
    HEADER(wasm_val_t);

    for (i = 0; i < addr->size; i++, node = (wasm_val_t *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(wasm_val_t);
        fwrite(&p_abs, sizeof(int), 1, fp);

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
                DUMP_PTR(node->of.ref);
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
dump_wasm_instance_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_engine_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_store_t(Pool_Info *addr)
{
    int i;
}
void
dump_Vector(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_valtype_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_functype_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_valtype_vec_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_globaltype_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_tabletype_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_memorytype_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_importtype_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_byte_vec_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_exporttype_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_ref_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_frame_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_trap_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_foreign_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_module_ex_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_func_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_global_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_table_t(Pool_Info *addr)
{
    int i;
}
void
dump_wasm_memory_t(Pool_Info *addr)
{
    int i;
}
/* 
    *end wasm-c-api
*/

void
dump_WASMExecEnv(Pool_Info *addr)
{
    int i;
    HEADER(WASMExecEnv);

    int p_abs = addr->p_abs;
    fwrite(&p_abs, sizeof(int), 1, fp);

    //struct WASMExecEnv *next;
    DUMP_PTR(node->next);

    //struct WASMExecEnv *prev;
    DUMP_PTR(node->prev);

    //struct WASMModuleInstanceCommon *module_inst;
    DUMP_PTR(node->module_inst);

    //uint8 *native_stack_boundary;
    DUMP_PTR(node->native_stack_boundary);

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
    DUMP_PTR(node->attachment);

    //void *user_data;??
    //DUMP_PTR(node->user_data);

    //struct WASMInterpFrame *cur_frame;
    DUMP_PTR(node->cur_frame);

    //korp_tid handle;

    /*
#ifdef OS_ENABLE_HW_BOUND_CHECK
    WASMJmpBuf *jmpbuf_stack_top;
#endif
*/

    //uint32 wasm_stack_size;
    fwrite(&node->wasm_stack_size, sizeof(uint32), 1, fp);
    /*
    union {
        uint64 __make_it_8_byte_aligned_;

        struct {
            uint8 *top_boundary;

            uint8 *top;

            uint8 bottom[1];
        } s;
    } wasm_stack;*/
    DUMP_PTR(node->wasm_stack.s.top_boundary);
    DUMP_PTR(node->wasm_stack.s.top);
    fwrite(node->wasm_stack.s.bottom, sizeof(uint8), addr->size, fp);
}

void
dump_NativeSymbolsNode(Pool_Info *addr)
{
    int i;
    HEADER(NativeSymbolsNode);
    for (i = 0; i < addr->size;
         i++, node = (NativeSymbolsNode *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(NativeSymbolsNode);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //struct NativeSymbolsNode *next;
        DUMP_PTR(node->next);
        //const char *module_name; 外部

        //NativeSymbol *native_symbols; 付け替え

        //uint32 n_native_symbols; 付け替え

        //bool call_conv_raw; wasm_runtime_full_initからだとfalse
    }
}

void
dump_WASMModuleCommon(Pool_Info *addr)
{
    int i;
    HEADER(WASMModuleCommon);

    for (i = 0; i < addr->size;
         i++, node = (WASMModuleCommon *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMModuleCommon);
        fwrite(&p_abs, sizeof(int), 1, fp);
        //uint32 module_type;
        fwrite(&node->module_type, sizeof(uint32), 1, fp);
        //uint8 module_data[1];
        fwrite(node->module_data, sizeof(uint8), 1, fp);
    }
}
void
dump_WASMModuleInstanceCommon(Pool_Info *addr)
{
    int i;
    HEADER(WASMModuleInstanceCommon);

    for (i = 0; i < addr->size;
         i++, node = (WASMModuleInstanceCommon *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMModuleInstanceCommon);
        fwrite(&p_abs, sizeof(int), 1, fp);
        //uint32 module_type;
        fwrite(&node->module_type, sizeof(uint32), 1, fp);
        //uint8 module_inst_data[1];
        fwrite(node->module_inst_data, sizeof(uint8), 1, fp);
    }
}

void
dump_WASMModuleMemConsumption(Pool_Info *addr)
{
    int i;
    HEADER(WASMModuleMemConsumption);

    for (i = 0; i < addr->size;
         i++, node = (WASMModuleMemConsumption *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMModuleMemConsumption);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //uint32 total_size;
        fwrite(&node->total_size, sizeof(uint32), 1, fp);
        //uint32 module_struct_size;
        fwrite(&node->module_struct_size, sizeof(uint32), 1, fp);
        //uint32 types_size;
        fwrite(&node->types_size, sizeof(uint32), 1, fp);
        //uint32 imports_size;
        fwrite(&node->imports_size, sizeof(uint32), 1, fp);
        //uint32 functions_size;
        fwrite(&node->functions_size, sizeof(uint32), 1, fp);
        //uint32 tables_size;
        fwrite(&node->tables_size, sizeof(uint32), 1, fp);
        //uint32 memories_size;
        fwrite(&node->memories_size, sizeof(uint32), 1, fp);
        //uint32 globals_size;
        fwrite(&node->globals_size, sizeof(uint32), 1, fp);
        //uint32 exports_size;
        fwrite(&node->exports_size, sizeof(uint32), 1, fp);
        //uint32 table_segs_size;
        fwrite(&node->table_segs_size, sizeof(uint32), 1, fp);
        //uint32 data_segs_size;
        fwrite(&node->data_segs_size, sizeof(uint32), 1, fp);
        //uint32 const_strs_size;
        fwrite(&node->const_strs_size, sizeof(uint32), 1, fp);
        /*
#if WASM_ENABLE_AOT != 0
    uint32 aot_code_size;
#endif*/
    }
}
void
dump_WASMModuleInstMemConsumption(Pool_Info *addr)
{
    int i;
    HEADER(WASMModuleInstMemConsumption);

    for (i = 0; i < addr->size;
         i++, node = (WASMModuleInstMemConsumption *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMModuleInstMemConsumption);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //uint32 total_size;
        fwrite(&node->total_size, sizeof(uint32), 1, fp);
        //uint32 module_inst_struct_size;
        fwrite(&node->module_inst_struct_size, sizeof(uint32), 1, fp);
        //uint32 memories_size;
        fwrite(&node->memories_size, sizeof(uint32), 1, fp);
        //uint32 app_heap_size;
        fwrite(&node->app_heap_size, sizeof(uint32), 1, fp);
        //uint32 tables_size;
        fwrite(&node->tables_size, sizeof(uint32), 1, fp);
        //uint32 globals_size;
        fwrite(&node->globals_size, sizeof(uint32), 1, fp);
        //uint32 functions_size;
        fwrite(&node->functions_size, sizeof(uint32), 1, fp);
        //uint32 exports_size;
        fwrite(&node->exports_size, sizeof(uint32), 1, fp);
    }
}
void
dump_WASMMemoryInstanceCommon(Pool_Info *addr)
{
    int i;
    HEADER(WASMMemoryInstanceCommon);

    for (i = 0; i < addr->size;
         i++, node = (WASMMemoryInstanceCommon *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMMemoryInstanceCommon);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //uint32 module_type;
        fwrite(&node->module_type, sizeof(uint32), 1, fp);
        //uint8 memory_inst_data[1];
        fwrite(node->memory_inst_data, sizeof(uint8), 1, fp);
    }
}
void
dump_WASMSection(Pool_Info *addr) //==wasm_section_t
{
    int i;
    HEADER(WASMSection);

    for (i = 0; i < addr->size; i++, node = (WASMSection *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMSection);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //struct wasm_section_t *next;
        DUMP_PTR(node->next);
        //int section_type;
        fwrite(&node->section_type, sizeof(int), 1, fp);
        //uint8_t *section_body; 外部wasmファイル => loader_mallocでuint8を確保
        DUMP_PTR(node->section_body);
        //uint32_t section_body_size;
        fwrite(&node->section_body_size, sizeof(uint32_t), 1, fp);
    }
}
void
dump_WASMCApiFrame(Pool_Info *addr) //==wasm_frame_t
{
    int i;
    HEADER(WASMCApiFrame);

    for (i = 0; i < addr->size; i++, node = (WASMCApiFrame *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMCApiFrame);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //void *instance;
        DUMP_PTR(node->instance);
        //uint32 module_offset;
        fwrite(&node->module_offset, sizeof(uint32), 1, fp);
        //uint32 func_index;
        fwrite(&node->func_index, sizeof(uint32), 1, fp);
        //uint32 func_offset;
        fwrite(&node->func_offset, sizeof(uint32), 1, fp);
    }
}

void
dump_WASMSharedMemNode(Pool_Info *addr)
{
    int i;
    //skip
}

void
dump_WASMModule(Pool_Info *addr) // 要チェック
{
    int i;
    HEADER(WASMModule);

    for (i = 0; i < addr->size; i++, node = (WASMModule *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMModule);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //uint32 module_type;
        fwrite(&node->module_type, sizeof(uint32), 1, fp);
        //uint32 type_count;
        fwrite(&node->type_count, sizeof(uint32), 1, fp);
        //uint32 import_count;
        fwrite(&node->import_count, sizeof(uint32), 1, fp);
        //uint32 function_count;
        fwrite(&node->function_count, sizeof(uint32), 1, fp);
        //uint32 table_count;
        fwrite(&node->table_count, sizeof(uint32), 1, fp);
        //uint32 memory_count;
        fwrite(&node->memory_count, sizeof(uint32), 1, fp);
        //uint32 global_count;
        fwrite(&node->global_count, sizeof(uint32), 1, fp);
        //uint32 export_count;
        fwrite(&node->export_count, sizeof(uint32), 1, fp);
        //uint32 table_seg_count;
        fwrite(&node->table_seg_count, sizeof(uint32), 1, fp);
        //uint32 data_seg_count;
        fwrite(&node->data_seg_count, sizeof(uint32), 1, fp);

        //uint32 import_function_count;
        fwrite(&node->import_function_count, sizeof(uint32), 1, fp);
        //uint32 import_table_count;
        fwrite(&node->import_table_count, sizeof(uint32), 1, fp);
        //uint32 import_memory_count;
        fwrite(&node->import_memory_count, sizeof(uint32), 1, fp);
        //uint32 import_global_count;
        fwrite(&node->import_global_count, sizeof(uint32), 1, fp);

        //WASMImport *import_functions;
        DUMP_PTR(node->import_functions);
        //WASMImport *import_tables;
        DUMP_PTR(node->import_tables);
        //WASMImport *import_memories;
        DUMP_PTR(node->import_memories);
        //WASMImport *import_globals;
        DUMP_PTR(node->import_globals);

        //WASMType **types;
        DUMP_PTR(node->types);
        //WASMImport *imports;
        DUMP_PTR(node->imports);
        //WASMFunction **functions;
        DUMP_PTR(node->functions);
        //WASMTable *tables;
        DUMP_PTR(node->tables);
        // WASMMemory *memories;
        DUMP_PTR(node->memories);
        // WASMGlobal *globals;
        DUMP_PTR(node->globals);
        // WASMExport *exports;
        DUMP_PTR(node->exports);
        // WASMTableSeg *table_segments;
        DUMP_PTR(node->table_segments);
        // WASMDataSeg **data_segments;
        DUMP_PTR(node->data_segments);
        // uint32 start_function;
        fwrite(&node->start_function, sizeof(uint32), 1, fp);

        //uint32 aux_data_end_global_index;
        fwrite(&node->aux_data_end_global_index, sizeof(uint32), 1, fp);
        //uint32 aux_data_end;
        fwrite(&node->aux_data_end, sizeof(uint32), 1, fp);

        //uint32 aux_heap_base_global_index;
        fwrite(&node->aux_heap_base_global_index, sizeof(uint32), 1, fp);
        //uint32 aux_heap_base;
        fwrite(&node->aux_heap_base, sizeof(uint32), 1, fp);

        //uint32 aux_stack_top_global_index;
        fwrite(&node->aux_stack_top_global_index, sizeof(uint32), 1, fp);
        //uint32 aux_stack_bottom;
        fwrite(&node->aux_stack_bottom, sizeof(uint32), 1, fp);
        //uint32 aux_stack_size;
        fwrite(&node->aux_stack_size, sizeof(uint32), 1, fp);

        //uint32 malloc_function;
        fwrite(&node->malloc_function, sizeof(uint32), 1, fp);
        //uint32 free_function;
        fwrite(&node->free_function, sizeof(uint32), 1, fp);

        //uint32 retain_function;
        fwrite(&node->retain_function, sizeof(uint32), 1, fp);

        //bool possible_memory_grow;
        fwrite(&node->possible_memory_grow, sizeof(bool), 1, fp);

        StringList const_str_list;
        DUMP_PTR(node->const_str_list);

#if WASM_ENABLE_LIBC_WASI != 0
        /*WASIArguments wasi_args;
    bool is_wasi_module;*/
#endif
    }
}
void
dump_WASMFunction(Pool_Info *addr) //要チェック
{
    int i;
    HEADER(WASMFunction);
    int p_abs = addr->p_abs + i * sizeof(WASMFunction);
    fwrite(&p_abs, sizeof(int), 1, fp);

#if WASM_ENABLE_CUSTOM_NAME_SECTION != 0
    char *field_name;
#endif
    // WASMType *func_type;
    DUMP_PTR(node->func_type);
    // uint32 local_count;
    fwrite(&node->local_count, sizeof(uint32), 1, fp);
    // uint8 *local_types;
    DUMP_PTR(node->local_types);

    // uint16 param_cell_num;
    fwrite(&node->param_cell_num, sizeof(uint16), 1, fp);
    // uint16 ret_cell_num;
    fwrite(&node->ret_cell_num, sizeof(uint16), 1, fp);
    // uint16 local_cell_num;
    fwrite(&node->local_cell_num, sizeof(uint16), 1, fp);
    // uint16 *local_offsets; probably Ok
    DUMP_PTR(node->local_offsets);

    //uint32 max_stack_cell_num;
    fwrite(&node->max_stack_cell_num, sizeof(uint32), 1, fp);
    //uint32 max_block_num;
    fwrite(&node->max_block_num, sizeof(uint32), 1, fp);

    //bool has_op_memory_grow;
    fwrite(&node->has_op_memory_grow, sizeof(bool), 1, fp);
    //bool has_op_func_call;
    fwrite(&node->has_op_func_call, sizeof(bool), 1, fp);
    // uint32 code_size;
    fwrite(&node->code_size, sizeof(uint32), 1, fp);
    // uint8 *code; //loader_mallocでcodeをcopy
    DUMP_PTR(node->code);

    // +local_count // sizeof(WASMFunction) + (uint64)local_count
    fwrite(node->local_types, sizeof(uint8), node->local_count, fp);
}
void
dump_WASMFunctionT(Pool_Info *addr)
{
    int i;
    fputc(WASMFunctionTT, fp);
    fwrite(&addr->size, sizeof(size_t), 1, fp);
    for (i = 0; i < addr->size; i++) {
        int p_abs = addr->p_abs + i * sizeof(WASMFunction *);
        fwrite(&p_abs, sizeof(int), 1, fp);
        DUMP_PTR(addr->p_raw + i);
    }
}
void
dump_WASMGlobal(Pool_Info *addr)
{
    int i;
    HEADER(WASMGlobal);
    for (i = 0; i < addr->size; i++, node = (WASMGlobal *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMGlobal);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //uint8 type;
        fwrite(&node->type, sizeof(uint8), 1, fp);
        //bool is_mutable;
        fwrite(&node->is_mutable, sizeof(bool), 1, fp);
        //InitializerExpression init_expr;
        {
            //uint8 init_expr_type;
            fwrite(&node->init_expr.init_expr_type, sizeof(uint8), 1, fp);
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
                        fwrite(&node->init_expr.u.i64, sizeof(uint64), 1, fp);
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
                        fwrite(&node->init_expr.u.v128.f32x4, sizeof(float64),
                               2, fp);
                        break;

                    //uint32 ref_index;
                    case INIT_EXPR_TYPE_FUNCREF_CONST:
                    case INIT_EXPR_TYPE_REFNULL_CONST:
                        fwrite(&node->init_expr.u.ref_index, sizeof(uint32), 1,
                               fp);
                        break;

                    //uint32 global_index;
                    case INIT_EXPR_TYPE_GET_GLOBAL:
                        fwrite(&node->init_expr.u.global_index, sizeof(uint32),
                               1, fp);

                        break;

                    //uintptr_t addr;
                    default:
                        fwrite(&node->init_expr.u.addr, sizeof(uintptr_t), 1,
                               fp);
                        break;
                }
            }
        }
    }
}

void
dump_WASMExport(Pool_Info *addr)
{
    int i;
    HEADER(WASMExport);
    for (i = 0; i < addr->size; i++, node = (WASMExport *)addr->p_raw + i) {
        //char *name; stringnodeの途中を指す
        fputs(node->name, fp);
        //uint8 kind;
        fwrite(&node->kind, sizeof(uint8), 1, fp);
        //uint32 index;
        fwrite(&node->index, sizeof(uint32), 1, fp);
    }
}
void
dump_V128(Pool_Info *addr)
{
    int i;
    HEADER(V128);
    for (i = 0; i < addr->size; i++, node = (WASMExport *)addr->p_raw + i) {
        fwrite(&node->f64x2, sizeof(float64), 2, fp);
    }
}
void
dump_WASMValue(Pool_Info *addr)
{
    int i;
    HEADER(WASMValue);
    for (i = 0; i < addr->size; i++, node = (WASMValue *)addr->p_raw + i) {
        fwrite(&node->v128.f32x4, sizeof(float64), 2, fp);
    }
}
void
dump_InitializerExpression(Pool_Info *addr)
{
    int i;
    HEADER(InitializerExpression);

    for (i = 0; i < addr->size;
         i++, node = (InitializerExpression *)addr->p_raw + i) {
        //uint8 init_expr_type;
        fwrite(&node->init_expr_type, sizeof(uint8), 1, fp);
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
                    fwrite(&node->u.i64, sizeof(uint64), 1, fp);
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
                    fwrite(&node->u.v128.f32x4, sizeof(float64), 2, fp);
                    break;

                //uint32 ref_index;
                case INIT_EXPR_TYPE_FUNCREF_CONST:
                case INIT_EXPR_TYPE_REFNULL_CONST:
                    fwrite(&node->u.ref_index, sizeof(uint32), 1, fp);
                    break;

                //uint32 global_index;
                case INIT_EXPR_TYPE_GET_GLOBAL:
                    fwrite(&node->u.global_index, sizeof(uint32), 1, fp);
                    break;

                //uintptr_t addr;
                default:
                    fwrite(&node->u.addr, sizeof(uintptr_t), 1, fp);
                    break;
            }
        }
    }
}
void
dump_WASMType(Pool_Info *addr)
{
    int i;
    HEADER(WASMType);
    // uint16 param_count;
    fwrite(&node->param_count, sizeof(uint16), 1, fp);
    // uint16 result_count;
    fwrite(&node->result_count, sizeof(uint16), 1, fp);
    // uint16 param_cell_num;
    fwrite(&node->param_cell_num, sizeof(uint16), 1, fp);
    // uint16 ret_cell_num;
    fwrite(&node->ret_cell_num, sizeof(uint16), 1, fp);
    // uint8 types[1]; offsetof(WASMType, types) + sizeof(uint8) * (uint64)(param_count + result_count);
    fwrite(node->types, sizeof(uint8), node->param_count + node->result_count,
           fp);

    //+
}
void
dump_WASMTypeT(Pool_Info *addr)
{
    int i;
    fputc(WASMTypeTT, fp);
    fwrite(&addr->size, sizeof(size_t), 1, fp);
    for (i = 0; i < addr->size; i++) {
        int p_abs = addr->p_abs + i * sizeof(WASMType *);
        fwrite(&p_abs, sizeof(int), 1, fp);
        DUMP_PTR(addr->p_raw + i);
    }
}
void
dump_WASMTable(Pool_Info *addr)
{
    int i;
}
void
dump_WASMMemory(Pool_Info *addr)
{
    int i;
}
void
dump_WASMTableImport(Pool_Info *addr)
{
    int i;
}
void
dump_WASMMemoryImport(Pool_Info *addr)
{
    int i;
}
void
dump_WASMFunctionImport(Pool_Info *addr)
{
    int i;
}
void
dump_WASMGlobalImport(Pool_Info *addr)
{
    int i;
}
void
dump_WASMImport(Pool_Info *addr)
{
    int i;
}
void
dump_WASMTableSeg(Pool_Info *addr)
{
    int i;
}
void
dump_WASMDataSeg(Pool_Info *addr)
{
    int i;
}
void
dump_BlockAddr(Pool_Info *addr)
{
    int i;
}
void
dump_WASIArguments(Pool_Info *addr)
{
    int i;
}
void
dump_StringNode(Pool_Info *addr)
{
    int i;
}
void
dump_BlockType(Pool_Info *addr)
{
    int i;
}
void
dump_WASMBranchBlock(Pool_Info *addr)
{
    int i;
}

void
dump_WASMInterpFrame(Pool_Info *addr)
{
    int i;
}

void
dump_BranchBlock(Pool_Info *addr)
{
    int i;
}
void
dump_WASMLoaderContext(Pool_Info *addr)
{
    int i;
}
void
dump_Const(Pool_Info *addr)
{
    int i;
}

void
dump_WASMModuleInstance(Pool_Info *addr)
{
    int i;
}
void
dump_WASMFunctionInstance(Pool_Info *addr)
{
    int i;
}
void
dump_WASMMemoryInstance(Pool_Info *addr)
{
    int i;
}
void
dump_WASMMemoryInstanceT(Pool_Info *addr)
{
    int i;
}
void
dump_WASMTableInstance(Pool_Info *addr)
{
    int i;
}
void
dump_WASMTableInstanceT(Pool_Info *addr)
{
    int i;
}
void
dump_WASMGlobalInstance(Pool_Info *addr)
{
    int i;
}
void
dump_WASMExportFuncInstance(Pool_Info *addr)
{
    int i;
}
void
dump_WASMRuntimeFrame(Pool_Info *addr)
{
    int i;
}

void
dump_WASMOpcode(Pool_Info *addr)
{
    int i;
}
void
dump_WASMMiscEXTOpcode(Pool_Info *addr)
{
    int i;
}
void
dump_WASMSimdEXTOpcode(Pool_Info *addr)
{
    int i;
}
void
dump_WASMAtomicEXTOpcode(Pool_Info *addr)
{
    int i;
}
