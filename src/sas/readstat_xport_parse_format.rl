
#include "../readstat.h"
#include "readstat_xport_parse_format.h"

%%{
    machine xport_format_parse;
    write data nofinal noerror;
}%%

readstat_error_t xport_parse_format(const char *data, size_t len,
        char *name, size_t name_len, int *width, int *decimals,
        readstat_error_handler error_handler, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    const char *p = data;
    const char *pe = p + len;
    const char *eof = pe;
    int cs;
    unsigned int temp_val = 0;
    size_t parsed_len = 0;
    %%{
        action incr_val {
            temp_val = 10 * temp_val + (fc - '0');
        }

        action write_name {
            parsed_len = p - data;
            if (parsed_len < name_len) {
                memcpy(name, data, parsed_len);
                name[parsed_len] = '\0';
            }
        }

        integer = [0-9]+ >{ temp_val = 0; } $incr_val;

        name = [A-Za-z_] ([A-Za-z0-9_]+ [A-Za-z_a])?;

        char_name = '$' name? %write_name;
        dbl_name = name? %write_name;

        width = integer %{ *width = temp_val; };
        decimals = integer %{ *decimals = temp_val; };

        main := (char_name width? '.'?) | (dbl_name? (width? ('.' decimals?)?)?);

        write init;
        write exec;
    }%%

    if (cs < %%{ write first_final; }%%|| p != pe || parsed_len + 1 > name_len) {
        char error_buf[1024];
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Invalid format string (length=%d): %.*s", (int)len, (int)len, data);
            error_handler(error_buf, user_ctx);
        }
        retval = READSTAT_ERROR_BAD_FORMAT_STRING;
    }

    (void)xport_format_parse_en_main;

    return retval;
}
