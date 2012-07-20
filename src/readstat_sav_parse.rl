
#include "readstat_sav.h"
#include "readstat_sav_parse.h"
#include <stdlib.h>

typedef struct varlookup {
    char      name[9];
    int       index;
} varlookup_t;

static int compare_key_varlookup(const void *elem1, const void *elem2) {
    const char *key = (const char *)elem1;
    const varlookup_t *v = (const varlookup_t *)elem2;
    return strcmp(key, v->name);
}

static int compare_varlookups(const void *elem1, const void *elem2) {
    const varlookup_t *v1 = (const varlookup_t *)elem1;
    const varlookup_t *v2 = (const varlookup_t *)elem2;
    return strcmp(v1->name, v2->name);
}

%%{
    machine sav_long_variable_parse;
    write data;
}%%

int sav_parse_long_variable_names_record(void *data, int count, sav_ctx_t *ctx) {
    varlookup_t *table = malloc(ctx->var_index * sizeof(varlookup_t));
    int i;
    int retval = 0;
    int var_count = ctx->var_index;
    for (i=0; i<var_count; i++) {
        memcpy(table[i].name, ctx->varinfo[i].name, 9);
        table[i].index = ctx->varinfo[i].index;
    }
    qsort(table, var_count, sizeof(varlookup_t), &compare_varlookups);
        
    char temp_key[9];
    char temp_val[65];
    int key_offset = 0;
    int val_offset = 0;
    
    u_char *p = (u_char *)data;
    u_char *pe = (u_char *)data + count;
    u_char *eof = pe;

    int cs;

    %%{
        action set_long_name {
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                memcpy(ctx->varinfo[found->index].longname, temp_val, val_offset);
            } else {
                printf("Failed to find %s\n", temp_key);
            }
        }
        
        key = ( [A-Z@] [A-Z0-9@#$_\.]{0,7} )  >{ key_offset = 0; } ${ temp_key[key_offset++] = fc; } %{ temp_key[key_offset++] = '\0'; };
        
        value = graph{1,64} >{ val_offset = 0; } ${ temp_val[val_offset++] = fc; } %{ temp_val[val_offset++] = '\0'; };
        
        keyval = ( key "=" value ) %set_long_name;
        
        main := keyval ("\t" keyval)*;
        
        write init;
        write exec;
    }%%

    if (cs < sav_long_variable_parse_first_final || p != pe) {
        printf("Parse error around byte #%ld, character %d\n", p - (u_char *)data, *p);
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    return retval;
}

%%{
    machine sav_very_long_string_parse;
    write data;
}%%

int sav_parse_very_long_string_record(void *data, int count, sav_ctx_t *ctx) {
    varlookup_t *table = malloc(ctx->var_index * sizeof(varlookup_t));
    int i;
    int retval = 0;
    int var_count = ctx->var_index;
    for (i=0; i<var_count; i++) {
        memcpy(table[i].name, ctx->varinfo[i].name, 9);
        table[i].index = ctx->varinfo[i].index;
    }
    qsort(table, var_count, sizeof(varlookup_t), &compare_varlookups);
    
    char temp_key[9];
    int temp_val;
    int key_offset = 0;
    
    u_char *p = (u_char *)data;
    u_char *pe = (u_char *)data + count;
    
    int cs;
    
    %%{
        action set_width {
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                ctx->varinfo[found->index].string_length = temp_val;
            }
        }
        
        action incr_val {
            if (fc != '\0') { 
                temp_val = 10 * temp_val + (fc - '0'); 
            }
        }
        
        key = ( [A-Z@] [A-Z0-9@#$_\.]{0,7} )  >{ key_offset = 0; } ${ temp_key[key_offset++] = fc; } %{ temp_key[key_offset++] = '\0'; };
        
        value = [0-9]+ >{ temp_val = 0; } $incr_val;
        
        keyval = ( key "=" value ) %set_width;
        
        main := keyval ("\0"+ "\t" keyval)* "\0"+ "\t"?;
        
        write init;
        write exec;
    }%%
    
    if (cs < sav_very_long_string_parse_first_final || p != pe) {
        printf("Parsed %ld of %ld bytes\n", p - (u_char *)data, pe - (u_char *)data);
        printf("Remaining bytes: %s\n", p);
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    return retval;
}
