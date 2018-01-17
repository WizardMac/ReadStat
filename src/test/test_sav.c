#include <stdlib.h>

#include "../readstat.h"

#include "test_readstat.h"

long sav_file_format_version(long format_code) {
    if ((format_code & RT_FORMAT_SAV_COMP_ZLIB))
        return 3;

    return 2;
}
