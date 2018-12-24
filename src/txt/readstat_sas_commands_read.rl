#include <stdlib.h>

#include "../readstat.h"
#include "readstat_schema.h"
#include "readstat_sas_commands_read.h"

#include "readstat_copy.h"

enum {
    LABEL_TYPE_NAN = -1,
    LABEL_TYPE_DOUBLE,
    LABEL_TYPE_STRING,
    LABEL_TYPE_RANGE
};

static readstat_error_t submit_columns(readstat_parser_t *parser, readstat_schema_t *dct, void *user_ctx) {
    if (!parser->handlers.variable)
        return READSTAT_OK;
    int i;
    int partial_entry_count = 0;
    for (i=0; i<dct->entry_count; i++) {
        readstat_schema_entry_t *entry = &dct->entries[i];
        entry->variable.index = i;
        entry->variable.index_after_skipping = partial_entry_count;
        int cb_retval = parser->handlers.variable(i, &entry->variable,
            entry->labelset[0] ? entry->labelset : NULL, user_ctx);
        if (cb_retval == READSTAT_HANDLER_SKIP_VARIABLE) {
            entry->skip = 1;
        } else if (cb_retval == READSTAT_HANDLER_ABORT) {
            return READSTAT_ERROR_USER_ABORT;
        } else {
            partial_entry_count++;
        }
    }
    return READSTAT_OK;
}

static readstat_schema_entry_t *find_or_create_entry(readstat_schema_t *dct, const char *var_name) {
    readstat_schema_entry_t *entry = NULL;
    int i;
    /* linear search. this is shitty, but whatever */
    for (i=0; i<dct->entry_count; i++) {
        if (strcmp(dct->entries[i].variable.name, var_name) == 0) {
            entry = &dct->entries[i];
            break;
        }
    }
    if (!entry) {
        dct->entries = realloc(dct->entries, sizeof(readstat_schema_entry_t) * (dct->entry_count + 1));
        entry = &dct->entries[dct->entry_count];
        memset(entry, 0, sizeof(readstat_schema_entry_t));
        
        readstat_copy(entry->variable.name, sizeof(entry->variable.name),
            (unsigned char *)var_name, strlen(var_name));
        entry->decimal_separator = '.';
        
        dct->entry_count++;
    }
    return entry;
}

%%{
    machine sas_commands;
    write data noerror nofinal;
}%%

readstat_schema_t *readstat_parse_sas_commands(readstat_parser_t *parser,
    const char *filepath, void *user_ctx, readstat_error_t *outError) {
    /* TODO use the schema_entry data structure, and access it
     * with a hash table (or linear search if we're lazy)
     */
    readstat_schema_t *schema = NULL;
    unsigned char *bytes = NULL;
    readstat_error_t error = READSTAT_OK;
    ssize_t len = parser->io->seek(0, READSTAT_SEEK_END, parser->io->io_ctx);
    if (len == -1) {
        error = READSTAT_ERROR_SEEK;
        goto cleanup;
    }
    parser->io->seek(0, READSTAT_SEEK_SET, parser->io->io_ctx);

    bytes = malloc(len);

    parser->io->read(bytes, len, parser->io->io_ctx);

    unsigned char *p = bytes;
    unsigned char *pe = bytes + len;
    
    unsigned char *eof = pe;
    
    unsigned char *str_start = NULL;

    size_t str_len = 0;
    
    int cs;
    
    int first_integer = 0;
    int integer = 0;
    int line_no = 0;
    u_char *line_start = p;

    char varname[32];
    char argname[32];
    char labelset[32];
    char string_value[32];
    char buf[1024];

    readstat_type_t var_type = READSTAT_TYPE_DOUBLE;
    int label_type;
    int var_row = 0, var_col = 0;
    int var_len = 0;

    if ((schema = calloc(1, sizeof(readstat_schema_t))) == NULL) {
        error = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    schema->rows_per_observation = 1;
    
    %%{
        action start_integer {
            integer = 0;
        }
        
        action incr_integer {
            integer = 10 * integer + (fc - '0');
        }

        action incr_hex_integer {
            int value = 0;
            if (fc >= '0' && fc <= '9') {
                value = fc - '0';
            } else if (fc >= 'A' && fc <= 'F') {
                value = fc - 'A' + 10;
            } else if (fc >= 'a' && fc <= 'f') {
                value = fc - 'a' + 10;
            }
            integer = 16 * integer + value;
        }

        action copy_pos {
            var_col = integer - 1;
            var_len = 1;
        }

        action set_len {
            var_len = integer - var_col;
        }
                                       
        action set_str {
            var_type = READSTAT_TYPE_STRING;
        }
                                       
        action set_dbl {
            var_type = READSTAT_TYPE_DOUBLE;
        }

        action copy_buf {
            readstat_copy(buf, sizeof(buf), str_start, str_len);
        }

        action copy_labelset {
            readstat_copy(labelset, sizeof(labelset), str_start, str_len);
        }

        action copy_string {
            readstat_copy(string_value, sizeof(string_value), str_start, str_len);
        }

        action copy_argname {
            readstat_copy(argname, sizeof(argname), str_start, str_len);
        }

        action copy_varname {
            readstat_copy_lower(varname, sizeof(varname), str_start, str_len);
        }

        action handle_arg {
            if (strcasecmp(argname, "firstobs") == 0) {
                schema->first_line = integer;
            }
            if (strcasecmp(argname, "dlm") == 0) {
                schema->field_delimiter = integer ? integer : buf[0];
            }
        }

        action handle_var {
            readstat_schema_entry_t *entry = find_or_create_entry(schema, varname);
            entry->variable.type = var_type;
            entry->row = var_row;
            entry->col = var_col;
            entry->len = var_len;
        }

        action handle_var_len {
            readstat_schema_entry_t *entry = find_or_create_entry(schema, varname);
            entry->len = var_len;
        }

        action handle_var_label {
            readstat_schema_entry_t *entry = find_or_create_entry(schema, varname);
            readstat_copy(entry->variable.label, sizeof(entry->variable.label),
                (unsigned char *)buf, sizeof(buf));
        }

        action handle_var_labelset {
            readstat_schema_entry_t *entry = find_or_create_entry(schema, varname);
            readstat_copy(entry->labelset, sizeof(entry->labelset),
                (unsigned char *)labelset, sizeof(labelset));
        }

        action handle_value_label {
            if (parser->handlers.value_label) {
                if (label_type == LABEL_TYPE_RANGE) {
                    int i;
                    for (i=first_integer; i<=integer; i++) {
                        readstat_value_t value = { 
                            .type = READSTAT_TYPE_DOUBLE,
                            .v = { .double_value = i } };
                        parser->handlers.value_label(labelset, value, buf, user_ctx);
                    }
                } else {
                    readstat_value_t value = { { 0 } };
                    if (label_type == LABEL_TYPE_DOUBLE) {
                        value.type = READSTAT_TYPE_DOUBLE;
                        value.v.double_value = integer;
                    } else if (label_type == LABEL_TYPE_STRING) {
                        value.type = READSTAT_TYPE_STRING;
                        value.v.string_value = string_value;
                    } else if (label_type == LABEL_TYPE_NAN) {
                        value.type = READSTAT_TYPE_DOUBLE;
                        value.v.double_value = NAN;
                    }

                    parser->handlers.value_label(labelset, value, buf, user_ctx);
                }
            }
        }
        
        single_quoted_string = "'" ( [^']* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; } "'";
        
        double_quoted_string = "\"" ( [^"]* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; } "\"";

        unquoted_string = [A-Za-z] [_A-Za-z0-9\.]*;
        
        quoted_string = ( single_quoted_string | double_quoted_string ) %copy_buf;

        hex_string = "'" ( [0-9A-Fa-f]+ ) >start_integer $incr_hex_integer "'x";
        
        newline = ( "\n" | "\r\n" ) %{ line_no++; line_start = p; };
                                       
        missing_value = "." [A-Z]?;
        
        identifier = ( [$_A-Za-z] [_A-Za-z0-9]* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; };

        identifier_eval = "&"? identifier "."?;
        
        integer = [0-9]+ >start_integer $incr_integer;
        
        true_whitespace = [ \t] | newline;
                                       
        multiline_comment = "/*" ( any* - ( any* "*/" any* ) ) "*/";

        comment = "*" ( any* - ( any* ";" true_whitespace* newline any* ) ) ";" true_whitespace* newline |
            multiline_comment;
                                       
        whitespace = true_whitespace | multiline_comment;
        
        var = identifier %copy_varname;

        labelset = identifier %copy_labelset;

        arg = identifier %copy_argname (whitespace* "=" whitespace* (identifier_eval | quoted_string | hex_string | integer) >start_integer %handle_arg)?;

        args = arg ( whitespace+ arg)*;

        options_cmd = "OPTIONS"i whitespace+ args
            whitespace* ";";

        let_macro = "%LET"i whitespace+ identifier whitespace* "=" whitespace* (unquoted_string | quoted_string) 
            whitespace* ";";

        libname_cmd = "LIBNAME"i whitespace+ identifier whitespace+ ( 
                quoted_string (whitespace+ args)? | 
                "CLEAR"i | "_ALL_"i whitespace* "CLEAR"i |
                "LIST"i | "_ALL_"i whitespace* "LIST"i
                ) whitespace* ";";
                                       
        footnote_cmd = "FOOTNOTE"i whitespace+ quoted_string whitespace* ";";

        empty_cmd = ";";

        value_label = ( "-" integer %{ label_type = LABEL_TYPE_DOUBLE; integer *= -1; } |
                integer %{ label_type = LABEL_TYPE_DOUBLE; } |
                integer whitespace+ "-" whitespace+ %{ first_integer = integer; label_type = LABEL_TYPE_RANGE; } integer |
                unquoted_string %{ label_type = LABEL_TYPE_STRING; } %copy_string |
                quoted_string %{ label_type = LABEL_TYPE_STRING; } %copy_string
                ) whitespace* "=" whitespace* quoted_string %handle_value_label;

        var_len = ("$" whitespace* integer %set_str | integer %set_dbl) %{ var_len = integer; };

        value_cmd = "VALUE"i whitespace+ labelset whitespace+ ("(" args ")" whitespace*)?
            value_label (whitespace+ value_label)* 
            whitespace* ";";
        
        proc_format_cmd = "PROC"i whitespace+ "FORMAT"i whitespace* ( args whitespace* )? ";"
            ( whitespace | empty_cmd | value_cmd )+;

        filename_cmd = "FILENAME"i (whitespace+ args)? whitespace+ quoted_string
            whitespace* ";";

        if_statement = "IF"i ( whitespace | identifier | "-"? integer | "(" | ")" | ".")+ ";";

        data_cmd = "DATA"i (whitespace+ identifier_eval | unquoted_string | quoted_string )+
            whitespace* ";";

        missing_cmd = "MISSING"i whitespace+ identifier 
            whitespace* ";";

        # lrecl_option = "LRECL"i whitespace* "=" whitespace* integer %handle_info;

        infile_cmd = "INFILE"i (whitespace+ quoted_string)? (whitespace* args)?
            whitespace* ";";

        length_spec = var whitespace+ var_len %handle_var_len;

        length_cmd = "LENGTH"i whitespace+ length_spec (whitespace+ length_spec)*
            whitespace* ";";

        label_spec = var whitespace* "=" whitespace* quoted_string %handle_var_label;

        label_cmd = "LABEL"i whitespace+ label_spec (whitespace+ label_spec)*
            whitespace* ";";
                                       
        date_separator = [SN];
                                       
        date_format = ( "MMDDYY" integer |
                       "DATE" |
                       "DATE9" |
                       "DATETIME" |
                       "DAY" |
                       "DDMMYY" date_separator? integer |
                       "DOWNAME" |
                       "JULDAY" |
                       "JULIAN" |
                       "MMDDYY" date_separator? integer |
                       "MMYY" date_separator? |
                       "MONNAME" |
                       "MONTH" |
                       "MONYY" |
                       "PDFJULG" |
                       "WEEKDATE" |
                       "WEEKDAY" |
                       "WORDDATE" |
                       "WORDDATX" |
                       "QTR" |
                       "QTRR" |
                       "TIME" |
                       "TIMEAMPM" |
                       "TOD" |
                       "YEAR" |
                       "YYMMDD" |
                       "YYMM" date_separator? |
                       "YYQ" date_separator? |
                       "YYQR" date_separator? );
                       
                                       
        format_lbl_spec = labelset "." %handle_var_labelset;
                                       
        format_dbl_spec = integer "." integer?;
                                       
        format_date_spec = date_format "." integer?;
                                       
        var_format_spec = var whitespace+ ( format_lbl_spec | format_dbl_spec | format_date_spec );

        format_cmd = "FORMAT"i whitespace+ var_format_spec (whitespace+ var_format_spec)*
            whitespace* ";";
                                       
        var_attribute = (
                        "LENGTH"i whitespace* "=" whitespace* var_len %handle_var_len |
                        "LABEL"i whitespace* "=" whitespace* quoted_string %handle_var_label |
                        "FORMAT"i whitespace* "=" whitespace* format_dbl_spec
                        );
       
        var_attributes = var_attribute (whitespace+ var_attribute)*;
       
        attrib_spec = var whitespace+ var_attributes %handle_var;
       
        attrib_cmd = "ATTRIB"i whitespace+ attrib_spec (whitespace+ attrib_spec)* whitespace* ";";
                                       
        input_format_spec = ("$CHAR" integer %set_str | identifier %set_dbl);

        input_int_spec = var whitespace+ integer %copy_pos "-" integer %set_len %set_dbl %handle_var;
                                       
        input_dbl_spec = "@" integer %copy_pos whitespace+ var whitespace+ (var_len | input_format_spec) "." %handle_var integer?;

        input_txt_spec = var whitespace+ "$" whitespace+ integer %copy_pos "-" integer %set_len %set_str %handle_var;

        row_spec = "#" integer %{ var_row = integer - 1; };
                                       
        input_spec = (input_int_spec | input_dbl_spec | input_txt_spec | row_spec | var);

        input_cmd = "INPUT"i whitespace+ %{ var_row = 0; } input_spec (whitespace+ input_spec)*
            whitespace* ";";
                                       
        invalue_missing_spec = single_quoted_string whitespace* "=" whitespace* missing_value;
        
        invalue_format_spec = format_dbl_spec | format_date_spec;
        
        invalue_other_spec = "OTHER" whitespace* "=" whitespace* "(|" invalue_format_spec "|)";
                                       
        invalue_spec = invalue_missing_spec | invalue_other_spec;
                                       
        invalue_cmd = "INVALUE"i whitespace+ identifier whitespace+ invalue_spec (whitespace+ invalue_spec)* whitespace* ";";

        proc_print_cmd = "PROC"i whitespace+ "PRINT"i (whitespace+ args) (whitespace+ "(" args ")")? 
            whitespace* ";";

        proc_contents_cmd = "PROC"i whitespace+ "CONTENTS"i (whitespace+ args) whitespace* ";";

        run_cmd = "RUN"i whitespace* ";";

        command = 
            options_cmd |
            let_macro |
            libname_cmd |
            footnote_cmd |
            value_cmd |
            proc_format_cmd |
            filename_cmd |
            attrib_cmd |
            data_cmd |
            if_statement |
            missing_cmd |
            infile_cmd |
            format_cmd |
            label_cmd |
            length_cmd |
            input_cmd |
            invalue_cmd |
            proc_print_cmd |
            proc_contents_cmd |
            run_cmd;
        
        main := ( true_whitespace | comment | command )*;

        write init;
        write exec;
    }%%
                                       
   /* suppress warnings */
   (void)sas_commands_en_main;

    if (cs < %%{ write first_final; }%%) {
        char error_buf[1024];
        snprintf(error_buf, sizeof(error_buf), "Error parsing .sas file around line #%d, col #%ld (%c)",
            line_no + 1, (long)(p - line_start + 1), *p);
        if (parser->handlers.error) {
            parser->handlers.error(error_buf, user_ctx);
        }
        error = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
    schema->rows_per_observation = var_row + 1;

    error = submit_columns(parser, schema, user_ctx);

cleanup:
    parser->io->close(parser->io->io_ctx);
    free(bytes);
    if (error != READSTAT_OK) {
        if (outError)
            *outError = error;
        readstat_schema_free(schema);
        schema = NULL;
    }

    return schema;
}
