#ifndef __FORMAT_H
#define __FORMAT_H

#define RS_FORMAT_UNKNOWN       0x00
#define RS_FORMAT_DTA           0x01
#define RS_FORMAT_SAV           0x02
#define RS_FORMAT_POR           0x04
#define RS_FORMAT_SAS_DATA      0x08
#define RS_FORMAT_SAS_CATALOG   0x10
#define RS_FORMAT_XPORT         0x20
#define RS_FORMAT_CSV           0x40
#define RS_FORMAT_JSON          0x80

int format(const char *filename);

#include "../readstat.h"

const char* readstat_type_str(readstat_type_t type);

#endif
