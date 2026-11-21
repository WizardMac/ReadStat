#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../readstat.h"
#include "../readstat_malloc.h"
#include "../readstat_iconv.h"
#include "../readstat_convert.h"
#include "readstat_sav.h"
#include "readstat_sav_parse_mr_name.h"

/* Multiple response set records (type 7, subtypes 7 and 19) hold one set per
 * line. See the "Multiple Response Sets Records" section of the PSPP system
 * file format documentation. Each line looks like one of:
 *
 *   $name=C <label_len> <label> <var> <var> ...
 *   $name=D<value_len> <value> <label_len> <label> <var> <var> ...
 *   $name=E <1|11> <value_len> <value> <label_len> <label> <var> <var> ...
 */

static void mr_set_free_contents(mr_set_t *mr) {
    if (mr->name)
        free(mr->name);
    if (mr->label)
        free(mr->label);
    if (mr->counted_string)
        free(mr->counted_string);
    if (mr->subvariables) {
        int i;
        for (i=0; i<mr->num_subvars; i++) {
            if (mr->subvariables[i])
                free(mr->subvariables[i]);
        }
        free(mr->subvariables);
    }
    memset(mr, 0, sizeof(mr_set_t));
}

static readstat_error_t mr_convert(char **out, const char *src, size_t src_len, sav_ctx_t *ctx) {
    size_t dst_len = 4 * src_len + 1; /* UTF-8 expansion: up to 4 bytes per char */
    char *dst = readstat_malloc(dst_len);
    if (dst == NULL)
        return READSTAT_ERROR_MALLOC;
    readstat_error_t retval = readstat_convert(dst, dst_len, src, src_len, ctx->converter);
    if (retval != READSTAT_OK) {
        free(dst);
        return retval;
    }
    *out = dst;
    return READSTAT_OK;
}

/* Parses a decimal count followed by a single space, then that many bytes.
 * On success *pp points at the byte after the string. */
static readstat_error_t mr_read_counted_string(const char **pp, const char *end,
        const char **str, size_t *str_len) {
    const char *p = *pp;
    size_t n = 0;
    if (p == end || *p < '0' || *p > '9')
        return READSTAT_ERROR_BAD_MR_STRING;
    while (p < end && *p >= '0' && *p <= '9') {
        if (n > (size_t)(end - p))
            return READSTAT_ERROR_BAD_MR_STRING;
        n = n * 10 + (*p - '0');
        p++;
    }
    if (p == end || *p != ' ')
        return READSTAT_ERROR_BAD_MR_STRING;
    p++;
    if (n > (size_t)(end - p))
        return READSTAT_ERROR_BAD_MR_STRING;
    *str = p;
    *str_len = n;
    *pp = p + n;
    return READSTAT_OK;
}

static readstat_error_t parse_mr_line(const char *line, size_t line_len, mr_set_t *result, sav_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    const char *p = line;
    const char *end = line + line_len;
    const char *str = NULL;
    size_t str_len = 0;

    memset(result, 0, sizeof(mr_set_t));

    /* Name: begins with '$', runs up to '=' */
    if (p == end || *p != '$') {
        retval = READSTAT_ERROR_BAD_MR_STRING;
        goto cleanup;
    }
    str = p;
    while (p < end && *p != '=' && *p != ' ')
        p++;
    if (p == end || *p != '=' || p == str) {
        retval = READSTAT_ERROR_BAD_MR_STRING;
        goto cleanup;
    }
    if ((retval = mr_convert(&result->name, str, p - str, ctx)) != READSTAT_OK)
        goto cleanup;
    p++;

    /* Type */
    if (p == end || (*p != 'C' && *p != 'D' && *p != 'E')) {
        retval = READSTAT_ERROR_BAD_MR_STRING;
        goto cleanup;
    }
    result->type = *p++;
    result->counted_value = -1;

    if (result->type == 'E') {
        /* " 1 " or " 11 ": 11 means LABELSOURCE=VARLABEL */
        long flag = 0;
        if (p == end || *p != ' ') {
            retval = READSTAT_ERROR_BAD_MR_STRING;
            goto cleanup;
        }
        p++;
        if (p == end || *p < '0' || *p > '9') {
            retval = READSTAT_ERROR_BAD_MR_STRING;
            goto cleanup;
        }
        while (p < end && *p >= '0' && *p <= '9' && flag < 1000)
            flag = flag * 10 + (*p++ - '0');
        if (p == end || *p != ' ') {
            retval = READSTAT_ERROR_BAD_MR_STRING;
            goto cleanup;
        }
        p++;
        result->label_from_var_label = (flag == 11);
    }

    if (result->type == 'D' || result->type == 'E') {
        result->is_dichotomy = 1;
        if ((retval = mr_read_counted_string(&p, end, &str, &str_len)) != READSTAT_OK)
            goto cleanup;
        if ((retval = mr_convert(&result->counted_string, str, str_len, ctx)) != READSTAT_OK)
            goto cleanup;
        /* readstat_convert trims trailing spaces; older SPSS pads to 8 bytes */
        if (result->counted_string[0] >= '0' && result->counted_string[0] <= '9') {
            result->counted_value = (int)strtol(result->counted_string, NULL, 10);
        }
        if (p == end || *p != ' ') {
            retval = READSTAT_ERROR_BAD_MR_STRING;
            goto cleanup;
        }
        p++;
    } else {
        if (p == end || *p != ' ') {
            retval = READSTAT_ERROR_BAD_MR_STRING;
            goto cleanup;
        }
        p++;
    }

    /* Label */
    if ((retval = mr_read_counted_string(&p, end, &str, &str_len)) != READSTAT_OK)
        goto cleanup;
    if ((retval = mr_convert(&result->label, str, str_len, ctx)) != READSTAT_OK)
        goto cleanup;

    /* Variables: space-separated; some files have zero or one variable.
     * Count them first and allocate the array once: readstat_realloc frees
     * the old block when it refuses a size, so growing it one variable at a
     * time would leave result->subvariables dangling. */
    size_t subvar_count = 0;
    for (str = p; str < end; str++) {
        if (*str != ' ' && (str == p || str[-1] == ' '))
            subvar_count++;
    }
    if (subvar_count &&
            (result->subvariables = readstat_malloc(subvar_count * sizeof(char *))) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    while (p < end) {
        if (*p == ' ') {
            p++;
            continue;
        }
        str = p;
        while (p < end && *p != ' ')
            p++;
        char *subvar = NULL;
        if ((retval = mr_convert(&subvar, str, p - str, ctx)) != READSTAT_OK)
            goto cleanup;
        result->subvariables[result->num_subvars++] = subvar;
    }

cleanup:
    if (retval != READSTAT_OK) {
        mr_set_free_contents(result);
    }
    return retval;
}

/* Returns the next line, without its line feed or carriage return, and
 * advances *pp past it */
static const char *mr_next_line(const char **pp, const char *end, size_t *line_len) {
    const char *p = *pp;
    const char *line = p;
    while (p < end && *p != '\n' && *p != '\0')
        p++;
    *line_len = p - line;
    if (p < end)
        p++; /* consume the line feed */
    if (*line_len && line[*line_len-1] == '\r')
        (*line_len)--;
    *pp = p;
    return line;
}

readstat_error_t parse_mr_string(const char *data, size_t data_len, mr_set_t **mr_sets, size_t *n_mr_lines, sav_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    const char *p = data;
    const char *end = data + data_len;
    size_t line_len = 0;
    size_t line_count = 0;

    /* Count the sets and grow *mr_sets once. Copy rather than call
     * readstat_realloc, which frees the old block when it refuses a size and
     * would leave *mr_sets, with the sets of an earlier record, dangling. */
    while (p < end) {
        mr_next_line(&p, end, &line_len);
        if (line_len)
            line_count++;
    }
    if (line_count == 0)
        return READSTAT_OK;

    mr_set_t *new_mr_sets = readstat_malloc((*n_mr_lines + line_count) * sizeof(mr_set_t));
    if (new_mr_sets == NULL)
        return READSTAT_ERROR_MALLOC;
    if (*n_mr_lines)
        memcpy(new_mr_sets, *mr_sets, *n_mr_lines * sizeof(mr_set_t));
    free(*mr_sets);
    *mr_sets = new_mr_sets;

    p = data;
    while (p < end) {
        const char *line = mr_next_line(&p, end, &line_len);
        if (line_len == 0)
            continue; /* the spec allows any number of line feeds between sets */

        retval = parse_mr_line(line, line_len, &(*mr_sets)[*n_mr_lines], ctx);
        if (retval != READSTAT_OK)
            goto cleanup;

        (*n_mr_lines)++;
    }

cleanup:
    return retval;
}
