#ifndef __PRODUCE_CSV_VALUE_H
#define __PRODUCE_CSV_VALUE_H

#include "produce_csv_column_header.h"

readstat_value_t value_sysmiss(const char *s, size_t len, struct csv_metadata *c);
readstat_value_t value_string(const char *s, size_t len, struct csv_metadata *c);
readstat_value_t value_double(const char *s, size_t len, struct csv_metadata *c);
void produce_csv_column_value(void *s, size_t len, void *data);

#endif
