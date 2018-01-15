#include <stdlib.h>

#include "../readstat.h"

#include "test_readstat.h"

long dta_file_format_version(long format_code) {
    long version = -1;
    if (format_code == RT_FORMAT_DTA_104) {
        version = 104;
    } else if (format_code == RT_FORMAT_DTA_105) {
        version = 105;
    } else if (format_code == RT_FORMAT_DTA_108) {
        version = 108;
    } else if (format_code == RT_FORMAT_DTA_110) {
        version = 110;
    } else if (format_code == RT_FORMAT_DTA_111) {
        version = 111;
    } else if (format_code == RT_FORMAT_DTA_114) {
        version = 114;
    } else if (format_code == RT_FORMAT_DTA_117) {
        version = 117;
    } else if (format_code == RT_FORMAT_DTA_118) {
        version = 118;
    }
    return version;
}

