typedef int (*rs_mod_will_write_file)(const char *filename);
typedef void * (*rs_mod_ctx_init)(const char *filename);
typedef void (*rs_mod_finish_file)(void *ctx);

typedef struct rs_module_s {
    rs_mod_will_write_file      accept;
    rs_mod_ctx_init             init;
    rs_mod_finish_file          finish;
    readstat_info_handler       handle_info;
    readstat_metadata_handler   handle_metadata;
    readstat_note_handler       handle_note;
    readstat_variable_handler   handle_variable;
    readstat_fweight_handler    handle_fweight;
    readstat_value_handler      handle_value;
    readstat_value_label_handler handle_value_label;
} rs_module_t;

