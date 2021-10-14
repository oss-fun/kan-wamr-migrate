#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "wasm_runtime_common.h"
#include "../../shared/utils/bh_platform.h"
#include "mem_alloc.h"
#include "../../shared/mem-alloc/ems/ems_gc_internal.h"
#include "wasm_memory.h"
#include "wasm_dump.h"
#include "wasm_interp.h"
#include "wasm_loader.h"
#include "wasm_runtime.h"

static FILE *fp;
static unsigned long base;

#define HEADER(type)                                                          \
    type *node = (type *)addr->p_raw;                                         \
    fputc(type##T, fp);                                                       \
    fwrite(&addr->size, sizeof(int), 1, fp)

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
    int i, j;
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

    //#if WASM_ENABLE_INTERP != 0 && WASM_ENABLE_FAST_INTERP == 0
    //BlockAddr block_addr_cache[BLOCK_ADDR_CACHE_SIZE][BLOCK_ADDR_CONFLICT_SIZE];
    BLOCK_ADDR_CACHE_SIZE;
    BLOCK_ADDR_CONFLICT_SIZE;
    for (i = 0; i < BLOCK_ADDR_CACHE_SIZE; i++) {
        for (j = 0; j < BLOCK_ADDR_CONFLICT_SIZE; j++) {
            // const uint8 *start_addr;
            DUMP_PTR(node->block_addr_cache[i][j].start_addr); // frame_ip
            // uint8 *else_addr;
            DUMP_PTR(node->block_addr_cache[i][j].else_addr); // frame_ip
            // uint8 *end_addr;
            DUMP_PTR(node->block_addr_cache[i][j].end_addr);
        }
    }

    //#endif
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
    //fwrite(node->wasm_stack.s.bottom, sizeof(uint8), addr->size, fp);
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
        int p_abs = addr->p_abs + i * sizeof(WASMExport);
        fwrite(&p_abs, sizeof(int), 1, fp);

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
        int p_abs = addr->p_abs + i * sizeof(V128);
        fwrite(&p_abs, sizeof(int), 1, fp);

        fwrite(&node->f64x2, sizeof(float64), 2, fp);
    }
}
void
dump_WASMValue(Pool_Info *addr)
{
    int i;
    HEADER(WASMValue);
    for (i = 0; i < addr->size; i++, node = (WASMValue *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMValue);
        fwrite(&p_abs, sizeof(int), 1, fp);

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
        int p_abs = addr->p_abs + i * sizeof(InitializerExpression);
        fwrite(&p_abs, sizeof(int), 1, fp);

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

    int p_abs = addr->p_abs + i * sizeof(WASMType);
    fwrite(&p_abs, sizeof(int), 1, fp);

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
    HEADER(WASMTable);

    for (i = 0; i < addr->size; i++, node = (WASMTable *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMTable);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //uint8 elem_type;
        fwrite(&node->elem_type, sizeof(uint8), 1, fp);
        //uint32 flags;
        fwrite(&node->flags, sizeof(uint32), 1, fp);
        //uint32 init_size;
        fwrite(&node->init_size, sizeof(uint32), 1, fp);
        // uint32 max_size;
        fwrite(&node->max_size, sizeof(uint32), 1, fp);
        // bool possible_grow;
        fwrite(&node->possible_grow, sizeof(bool), 1, fp);
    }
}
void
dump_WASMMemory(Pool_Info *addr)
{
    int i;
    HEADER(WASMMemory);

    for (i = 0; i < addr->size; i++, node = (WASMMemory *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMMemory);
        fwrite(&p_abs, sizeof(int), 1, fp);

        // uint32 flags;
        fwrite(&node->flags, sizeof(uint32), 1, fp);
        // uint32 num_bytes_per_page;
        fwrite(&node->num_bytes_per_page, sizeof(uint32), 1, fp);
        // uint32 init_page_count;
        fwrite(&node->init_page_count, sizeof(uint32), 1, fp);
        // uint32 max_page_count;
        fwrite(&node->max_page_count, sizeof(uint32), 1, fp);
    }
}

/*
for (i = 0; i < addr->size; i++, node = (*)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof();
        fwrite(&p_abs, sizeof(int), 1, fp);
*/
void
dump_WASMTableImport(Pool_Info *addr)
{
    int i;
    // maybe skip
}
void
dump_WASMMemoryImport(Pool_Info *addr)
{
    int i;
    // maybe skip
}
void
dump_WASMFunctionImport(Pool_Info *addr)
{
    int i;
    // maybe skip
}
void
dump_WASMGlobalImport(Pool_Info *addr)
{
    int i;
    // maybe skip
}
void
dump_WASMImport(Pool_Info *addr)
{
    int i;
    HEADER(WASMImport);

    for (i = 0; i < addr->size; i++, node = (WASMImport *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMImport);
        fwrite(&p_abs, sizeof(int), 1, fp);

        // uint8 kind;
        fwrite(&node->kind, sizeof(uint8), 1, fp);

        DUMP_PTR(node->u.names.module_name);
        DUMP_PTR(node->u.names.field_name);
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
                DUMP_PTR(node->u.function.func_type);
                // void *func_ptr_linked;
                /* restore時に再設定 */

                // const char *signature;
                DUMP_PTR(node->u.function.signature); // maybe ok

                // void *attachment;
                DUMP_PTR(node->u.function.attachment); // 要チェック
                // bool call_conv_raw;
                fwrite(&node->u.function.call_conv_raw, sizeof(bool), 1, fp);
#if WASM_ENABLE_MULTI_MODULE != 0
                WASMModule *import_module;
                WASMFunction *import_func_linked;
#endif
                //bool call_conv_wasm_c_api;
                fwrite(&node->u.function.call_conv_wasm_c_api, sizeof(bool), 1,
                       fp);
                //bool wasm_c_api_with_env;
                fwrite(&node->u.function.wasm_c_api_with_env, sizeof(bool), 1,
                       fp);
                break;

            case IMPORT_KIND_TABLE:
                // uint8 elem_type;
                fwrite(&node->u.table.elem_type, sizeof(uint8), 1, fp);
                // uint32 flags;
                fwrite(&node->u.table.flags, sizeof(uint32), 1, fp);
                // uint32 init_size;
                fwrite(&node->u.table.init_size, sizeof(uint32), 1, fp);
                // uint32 max_size;
                fwrite(&node->u.table.max_size, sizeof(uint32), 1, fp);
                // bool possible_grow;
                fwrite(&node->u.table.possible_grow, sizeof(bool), 1, fp);
#if WASM_ENABLE_MULTI_MODULE != 0
                WASMModule *import_module;
                WASMTable *import_table_linked;
#endif
                break;

            case IMPORT_KIND_MEMORY:
                // uint32 flags;
                fwrite(&node->u.memory.flags, sizeof(uint32), 1, fp);
                // uint32 num_bytes_per_page;
                fwrite(&node->u.memory.num_bytes_per_page, sizeof(uint32), 1,
                       fp);
                // uint32 init_page_count;
                fwrite(&node->u.memory.init_page_count, sizeof(uint32), 1, fp);
                // uint32 max_page_count;
                fwrite(&node->u.memory.max_page_count, sizeof(uint32), 1, fp);
#if WASM_ENABLE_MULTI_MODULE != 0
                WASMModule *import_module;
                WASMMemory *import_memory_linked;
#endif
                break;

            case IMPORT_KIND_GLOBAL:
                // uint8 type;
                fwrite(&node->u.global.type, sizeof(uint8), 1, fp);
                // bool is_mutable;
                fwrite(&node->u.global.is_mutable, sizeof(bool), 1, fp);
                // WASMValue global_data_linked; /* global data after linked =>???*/
                fwrite(&node->u.global.global_data_linked.v128.f32x4,
                       sizeof(float64), 2, fp);
                // bool is_linked;
                fwrite(&node->u.global.is_linked, sizeof(bool), 1, fp);
#if WASM_ENABLE_MULTI_MODULE != 0
                /* imported function pointer after linked */
                /* TODO: remove if not needed */
                WASMModule *import_module;
                WASMGlobal *import_global_linked;
#endif
                break;

            default:
                break;
        }
    }
}
void
dump_WASMTableSeg(Pool_Info *addr)
{
    int i;
    HEADER(WASMTableSeg);

    for (i = 0; i < addr->size; i++, node = (WASMTableSeg *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMTableSeg);
        fwrite(&p_abs, sizeof(int), 1, fp);

        // uint32 mode;
        fwrite(&node->mode, sizeof(uint32), 1, fp);
        // uint32 elem_type;
        fwrite(&node->elem_type, sizeof(uint32), 1, fp);
        // bool is_dropped;
        fwrite(&node->is_dropped, sizeof(bool), 1, fp);
        // uint32 table_index;
        fwrite(&node->table_index, sizeof(uint32), 1, fp);

        // InitializerExpression base_offset;
        {
            //uint8 init_expr_type;
            fwrite(&node->base_offset.init_expr_type, sizeof(uint8), 1, fp);
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
                        fwrite(&node->base_offset.u.i64, sizeof(uint64), 1,
                               fp);
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
                        fwrite(&node->base_offset.u.v128.f32x4,
                               sizeof(float64), 2, fp);
                        break;

                    //uint32 ref_index;
                    case INIT_EXPR_TYPE_FUNCREF_CONST:
                    case INIT_EXPR_TYPE_REFNULL_CONST:
                        fwrite(&node->base_offset.u.ref_index, sizeof(uint32),
                               1, fp);
                        break;

                    //uint32 global_index;
                    case INIT_EXPR_TYPE_GET_GLOBAL:
                        fwrite(&node->base_offset.u.global_index,
                               sizeof(uint32), 1, fp);
                        break;

                    //uintptr_t addr;
                    default:
                        fwrite(&node->base_offset.u.addr, sizeof(uintptr_t), 1,
                               fp);
                        break;
                }
            }
        }

        // uint32 function_count;
        fwrite(&node->function_count, sizeof(uint32), 1, fp);
        // uint32 *func_indexes;
        DUMP_PTR(node->func_indexes);
    }
}
void
dump_WASMDataSeg(Pool_Info *addr)
{
    int i;
    HEADER(WASMDataSeg);

    for (i = 0; i < addr->size; i++, node = (WASMDataSeg *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMDataSeg);
        fwrite(&p_abs, sizeof(int), 1, fp);

        // uint32 memory_index;
        fwrite(&node->memory_index, sizeof(uint32), 1, fp);
        // InitializerExpression base_offset;
        {
            //uint8 init_expr_type;
            fwrite(&node->base_offset.init_expr_type, sizeof(uint8), 1, fp);
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
                        fwrite(&node->base_offset.u.i64, sizeof(uint64), 1,
                               fp);
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
                        fwrite(&node->base_offset.u.v128.f32x4,
                               sizeof(float64), 2, fp);
                        break;

                    //uint32 ref_index;
                    case INIT_EXPR_TYPE_FUNCREF_CONST:
                    case INIT_EXPR_TYPE_REFNULL_CONST:
                        fwrite(&node->base_offset.u.ref_index, sizeof(uint32),
                               1, fp);
                        break;

                    //uint32 global_index;
                    case INIT_EXPR_TYPE_GET_GLOBAL:
                        fwrite(&node->base_offset.u.global_index,
                               sizeof(uint32), 1, fp);
                        break;

                    //uintptr_t addr;
                    default:
                        fwrite(&node->base_offset.u.addr, sizeof(uintptr_t), 1,
                               fp);
                        break;
                }
            }
        }

        // uint32 data_length;
        fwrite(&node->data_length, sizeof(uint32), 1, fp);
#if WASM_ENABLE_BULK_MEMORY != 0
        bool is_passive;
#endif
        // uint8 *data;
        DUMP_PTR(node->data);
    }
}
void
dump_WASMDataSegT(Pool_Info *addr)
{
    int i;
    fputc(WASMDataSegTT, fp);
    fwrite(&addr->size, sizeof(size_t), 1, fp);
    for (i = 0; i < addr->size; i++) {
        int p_abs = addr->p_abs + i * sizeof(WASMDataSeg *);
        fwrite(&p_abs, sizeof(int), 1, fp);

        DUMP_PTR(addr->p_raw + i);
    }
}
void
dump_BlockAddr(Pool_Info *addr)
{
    int i;
    // skip
}
void
dump_WASIArguments(Pool_Info *addr)
{
    int i;
    // maybe skip
    // argvだけcheckpoint
}
void
dump_StringNode(Pool_Info *addr)
{
    int i;
    HEADER(StringNode);

    //size
    fwrite(&addr->size, sizeof(int), 1, fp);
    //struct StringNode *next;
    DUMP_PTR(node->next);
    //char *str;
    fwrite(node->str, sizeof(char), addr->size, fp);
}
void
dump_BlockType(Pool_Info *addr)
{
    int i;
    // skip
}
void
dump_WASMBranchBlock(Pool_Info *addr) //要チェック アロケートはされない
{
    int i;
    HEADER(WASMBranchBlock);

    for (i = 0; i < addr->size;
         i++, node = (WASMBranchBlock *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMBranchBlock);
        fwrite(&p_abs, sizeof(int), 1, fp);
        // uint8 *begin_addr;
        DUMP_PTR(node->begin_addr);
        // uint8 *target_addr;
        DUMP_PTR(node->target_addr);
        // uint32 *frame_sp;
        DUMP_PTR(node->frame_sp);
        // uint32 cell_num;
        fwrite(&node->cell_num, sizeof(uint32), 1, fp);
    }
}

void
dump_WASMInterpFrame(Pool_Info *addr)
{
    int i;
    HEADER(WASMInterpFrame);

    for (i = 0; i < addr->size;
         i++, node = (WASMInterpFrame *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMInterpFrame);
        fwrite(&p_abs, sizeof(int), 1, fp);

        //struct WASMInterpFrame *prev_frame;
        DUMP_PTR(node->prev_frame);

        //struct WASMFunctionInstance *function;
        DUMP_PTR(node->function);

        //uint8 *ip;
        DUMP_PTR(node->ip);

#if WASM_ENABLE_FAST_INTERP != 0
        /* return offset of the first return value of current frame.
    the callee will put return values here continuously */
        uint32 ret_offset;
        uint32 *lp;
        uint32 operand[1];
#else
        // uint32 *sp_bottom;
        DUMP_PTR(node->sp_bottom);
        // uint32 *sp_boundary;
        DUMP_PTR(node->sp_boundary);
        // uint32 *sp;
        DUMP_PTR(node->sp);

        // WASMBranchBlock *csp_bottom;
        //DUMP_PTR(node->csp_bottom);
        {
            // uint8 *begin_addr;
            DUMP_PTR(node->csp_bottom->begin_addr);
            // uint8 *target_addr;
            DUMP_PTR(node->csp_bottom->target_addr);
            // uint32 *frame_sp;
            DUMP_PTR(node->csp_bottom->frame_sp);
            // uint32 cell_num;
            fwrite(&node->csp_bottom->cell_num, sizeof(uint32), 1, fp);
        }
        // WASMBranchBlock *csp_boundary;
        //DUMP_PTR(node->csp_boundary);
        { // uint8 *begin_addr;
            DUMP_PTR(node->csp_boundary->begin_addr);
            // uint8 *target_addr;
            DUMP_PTR(node->csp_boundary->target_addr);
            // uint32 *frame_sp;
            DUMP_PTR(node->csp_boundary->frame_sp);
            // uint32 cell_num;
            fwrite(&node->csp_boundary->cell_num, sizeof(uint32), 1, fp);
        }
        // WASMBranchBlock *csp;
        //DUMP_PTR(node->csp);
        {
            // uint8 *begin_addr;
            DUMP_PTR(node->csp->begin_addr);
            // uint8 *target_addr;
            DUMP_PTR(node->csp->target_addr);
            // uint32 *frame_sp;
            DUMP_PTR(node->csp->frame_sp);
            // uint32 cell_num;
            fwrite(&node->csp->cell_num, sizeof(uint32), 1, fp);
        }

        /* Frame data, the layout is:
     lp: param_cell_count + local_cell_count
     sp_bottom to sp_boundary: stack of data
     csp_bottom to csp_boundary: stack of block
     ref to frame end: data types of local vairables and stack data
     */
        //uint32 lp[1]; 環境依存のサイズが微妙align_uint(cell_num) 要チェック
        fwrite(node->lp, sizeof(uint32),
               addr->size - (uint32)offsetof(WASMInterpFrame, lp), fp);

#endif
    }
}

void
dump_BranchBlock(Pool_Info *addr)
{
    int i;
    HEADER(BranchBlock);

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
dump_WASMLoaderContext(Pool_Info *addr)
{
    int i;
    // skip
}
void
dump_Const(Pool_Info *addr)
{
    int i;
    HEADER(Const);

    for (i = 0; i < addr->size; i++, node = (Const *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(Const);
        fwrite(&p_abs, sizeof(int), 1, fp);

        // WASMValue value;
        fwrite(&node->value.v128.f32x4, sizeof(float64), 2, fp);
        // uint16 slot_index;
        fwrite(&node->slot_index, sizeof(uint16), 1, fp);
        // uint8 value_type;
        fwrite(&node->value_type, sizeof(uint8), 1, fp);
    }
}

void
dump_WASMModuleInstance(Pool_Info *addr)
{
    int i;
    HEADER(WASMModuleInstance);

    // uint32 module_type;
    fwrite(&node->module_type, sizeof(uint32), 1, fp);

    // uint32 memory_count;
    fwrite(&node->memory_count, sizeof(uint32), 1, fp);
    // uint32 table_count;
    fwrite(&node->table_count, sizeof(uint32), 1, fp);
    // uint32 global_count;
    fwrite(&node->global_count, sizeof(uint32), 1, fp);
    // uint32 function_count;
    fwrite(&node->function_count, sizeof(uint32), 1, fp);

    // uint32 export_func_count;
    fwrite(&node->export_func_count, sizeof(uint32), 1, fp);

    // WASMMemoryInstance **memories;
    DUMP_PTR(node->memories);
    // WASMTableInstance **tables;
    DUMP_PTR(node->tables);
    // WASMGlobalInstance *globals;
    DUMP_PTR(node->globals);
    // WASMFunctionInstance *functions;
    DUMP_PTR(node->functions);

    // WASMExportFuncInstance *export_functions;
    DUMP_PTR(node->export_functions);

    // WASMMemoryInstance *default_memory;
    DUMP_PTR(node->default_memory);
    // WASMTableInstance *default_table;
    DUMP_PTR(node->default_table);
    // uint8 *global_data;
    DUMP_PTR(node->global_data);

    // WASMFunctionInstance *start_function;
    DUMP_PTR(node->start_function);
    // WASMFunctionInstance *malloc_function;
    DUMP_PTR(node->malloc_function);
    // WASMFunctionInstance *free_function;
    DUMP_PTR(node->free_function);
    // WASMFunctionInstance *retain_function;
    DUMP_PTR(node->retain_function);

    // WASMModule *module;
    DUMP_PTR(node->module);

#if WASM_ENABLE_LIBC_WASI != 0
    WASIContext *wasi_ctx;
#endif

    // WASMExecEnv *exec_env_singleton;
    DUMP_PTR(node->exec_env_singleton);

    // uint32 temp_ret;
    fwrite(&node->temp_ret, sizeof(uint32), 1, fp);
    // uint32 llvm_stack;
    fwrite(&node->llvm_stack, sizeof(uint32), 1, fp);

    // uint32 default_wasm_stack_size;
    fwrite(&node->default_wasm_stack_size, sizeof(uint32), 1, fp);

    /* The exception buffer of wasm interpreter for current thread. */
    // char cur_exception[128];
    fwrite(node->cur_exception, sizeof(char), 128, fp);

    /* The custom data that can be set/get by
     * wasm_set_custom_data/wasm_get_custom_data */
    // void *custom_data;
    DUMP_PTR(node->custom_data);
}

void
dump_WASMFunctionInstance(Pool_Info *addr)
{
    int i;
    HEADER(WASMFunctionInstance);

    for (i = 0; i < addr->size;
         i++, node = (WASMFunctionInstance *)addr->p_raw + i) {
        int p_abs = addr->p_abs + i * sizeof(WASMFunctionInstance);
        fwrite(&p_abs, sizeof(int), 1, fp);

        /* whether it is import function or WASM function */
        bool is_import_func;
        /* parameter count */
        uint16 param_count;
        /* local variable count, 0 for import function */
        uint16 local_count;
        /* cell num of parameters */
        uint16 param_cell_num;
        /* cell num of return type */
        uint16 ret_cell_num;
        /* cell num of local variables, 0 for import function */
        uint16 local_cell_num;
#if WASM_ENABLE_FAST_INTERP != 0
        /* cell num of consts */
        uint16 const_cell_num;
#endif
        uint16 *local_offsets;
        /* parameter types */
        uint8 *param_types;
        /* local types, NULL for import function */
        uint8 *local_types;
        union {
            WASMFunctionImport *func_import;
            WASMFunction *func;
        } u;
    }
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
