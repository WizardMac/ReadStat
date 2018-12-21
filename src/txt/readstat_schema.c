#include <stdlib.h>

#include "../readstat.h"
#include "readstat_schema.h"

void readstat_schema_free(readstat_schema_t *schema) {
    free(schema->entries);
    free(schema);
}
