#include <stdio.h>
#include <stdlib.h>

#include "../../readstat.h"
#include "../format.h"

#include "produce_csv_column_header.h"
#include "produce_value_label.h"
#include "produce_value_label_dta.h"
#include "produce_value_label_sav.h"

void produce_value_label(struct csv_metadata *c, const char* column) {
    if (c->output_format == RS_FORMAT_CSV) {
        // ignore
    } else if (c->output_format == RS_FORMAT_DTA) {
        produce_value_label_dta(c, column);
    } else if (c->output_format == RS_FORMAT_SAV) {
        produce_value_label_sav(c, column);
    } else {
        fprintf(stderr, "%s:%d unsupported output format %d\n", __FILE__, __LINE__, c->output_format);
        exit(EXIT_FAILURE);
    }
}
