#ifndef __EXTRACT_METADATA_H
#define __EXTRACT_METADATA_H

#include "../readstat.h"

typedef struct context {
    int count;
    FILE* fp;
    int variable_count;
    int input_format;
    readstat_label_set_t *label_set;
} context;

int escape(const char *s, char* dest);
char* quote_and_escape(const char *src);

#endif
