
#include "readstat.h"
#include "readstat_bits.h"

#pragma pack(push, 1)

typedef struct sas_header_start_s {
    unsigned char magic[32];
    unsigned char a2;
    unsigned char mystery1[2];
    unsigned char a1;
    unsigned char mystery2[1];
    unsigned char endian;
    unsigned char mystery3[1];
    char          file_format;
    unsigned char mystery4[30];
    unsigned char encoding;
    unsigned char mystery5[13];
    char          file_type[8];
    char          file_label[64];
    char          file_info[8];
} sas_header_start_t;

typedef struct sas_header_end_s {
    char          release[8];
    char          host[16];
    char          version[16];
    char          os_vendor[16];
    char          os_name[16];
    char          extra[48];
} sas_header_end_t;

#pragma pack(pop)

typedef struct sas_header_info_s {
    int      little_endian;
    int      u64;
    int      vendor;
    int      major_version;
    int      minor_version;
    int      revision;
    int      pad1;
    int64_t  page_size;
    int64_t  page_count;
    int64_t  header_size;
    time_t   creation_time;
    time_t   modification_time;
    char     file_label[64];
    char    *encoding;
} sas_header_info_t;


enum {
    READSTAT_VENDOR_STAT_TRANSFER,
    READSTAT_VENDOR_SAS
};

typedef struct sas_text_ref_s {
    uint16_t    index;
    uint16_t    offset;
    uint16_t    length;
} sas_text_ref_t;

#define SAS_ENDIAN_BIG       0x00
#define SAS_ENDIAN_LITTLE    0x01

#define SAS_FILE_FORMAT_UNIX    '1'
#define SAS_FILE_FORMAT_WINDOWS '2'

#define SAS_ALIGNMENT_OFFSET_0  0x22
#define SAS_ALIGNMENT_OFFSET_4  0x33

#define SAS_COLUMN_TYPE_NUM  0x01
#define SAS_COLUMN_TYPE_CHR  0x02

#define SAS_SUBHEADER_SIGNATURE_ROW_SIZE       0xF7F7F7F7
#define SAS_SUBHEADER_SIGNATURE_COLUMN_SIZE    0xF6F6F6F6
#define SAS_SUBHEADER_SIGNATURE_COUNTS         0xFFFFFC00
#define SAS_SUBHEADER_SIGNATURE_COLUMN_FORMAT  0xFFFFFBFE
#define SAS_SUBHEADER_SIGNATURE_COLUMN_UNKNOWN 0xFFFFFFFA
#define SAS_SUBHEADER_SIGNATURE_COLUMN_ATTRS   0xFFFFFFFC
#define SAS_SUBHEADER_SIGNATURE_COLUMN_TEXT    0xFFFFFFFD
#define SAS_SUBHEADER_SIGNATURE_COLUMN_LIST    0xFFFFFFFE
#define SAS_SUBHEADER_SIGNATURE_COLUMN_NAME    0xFFFFFFFF

#define SAS_PAGE_TYPE_META   0x0000
#define SAS_PAGE_TYPE_DATA   0x0100
#define SAS_PAGE_TYPE_MIX    0x0200
#define SAS_PAGE_TYPE_AMD    0x0400
#define SAS_PAGE_TYPE_MASK   0x0F00

#define SAS_PAGE_TYPE_META2  0x4000
#define SAS_PAGE_TYPE_COMP   0x9000

static unsigned char sas7bdat_magic_number[32] = {
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   0xc2, 0xea, 0x81, 0x60,
    0xb3, 0x14, 0x11, 0xcf,   0xbd, 0x92, 0x08, 0x00,
    0x09, 0xc7, 0x31, 0x8c,   0x18, 0x1f, 0x10, 0x11
};

static unsigned char sas7bcat_magic_number[32] = {
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   0xc2, 0xea, 0x81, 0x63,
    0xb3, 0x14, 0x11, 0xcf,   0xbd, 0x92, 0x08, 0x00,
    0x09, 0xc7, 0x31, 0x8c,   0x18, 0x1f, 0x10, 0x11
};


uint64_t sas_read8(const char *data, int bswap);
uint32_t sas_read4(const char *data, int bswap);
uint16_t sas_read2(const char *data, int bswap);
readstat_error_t sas_read_header(readstat_io_t *io, sas_header_info_t *ctx, readstat_error_handler error_handler, void *user_ctx);
