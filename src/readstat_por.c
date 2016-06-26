#include <stdlib.h>

#include "readstat.h"
#include "readstat_spss.h"
#include "CKHashTable.h"
#include "readstat_por.h"

uint16_t por_unicode_lookup[256] = {
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '1', '2', '3', '4', '5', 
    '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z', ' ', '.', '<', '(',
    '+', '|', '&', '[', ']', '!', '$', '*', ')', ';',
    '^', '-', '/', 0x00A3, ',', '%', '_', '>', '?', 0x2018,
    ':', 0x00A6, '@', 0x2019, '=', '"', 0x2264, 0x25A1, 0x00B1, 0x25A0,
    0x00B0, 0x2020, '~', 0x2013, 0x2514, 0x250C, 0x2265, 0x2070, 0x2071, 0x00B2,
    0x00B3, 0x2074, 0x2075, 0x2076, 0x2077, 0x2078, 0x2079, 0x2518, 0x2510, 0x2260,
    0x2014, 0x207D, 0x207E, 0x2E38, '{', '}', '\\', 0x00A2, 0x2022, '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0' };

por_ctx_t *por_ctx_init() {
    por_ctx_t *ctx = calloc(1, sizeof(por_ctx_t));

    ctx->space = ' ';
    ctx->var_dict = ck_hash_table_init(1024);
    return ctx;
}

void por_ctx_free(por_ctx_t *ctx) {
    if (ctx->string_buffer)
        free(ctx->string_buffer);
    if (ctx->varinfo) {
        int i;
        for (i=0; i<ctx->var_count; i++) {
            if (ctx->varinfo[i].label)
                free(ctx->varinfo[i].label);
        }
        free(ctx->varinfo);
    }
    if (ctx->var_dict)
        ck_hash_table_free(ctx->var_dict);
    free(ctx);
}

size_t por_utf8_encode(const unsigned char *input, size_t input_len, 
        char *output, size_t output_len, uint16_t lookup[256]) {
    int offset = 0;
    int i;
    for (i=0; i<input_len; i++) {
        uint16_t codepoint = lookup[input[i]];
        if (codepoint <= 0x7F) {
            if (offset + 1 > output_len)
                return offset;
            
            output[offset++] = codepoint;
        } else {
            if (codepoint <= 0x07FF) {
                if (offset + 2 > output_len)
                    return offset;
            } else /* if (codepoint <= 0xFFFF) */{
                if (offset + 3 > output_len)
                    return offset;
            }
            int printed = sprintf(output + offset, "%lc", codepoint);
            if (printed > 0) {
                offset += printed;
            } else {
                offset++;
            }
        }
    }
    return offset;
}

