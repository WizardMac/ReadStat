
#include "readstat.h"
#include "readstat_por_parse.h"
#include <unistd.h>

%%{
    machine por_field_parse;
    write data;
}%%

int readstat_por_parse_double(const char *data, size_t len, double *result, readstat_error_handler error_cb) {
    int retval = 0;
    double val = 0.0;
    long num = 0;
    long frac = 0;
    long exp = 0;
    
    long temp_val = 0;
    long frac_len = 0;
    
    const unsigned char *val_start = NULL;
    
    const unsigned char *p = (const unsigned char *)data;
    const unsigned char *eof = p + len;
    
    int cs;
    int is_negative = 0, exp_is_negative = 0;
    int success = 0;
    
    %%{
        action incr_val {
            if (fc >= '0' && fc <= '9') {
                temp_val = 30 * temp_val + (fc - '0');
            } else if (fc >= 'A' && fc <= 'T') {
                temp_val = 30 * temp_val + (10 + fc - 'A');
            }
        }
        
        value = [0-9A-T]+ >{ temp_val = 0; val_start = fpc; } $incr_val;

        fraction = "." value %{ frac = temp_val; frac_len = (fpc - val_start); };
        
        nonmissing_value = (("-" %{ is_negative = 1; })? value %{ num = temp_val; } fraction?
                            ( ("+" | "-" %{ exp_is_negative = 1; }) value %{ exp = temp_val; })?) "/";

        nonmissing_fraction = ("-" %{ is_negative = 1; })? fraction "/";
        
        missing_value = "*." %{ val = NAN; };
        
        main := " "* (missing_value | nonmissing_value | nonmissing_fraction ) @{ success = 1; fbreak; };
        
        write init;
        write exec noend;
    }%%

    val = 1.0 * num;
    if (frac_len)
        val += frac / pow(30.0, frac_len);
    if (exp_is_negative)
        exp *= -1;
    if (exp) {
        val *= pow(10.0, exp);
    }
    if (is_negative)
        val *= -1;
    
    if (!success) {
        retval = -1;
        if (error_cb) {
            char error_buf[1024];
            snprintf(error_buf, sizeof(error_buf), "Read bytes: %ld Ending state: %d\n", (long)(p - (const unsigned char *)data), cs);
            error_cb(error_buf);
        }
    }
    
    if (retval == 0) {
        if (result)
            *result = val;
        
        retval = (p - (const unsigned char *)data);
    }
    
    return retval;
}
