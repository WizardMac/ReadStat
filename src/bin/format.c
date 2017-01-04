#include <string.h>

#include "format.h"
#include "../readstat.h"

const char* readstat_type_str(readstat_type_t type) {
    if (type == READSTAT_TYPE_STRING) {
        return "READSTAT_TYPE_STRING";
    }
    
    if (type == READSTAT_TYPE_INT8) {
        return "READSTAT_TYPE_INT8";
    }

    if (type == READSTAT_TYPE_INT16) {
        return "READSTAT_TYPE_INT16";
    }

    if (type == READSTAT_TYPE_INT32) {
        return "READSTAT_TYPE_INT32";
    }

    if (type == READSTAT_TYPE_FLOAT) {
        return "READSTAT_TYPE_FLOAT";
    }

    if (type == READSTAT_TYPE_DOUBLE) {
        return "READSTAT_TYPE_DOUBLE";
    }

    if (type == READSTAT_TYPE_STRING_REF) {
        return "READSTAT_TYPE_STRING_REF";
    }

    return "UNKNOWN TYPE";
} 

int format(const char *filename) {
    size_t len = strlen(filename);
    if (len < sizeof(".dta")-1)
        return RS_FORMAT_UNKNOWN;

    if (strncmp(filename + len - 4, ".dta", 4) == 0)
        return RS_FORMAT_DTA;

    if (strncmp(filename + len - 4, ".sav", 4) == 0)
        return RS_FORMAT_SAV;

    if (strncmp(filename + len - 4, ".por", 4) == 0)
        return RS_FORMAT_POR;

    #if HAVE_CSVREADER
    if (strncmp(filename + len - 4, ".csv", 4) == 0)
        return RS_FORMAT_CSV;
    #endif

    if (strncmp(filename + len - 4, ".xpt", 4) == 0)
        return RS_FORMAT_XPORT;

    if (len < sizeof(".json")-1)
        return RS_FORMAT_UNKNOWN;
    
    if (strncmp(filename + len - 5, ".json", 5) == 0)
        return RS_FORMAT_JSON;

    if (len < sizeof(".sas7bdat")-1)
        return RS_FORMAT_UNKNOWN;

    if (strncmp(filename + len - 9, ".sas7bdat", 9) == 0)
        return RS_FORMAT_SAS_DATA;

    if (strncmp(filename + len - 9, ".sas7bcat", 9) == 0)
        return RS_FORMAT_SAS_CATALOG;

    return RS_FORMAT_UNKNOWN;
}
