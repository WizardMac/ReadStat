#ifndef __EXTRACT_LABEL_SET_H
#define __EXTRACT_LABEL_SET_H

#include "../readstat.h"
#include "extract_metadata.h"

readstat_label_set_t * get_label_set(const char *val_labels, struct context *ctx, int alloc_new);
int handle_value_label(const char *val_labels, readstat_value_t value, const char *label, void *c);

#endif
