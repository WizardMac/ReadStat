#include <stdlib.h>

#include "../readstat.h"

#include "test_readstat.h"

long sas_file_format_version(long format_code) {
    if (format_code == RT_FORMAT_SAS7BDAT_32BIT) {
        return 80101;
    }
    return 90101;
}
