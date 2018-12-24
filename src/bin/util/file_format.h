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
    RS_FORMAT_SAS_COMMANDS,
    RS_FORMAT_SPSS_COMMANDS,
    RS_FORMAT_STATA_DICTIONARY,
    RS_FORMAT_CSV,
    RS_FORMAT_JSON
};

int readstat_format(const char *filename);
const char *readstat_format_name(int format);
int is_catalog(const char *filename);
int is_json(const char *filename);
int is_dictionary(const char *filename);
int can_read(const char *filename);

#endif
