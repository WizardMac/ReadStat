#ifndef __MOD_CSV_READER_H
#define __MOD_CSV_READER_H

readstat_error_t readstat_parse_csv(readstat_parser_t *parser, const char *path, const char *jsonpath, struct csv_metadata* md2, void *user_ctx);

#endif
