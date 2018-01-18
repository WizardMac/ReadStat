#ifndef __FORMAT_H
#define __FORMAT_H

enum {
    RS_FORMAT_UNKNOWN,
    RS_FORMAT_DTA,
    RS_FORMAT_SAV,
    RS_FORMAT_ZSAV,
    RS_FORMAT_POR,
    RS_FORMAT_SAS_DATA,
    RS_FORMAT_SAS_CATALOG,
    RS_FORMAT_XPORT,
    RS_FORMAT_CSV,
    RS_FORMAT_JSON
};

int format(const char *filename);

#include "../readstat.h"

const char* readstat_type_str(readstat_type_t type);

#endif
