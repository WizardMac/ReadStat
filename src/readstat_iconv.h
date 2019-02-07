#include <iconv.h>

typedef ICONV_CONST char ** readstat_iconv_inbuf_t;

typedef struct readstat_charset_entry_s {
    int     code;
    char    name[32];
} readstat_charset_entry_t;
