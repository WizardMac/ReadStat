#include <stdlib.h>

#include "../readstat.h"
#include "readstat_schema.h"
#include "readstat_dct_read.h"

%%{
    machine stata_dictionary;
    write data noerror nofinal;
}%%

readstat_schema_t *readstat_parse_stata_dictionary(readstat_parser_t *parser,
    const u_char *bytes, size_t len, int *error_line_number) {
    u_char *p = (u_char *)bytes;
    u_char *pe = (u_char *)bytes + len;

    u_char *str_start = NULL;

    size_t str_len = 0;

    int cs;
//    u_char *eof = pe;

    int integer = 0;
    int current_row = 0;
    int current_col = 0;
    int line_no = 0;
    u_char *line_start = p;

    readstat_schema_entry_t current_entry;
    
    readstat_schema_t *schema = NULL;
    if ((schema = malloc(sizeof(readstat_schema_t))) == NULL) {
        return NULL;
    }

    schema->filename[0] = '\0';
    schema->rows_per_observation = 1;
    schema->cols_per_observation = 0;
    schema->first_line = 0;
    schema->entries = NULL;
    schema->entry_count = 0;
    
    %%{
        action start_integer {
            integer = 0;
        }

        action incr_integer {
            integer = 10 * integer + (fc - '0');
        }

        action start_entry {
            memset(&current_entry, 0, sizeof(readstat_schema_entry_t));
            current_entry.decimal_separator = '.';
            current_entry.type = READSTAT_TYPE_DOUBLE;
        }

        action end_entry {
            current_entry.row = current_row;
            current_entry.col = current_col;
            current_col += current_entry.len;
            schema->entries = realloc(schema->entries, sizeof(readstat_schema_entry_t) * (schema->entry_count+1));
            memcpy(&schema->entries[schema->entry_count++], &current_entry, sizeof(readstat_schema_entry_t));
        }

        action copy_filename {
            if (str_len < sizeof(schema->filename)) {
                memcpy(schema->filename, str_start, str_len);
                schema->filename[str_len] = '\0';
            }
        }

        action copy_varname {
            if (str_len < sizeof(current_entry.varname)) {
                memcpy(current_entry.varname, str_start, str_len);
                current_entry.varname[str_len] = '\0';
            }
        }

        action copy_varlabel {
            if (str_len < sizeof(current_entry.varlabel)) {
                memcpy(current_entry.varlabel, str_start, str_len);
                current_entry.varlabel[str_len] = '\0';
            }
        }

        quoted_string = "\"" ( [^"]* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; } "\"";

        unquoted_string = [A-Za-z0-9_/\\\.\-]+ >{ str_start = fpc; } %{ str_len = fpc - str_start; };

        identifier = ( [A-Za-z] [_\.A-Za-z0-9]* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; };

        newline = ( "\n" | "\r\n" ) %{ line_no++; line_start = p; };
        
        spacetab = [ \t];
        
        whitespace = spacetab | newline;

        filename = ( quoted_string | unquoted_string ) %copy_filename;

        integer = [0-9]+ >start_integer $incr_integer;

        lines_marker = "_lines(" spacetab* integer spacetab* ")" %{ schema->rows_per_observation = integer; };

        line_marker = "_line(" spacetab* integer spacetab* ")" %{ current_row = integer - 1; };

        column_marker = "_column(" spacetab* integer spacetab* ")" %{ current_col = integer - 1; };

        newline_marker = "_newline" %{ current_row++; } ( "(" spacetab* integer spacetab* ")" %{ current_row += (integer - 1); } )?;

        skip_marker = "_skip(" spacetab* integer spacetab* ")" %{ current_col += (integer - 1) };

        lrecl_marker = "_lrecl(" spacetab* integer spacetab* ")" %{ schema->cols_per_observation = integer; };

        firstlineoffile_marker = "_firstlineoffile(" spacetab* integer spacetab* ")" %{ schema->first_line = integer - 1; };

        marker = lrecl_marker | firstlineoffile_marker | lines_marker | line_marker | column_marker | newline_marker;

        type = "byte" %{ current_entry.type = READSTAT_TYPE_INT8; }
            | "int" %{ current_entry.type = READSTAT_TYPE_INT16; }
            | "long" %{ current_entry.type = READSTAT_TYPE_INT32; }
            | "float" %{ current_entry.type = READSTAT_TYPE_FLOAT; }
            | "double" %{ current_entry.type = READSTAT_TYPE_DOUBLE; }
            | "str" integer %{ current_entry.type = READSTAT_TYPE_STRING; };

        varname = identifier %copy_varname;

        varlabel = quoted_string %copy_varlabel;

        format = "%" integer %{ current_entry.len = integer; } 
        ( "s" | "S" | ( ( ( "." | "," %{ current_entry.decimal_separator = ','; } ) integer )? ( "f" | "g" | "e" ) ) );

        entry = ( ( type spacetab+ )? varname ( spacetab+ format )? ( spacetab+ varlabel )? spacetab* newline ) >start_entry %end_entry;

        comment = "*" [^\r\n]* newline | "/*" ( any* - ( any* "*/" any* ) ) "*/";

        contents = ( whitespace* ( marker | entry | comment ) )* whitespace*;

        main := comment* ("infile" whitespace+)? "dictionary" whitespace+ 
                ( "using" whitespace+ filename whitespace+ )?
                                "{" contents "}" any*;

        write init;
        write exec;
    }%%

    /* suppress warnings */
    (void)stata_dictionary_en_main;

    if (cs < %%{ write first_final; }%%) {
        char error_buf[1024];
        snprintf(error_buf, sizeof(error_buf), "Error parsing .dct file around line #%d, col #%ld (%c)",
            line_no + 1, (long)(p - line_start + 1), *p);
        if (parser->handlers.error) {
            parser->handlers.error(error_buf, NULL);
        }
        readstat_schema_free(schema);
        if (error_line_number)
            *error_line_number = line_no + 1;
        
        return NULL;
    }

    return schema;
}
