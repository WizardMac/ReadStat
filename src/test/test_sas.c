#include <stdlib.h>

#include "../readstat.h"

#include "test_readstat.h"

long sas_file_format_version(long format_code) {
    if ((format_code & RT_FORMAT_XPORT_5))
        return 5;

    if ((format_code & RT_FORMAT_XPORT_8))
        return 8;

    if ((format_code & RT_FORMAT_SAS7BDAT_32BIT))
        return 80101;

    return 90101;
}
