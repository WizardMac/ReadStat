
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "../readstat.h"
#include "readstat_schema.h"
#include "readstat_txt_read.h"

static int handle_value(int obs_index, int var_index, char *value, size_t len, void *ctx) {
    return 0;
}

static size_t txt_getdelim(char ** restrict linep, size_t * restrict linecapp,
        int delimiter, readstat_io_t *io) {
    char *value_buffer = *linep;
    size_t value_buffer_len = *linecapp;
    size_t i = 0;
    while (io->read(&value_buffer[i], 1, io->io_ctx) == 1 && value_buffer[i++] != delimiter) {
        if (i == value_buffer_len) {
            value_buffer = realloc(value_buffer, value_buffer_len *= 2);
        }
    }
    *linep = value_buffer;
    *linecapp = value_buffer_len;
    return i;
}

static readstat_error_t txt_parse_delimited(readstat_parser_t *parser,
        readstat_schema_t *schema, void *user_ctx) {
    size_t value_buffer_len = 4096;
    char *value_buffer = malloc(value_buffer_len);
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;
    int k=0;
    
    while (1) {
        int done = 0;
        for (int j=0; j<schema->entry_count; j++) {
            int delimiter = (j == schema->entry_count-1) ? '\n' : schema->field_delimiter;
            size_t chars_read = txt_getdelim(&value_buffer, &value_buffer_len, delimiter, io);
            if (chars_read == -1) {
                done = 1;
                break;
            }
            chars_read--; // delimiter
            if (chars_read > 0 && value_buffer[chars_read-1] == '\r') {
                chars_read--; // CRLF
            }
            value_buffer[chars_read] = '\0';
            if (handle_value(k, j, value_buffer, chars_read, user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
        k++;
        if (done)
            break;
    }
    
cleanup:
    
    if (value_buffer)
        free(value_buffer);
    
    return retval;
}

static readstat_error_t txt_parse_fixed_width(readstat_parser_t *parser,
        readstat_schema_t *schema, void *user_ctx, const size_t *line_lens, char *line_buffer) {
    char   value_buffer[4096];
    
    readstat_io_t *io = parser->io;
    readstat_error_t retval = READSTAT_OK;
    int k=0;
    while (1) {
        int j=0;
        for (int i=0; i<schema->rows_per_observation; i++) {
            size_t bytes_read = io->read(line_buffer, line_lens[i], io->io_ctx);
            if (bytes_read == 0)
                goto cleanup;
            
            if (bytes_read < line_lens[i]) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            for (; j<schema->entry_count && schema->entries[j].row == i; j++) {
                size_t field_len = schema->entries[j].len;
                size_t field_offset = schema->entries[j].col;
                if (field_len < sizeof(value_buffer)) {
                    memcpy(value_buffer, &line_buffer[field_offset], field_len);
                    value_buffer[field_len] = '\0';
                    if (handle_value(k, j, value_buffer, field_len, user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        goto cleanup;
                    }
                }
            }
            
            if (schema->cols_per_observation == 0) {
                char throwaway = '\0';
                while (io->read(&throwaway, 1, io->io_ctx) == 1 && throwaway != '\n');
            }
        }
        
        k++;
    }
cleanup:
    return retval;
}

readstat_error_t readstat_parse_txt(readstat_parser_t *parser, const char *filename, 
        readstat_schema_t *schema, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;
    int i;
        
    size_t *line_lens = NULL;
    
    size_t line_buffer_len = 0;
    char  *line_buffer = NULL;
    
    if (io->open(filename, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }
    
    if ((line_lens = malloc(schema->rows_per_observation * sizeof(size_t))) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    for (i=0; i<schema->rows_per_observation; i++) {
        line_lens[i] = schema->cols_per_observation;
    }
    
    for (i=0; i<schema->entry_count; i++) {
        readstat_schema_entry_t *entry = &schema->entries[i];
        if (line_lens[entry->row] < entry->col + entry->len)
            line_lens[entry->row] = entry->col + entry->len;
    }
    
    for (i=0; i<schema->rows_per_observation; i++) {
        if (line_buffer_len < line_lens[i])
            line_buffer_len = line_lens[i];
    }
    
    line_buffer_len += 2; /* CRLF */
    
    if ((line_buffer = malloc(line_buffer_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    if (schema->first_line > 1) {
        int throwaway_lines = schema->first_line - 1;
        char throwaway_char = '\0';
        
        while (throwaway_lines--) {
            while (io->read(&throwaway_char, 1, io->io_ctx) == 1 && throwaway_char != '\n');
        }
    }
    
    if (schema->field_delimiter) {
        retval = txt_parse_delimited(parser, schema, user_ctx);
    } else {
        retval = txt_parse_fixed_width(parser, schema, user_ctx, line_lens, line_buffer);
    }
    
cleanup:
    
    io->close(io->io_ctx);
    if (line_buffer)
        free(line_buffer);
    if (line_lens)
        free(line_lens);
    
    return retval;
}

