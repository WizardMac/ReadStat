
#include "readstat_por.h"
#include "readstat_por_parse.h"

%%{
    machine por_field_parse;
    write data;
}%%

int readstat_por_parse_double(const char *data, size_t len, double *result) {
    int retval = 0;
    double val = 0.0;
    int num = 0;
    int frac = 0;
    int exp = 0;
    
    int temp_val = 0;
    int frac_len = 0;
    
    const u_char *val_start = NULL;
    
    const u_char *p = (u_char *)data;
    const u_char *eof = (u_char *)p + len;
    
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
        
        nonmissing_value = (("-" %{ is_negative = 1; })? value %{ num = temp_val; }
                            ("." value %{ frac = temp_val; frac_len = (fpc - val_start); })?
                            ( ("+" | "-" %{ exp_is_negative = 1; }) value %{ exp = temp_val; })?) "/";
        
        missing_value = "*." %{ val = NAN; };
        
        main := " "* (missing_value | nonmissing_value ) @{ success = 1; fbreak; };
        
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
        printf("Read bytes: %ld Ending state: %d\n", (p - (u_char *)data), cs);
    }
    
    if (retval == 0) {
        if (result)
            *result = val;
        
        retval = (p - (u_char *)data);
    }
    
    return retval;
}