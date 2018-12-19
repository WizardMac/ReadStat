#include <string.h>

#include "file_format.h"
#include "../../readstat.h"

int readstat_format(const char *filename) {
    size_t len = strlen(filename);
    if (len < sizeof(".dta")-1)
        return RS_FORMAT_UNKNOWN;

    if (strncmp(filename + len - 4, ".dta", 4) == 0)
        return RS_FORMAT_DTA;

    if (strncmp(filename + len - 4, ".sav", 4) == 0)
        return RS_FORMAT_SAV;

    if (strncmp(filename + len - 4, ".por", 4) == 0)
        return RS_FORMAT_POR;

    #if HAVE_CSVREADER
    if (strncmp(filename + len - 4, ".csv", 4) == 0)
        return RS_FORMAT_CSV;
    #endif

    if (strncmp(filename + len - 4, ".xpt", 4) == 0)
        return RS_FORMAT_XPORT;

    if (len < sizeof(".json")-1)
        return RS_FORMAT_UNKNOWN;
    
    if (strncmp(filename + len - 5, ".json", 5) == 0)
        return RS_FORMAT_JSON;

    if (strncmp(filename + len - 5, ".zsav", 5) == 0)
        return RS_FORMAT_ZSAV;

    if (len < sizeof(".sas7bdat")-1)
        return RS_FORMAT_UNKNOWN;

    if (strncmp(filename + len - 9, ".sas7bdat", 9) == 0)
        return RS_FORMAT_SAS_DATA;

    if (strncmp(filename + len - 9, ".sas7bcat", 9) == 0)
        return RS_FORMAT_SAS_CATALOG;

    return RS_FORMAT_UNKNOWN;
}

const char *readstat_format_name(int format) {
    if (format == RS_FORMAT_DTA)
        return "Stata binary file (DTA)";

    if (format == RS_FORMAT_SAV)
        return "SPSS binary file (SAV)";

    if (format == RS_FORMAT_ZSAV)
        return "SPSS compressed binary file (ZSAV)";

    if (format == RS_FORMAT_POR)
        return "SPSS portable file (POR)";

    if (format == RS_FORMAT_SAS_DATA)
        return "SAS data file (SAS7BDAT)";

    if (format == RS_FORMAT_SAS_CATALOG)
        return "SAS catalog file (SAS7BCAT)";
    
    if (format == RS_FORMAT_CSV)
        return "CSV";

    if (format == RS_FORMAT_XPORT)
        return "SAS transport file (XPORT)";

    return "Unknown";
}

int is_catalog(const char *filename) {
    return (readstat_format(filename) == RS_FORMAT_SAS_CATALOG);
}

int is_json(const char *filename) {
    return (readstat_format(filename) == RS_FORMAT_JSON);
}

int can_read(const char *filename) {
    return (readstat_format(filename) != RS_FORMAT_UNKNOWN);
}

