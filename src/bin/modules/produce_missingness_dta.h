#ifndef __PRODUCE_MISSINGNESS_DTA_H
#define __PRODUCE_MISSINGNESS_DTA_H

#include "produce_csv_column_header.h"
#include "json_metadata.h"

void produce_missingness_dta(struct csv_metadata *c, const char* column);

#endif
