#ifndef __PRODUCE_MISSINGNESS_SAV_H
#define __PRODUCE_MISSINGNESS_SAV_H

#include "produce_csv_column_header.h"
#include "json_metadata.h"

void produce_missingness_sav(struct csv_metadata *c, const char* column);

#endif
