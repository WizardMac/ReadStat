#include "readstat.h"

#pragma pack(push, 1)

// DTA files

typedef struct dta_header_s {
    unsigned char    ds_format;
    unsigned char    byteorder;
    unsigned char    filetype;
    unsigned char    unused;
    int16_t          nvar;
    int32_t          nobs;
} dta_header_t;

typedef struct dta_expansion_field_s {
    unsigned char    data_type;
    int32_t          len;
    unsigned char    contents[0];
} dta_expansion_field_t;

typedef struct dta_short_expansion_field_s {
    unsigned char    data_type;
    int16_t          len;
    unsigned char    contents[0];
} dta_short_expansion_field_t;

typedef struct dta_value_label_table_header_s {
    int32_t          len;
    char             labname[33];
    char             padding[3];
} dta_value_label_table_header_t;

#pragma pack(pop)

typedef struct dta_ctx_s {
    size_t         data_label_len;
    char           typlist_is_char;
    unsigned char *typlist;
    size_t         typlist_len;
    char          *varlist;
    size_t         varlist_len;
    int16_t       *srtlist;
    size_t         srtlist_len;
    char          *fmtlist;
    size_t         fmtlist_len;
    char          *lbllist;
    size_t         lbllist_len;
    char          *variable_labels;
    size_t         variable_labels_len;

    size_t         variable_name_len;
    size_t         fmtlist_entry_len;
    size_t         lbllist_entry_len;
    size_t         variable_labels_entry_len;
    size_t         expansion_len_len;

    int            nvar;
    int            nobs;
    int            machine_needs_byte_swap:1;
    int            machine_is_twos_complement:1;
} dta_ctx_t;

#define DTA_HILO  0x01
#define DTA_LOHI  0x02

#define DTA_MAX_CHAR   0x64
#define DTA_MAX_INT16  0x7fe4
#define DTA_MAX_INT32  0x7fffffe4
#define DTA_MAX_FLOAT  1.7e38f
#define DTA_MAX_DOUBLE 8.9e307

#define DTA_MISSING_CHAR         0x65
#define DTA_MISSING_INT16      0x7FE5
#define DTA_MISSING_INT32  0x7FFFFFE5

#define DTA_TYPE_CODE_CHAR   0xFB
#define DTA_TYPE_CODE_INT16  0xFC
#define DTA_TYPE_CODE_INT32  0xFD
#define DTA_TYPE_CODE_FLOAT  0xFE
#define DTA_TYPE_CODE_DOUBLE 0xFF

#define DTA_OLD_TYPE_CODE_CHAR   'b'
#define DTA_OLD_TYPE_CODE_INT16  'i'
#define DTA_OLD_TYPE_CODE_INT32  'l'
#define DTA_OLD_TYPE_CODE_FLOAT  'f'
#define DTA_OLD_TYPE_CODE_DOUBLE 'd'

dta_ctx_t *dta_ctx_init(int16_t nvar, int32_t nobs, unsigned char byteorder, unsigned char ds_format);
void dta_ctx_free(dta_ctx_t *ctx);

int parse_dta(const char *filename, void *user_ctx, 
              readstat_handle_info_callback info_cb, readstat_handle_variable_callback variable_cb,
              readstat_handle_value_callback value_cb, readstat_handle_value_label_callback value_label_cb);

