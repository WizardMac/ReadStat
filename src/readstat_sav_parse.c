
#line 1 "src/readstat_sav_parse.rl"

#include "readstat_sav.h"
#include "readstat_sav_parse.h"
#include <stdlib.h>
#include <unistd.h>

typedef struct varlookup {
    char      name[9];
    int       index;
} varlookup_t;

static int compare_key_varlookup(const void *elem1, const void *elem2) {
    const char *key = (const char *)elem1;
    const varlookup_t *v = (const varlookup_t *)elem2;
    return strcmp(key, v->name);
}

static int compare_varlookups(const void *elem1, const void *elem2) {
    const varlookup_t *v1 = (const varlookup_t *)elem1;
    const varlookup_t *v2 = (const varlookup_t *)elem2;
    return strcmp(v1->name, v2->name);
}


#line 28 "src/readstat_sav_parse.c"
static const int sav_long_variable_parse_start = 1;
static const int sav_long_variable_parse_first_final = 11;
static const int sav_long_variable_parse_error = 0;

static const int sav_long_variable_parse_en_main = 1;


#line 28 "src/readstat_sav_parse.rl"


int sav_parse_long_variable_names_record(void *data, int count, sav_ctx_t *ctx) {
    varlookup_t *table = malloc(ctx->var_index * sizeof(varlookup_t));
    int i;
    int retval = 0;
    int var_count = ctx->var_index;
    for (i=0; i<var_count; i++) {
        memcpy(table[i].name, ctx->varinfo[i].name, 9);
        table[i].index = ctx->varinfo[i].index;
    }
    qsort(table, var_count, sizeof(varlookup_t), &compare_varlookups);
        
    char temp_key[9];
    char temp_val[65];
    int key_offset = 0;
    int val_offset = 0;
    
    u_char *p = (u_char *)data;
    u_char *pe = (u_char *)data + count;
    u_char *eof = pe;

    int cs;

    
#line 62 "src/readstat_sav_parse.c"
	{
	cs = sav_long_variable_parse_start;
	}

#line 67 "src/readstat_sav_parse.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) > 90u ) {
		if ( 192u <= (*p) && (*p) <= 223u )
			goto tr0;
	} else if ( (*p) >= 64u )
		goto tr0;
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 65 "src/readstat_sav_parse.rl"
	{ key_offset = 0; }
#line 65 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 93 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr2;
		case 61u: goto tr3;
		case 95u: goto tr2;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr2;
	} else if ( (*p) > 57u ) {
		if ( (*p) > 90u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto tr2;
		} else if ( (*p) >= 64u )
			goto tr2;
	} else
		goto tr2;
	goto st0;
tr2:
#line 65 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 119 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr4;
		case 61u: goto tr3;
		case 95u: goto tr4;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr4;
	} else if ( (*p) > 57u ) {
		if ( (*p) > 90u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto tr4;
		} else if ( (*p) >= 64u )
			goto tr4;
	} else
		goto tr4;
	goto st0;
tr4:
#line 65 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 145 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr5;
		case 61u: goto tr3;
		case 95u: goto tr5;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr5;
	} else if ( (*p) > 57u ) {
		if ( (*p) > 90u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto tr5;
		} else if ( (*p) >= 64u )
			goto tr5;
	} else
		goto tr5;
	goto st0;
tr5:
#line 65 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 171 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr6;
		case 61u: goto tr3;
		case 95u: goto tr6;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr6;
	} else if ( (*p) > 57u ) {
		if ( (*p) > 90u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto tr6;
		} else if ( (*p) >= 64u )
			goto tr6;
	} else
		goto tr6;
	goto st0;
tr6:
#line 65 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 197 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr7;
		case 61u: goto tr3;
		case 95u: goto tr7;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr7;
	} else if ( (*p) > 57u ) {
		if ( (*p) > 90u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto tr7;
		} else if ( (*p) >= 64u )
			goto tr7;
	} else
		goto tr7;
	goto st0;
tr7:
#line 65 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 223 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr8;
		case 61u: goto tr3;
		case 95u: goto tr8;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr8;
	} else if ( (*p) > 57u ) {
		if ( (*p) > 90u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto tr8;
		} else if ( (*p) >= 64u )
			goto tr8;
	} else
		goto tr8;
	goto st0;
tr8:
#line 65 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 249 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr9;
		case 61u: goto tr3;
		case 95u: goto tr9;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr9;
	} else if ( (*p) > 57u ) {
		if ( (*p) > 90u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto tr9;
		} else if ( (*p) >= 64u )
			goto tr9;
	} else
		goto tr9;
	goto st0;
tr9:
#line 65 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 275 "src/readstat_sav_parse.c"
	if ( (*p) == 61u )
		goto tr3;
	goto st0;
tr3:
#line 65 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = '\0'; }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 287 "src/readstat_sav_parse.c"
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr10;
tr10:
#line 67 "src/readstat_sav_parse.rl"
	{ val_offset = 0; }
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 304 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr12;
tr11:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = '\0'; }
#line 53 "src/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                memcpy(ctx->varinfo[found->index].longname, temp_val, val_offset);
            } else {
                dprintf(STDERR_FILENO, "Failed to find %s\n", temp_key);
            }
        }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 330 "src/readstat_sav_parse.c"
	if ( (*p) > 90u ) {
		if ( 192u <= (*p) && (*p) <= 223u )
			goto tr0;
	} else if ( (*p) >= 64u )
		goto tr0;
	goto st0;
tr12:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 345 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr13;
tr13:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 362 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr14;
tr14:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
#line 379 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr15;
tr15:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 396 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr16;
tr16:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 413 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr17;
tr17:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 430 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr18;
tr18:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 447 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr19;
tr19:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 464 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr20;
tr20:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 481 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr21;
tr21:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 498 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr22;
tr22:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st23;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
#line 515 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr23;
tr23:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
#line 532 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr24;
tr24:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 549 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr25;
tr25:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 566 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr26;
tr26:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 583 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr27;
tr27:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 600 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr28;
tr28:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 617 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr29;
tr29:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 634 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr30;
tr30:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 651 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr31;
tr31:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 668 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr32;
tr32:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
#line 685 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr33;
tr33:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 702 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr34;
tr34:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 719 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr35;
tr35:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 736 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr36;
tr36:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 753 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr37;
tr37:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 770 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr38;
tr38:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 787 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr39;
tr39:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st40;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
#line 804 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr40;
tr40:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
#line 821 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr41;
tr41:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st42;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
#line 838 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr42;
tr42:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
#line 855 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr43;
tr43:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st44;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
#line 872 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr44;
tr44:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
#line 889 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr45;
tr45:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st46;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
#line 906 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr46;
tr46:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st47;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
#line 923 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr47;
tr47:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st48;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
#line 940 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr48;
tr48:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st49;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
#line 957 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr49;
tr49:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st50;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
#line 974 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr50;
tr50:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st51;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
#line 991 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr51;
tr51:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st52;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
#line 1008 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr52;
tr52:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st53;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
#line 1025 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr53;
tr53:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st54;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
#line 1042 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr54;
tr54:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st55;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
#line 1059 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr55;
tr55:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st56;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
#line 1076 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr56;
tr56:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st57;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
#line 1093 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr57;
tr57:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st58;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
#line 1110 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr58;
tr58:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st59;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
#line 1127 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr59;
tr59:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st60;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
#line 1144 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr60;
tr60:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st61;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
#line 1161 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr61;
tr61:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st62;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
#line 1178 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr62;
tr62:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st63;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
#line 1195 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr63;
tr63:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st64;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
#line 1212 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr64;
tr64:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st65;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
#line 1229 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr65;
tr65:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st66;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
#line 1246 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr66;
tr66:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st67;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
#line 1263 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr67;
tr67:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st68;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
#line 1280 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr68;
tr68:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 1297 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr69;
tr69:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st70;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
#line 1314 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr70;
tr70:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st71;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
#line 1331 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr71;
tr71:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st72;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
#line 1348 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr72;
tr72:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st73;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
#line 1365 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr73;
tr73:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st74;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
#line 1382 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr74;
tr74:
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st75;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
#line 1399 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 11: 
	case 13: 
	case 14: 
	case 15: 
	case 16: 
	case 17: 
	case 18: 
	case 19: 
	case 20: 
	case 21: 
	case 22: 
	case 23: 
	case 24: 
	case 25: 
	case 26: 
	case 27: 
	case 28: 
	case 29: 
	case 30: 
	case 31: 
	case 32: 
	case 33: 
	case 34: 
	case 35: 
	case 36: 
	case 37: 
	case 38: 
	case 39: 
	case 40: 
	case 41: 
	case 42: 
	case 43: 
	case 44: 
	case 45: 
	case 46: 
	case 47: 
	case 48: 
	case 49: 
	case 50: 
	case 51: 
	case 52: 
	case 53: 
	case 54: 
	case 55: 
	case 56: 
	case 57: 
	case 58: 
	case 59: 
	case 60: 
	case 61: 
	case 62: 
	case 63: 
	case 64: 
	case 65: 
	case 66: 
	case 67: 
	case 68: 
	case 69: 
	case 70: 
	case 71: 
	case 72: 
	case 73: 
	case 74: 
	case 75: 
#line 67 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = '\0'; }
#line 53 "src/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                memcpy(ctx->varinfo[found->index].longname, temp_val, val_offset);
            } else {
                dprintf(STDERR_FILENO, "Failed to find %s\n", temp_key);
            }
        }
	break;
#line 1559 "src/readstat_sav_parse.c"
	}
	}

	_out: {}
	}

#line 75 "src/readstat_sav_parse.rl"


    if (cs < 
#line 1570 "src/readstat_sav_parse.c"
11
#line 77 "src/readstat_sav_parse.rl"
|| p != pe) {
        dprintf(STDERR_FILENO, "Error parsing string \"%s\" around byte #%ld/%d, character %c\n", 
                (char *)data, p - (u_char *)data, count, *p);
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    return retval;
}


#line 1585 "src/readstat_sav_parse.c"
static const int sav_very_long_string_parse_start = 1;
static const int sav_very_long_string_parse_first_final = 12;
static const int sav_very_long_string_parse_error = 0;

static const int sav_very_long_string_parse_en_main = 1;


#line 92 "src/readstat_sav_parse.rl"


int sav_parse_very_long_string_record(void *data, int count, sav_ctx_t *ctx) {
    varlookup_t *table = malloc(ctx->var_index * sizeof(varlookup_t));
    int i;
    int retval = 0;
    int var_count = ctx->var_index;
    for (i=0; i<var_count; i++) {
        memcpy(table[i].name, ctx->varinfo[i].name, 9);
        table[i].index = ctx->varinfo[i].index;
    }
    qsort(table, var_count, sizeof(varlookup_t), &compare_varlookups);
    
    char temp_key[9];
    int temp_val;
    int key_offset = 0;
    
    u_char *p = (u_char *)data;
    u_char *pe = (u_char *)data + count;
    
    int cs;
    
    
#line 1617 "src/readstat_sav_parse.c"
	{
	cs = sav_very_long_string_parse_start;
	}

#line 1622 "src/readstat_sav_parse.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( 64u <= (*p) && (*p) <= 90u )
		goto tr0;
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 128 "src/readstat_sav_parse.rl"
	{ key_offset = 0; }
#line 128 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 1645 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr2;
		case 61u: goto tr3;
		case 95u: goto tr2;
		case 192u: goto tr2;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr2;
	} else if ( (*p) > 57u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr2;
	} else
		goto tr2;
	goto st0;
tr2:
#line 128 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 1669 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr4;
		case 61u: goto tr3;
		case 95u: goto tr4;
		case 192u: goto tr4;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr4;
	} else if ( (*p) > 57u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr4;
	} else
		goto tr4;
	goto st0;
tr4:
#line 128 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 1693 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr5;
		case 61u: goto tr3;
		case 95u: goto tr5;
		case 192u: goto tr5;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr5;
	} else if ( (*p) > 57u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr5;
	} else
		goto tr5;
	goto st0;
tr5:
#line 128 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 1717 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr6;
		case 61u: goto tr3;
		case 95u: goto tr6;
		case 192u: goto tr6;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr6;
	} else if ( (*p) > 57u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr6;
	} else
		goto tr6;
	goto st0;
tr6:
#line 128 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 1741 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr7;
		case 61u: goto tr3;
		case 95u: goto tr7;
		case 192u: goto tr7;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr7;
	} else if ( (*p) > 57u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr7;
	} else
		goto tr7;
	goto st0;
tr7:
#line 128 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 1765 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr8;
		case 61u: goto tr3;
		case 95u: goto tr8;
		case 192u: goto tr8;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr8;
	} else if ( (*p) > 57u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr8;
	} else
		goto tr8;
	goto st0;
tr8:
#line 128 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 1789 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto tr9;
		case 61u: goto tr3;
		case 95u: goto tr9;
		case 192u: goto tr9;
	}
	if ( (*p) < 48u ) {
		if ( 35u <= (*p) && (*p) <= 36u )
			goto tr9;
	} else if ( (*p) > 57u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr9;
	} else
		goto tr9;
	goto st0;
tr9:
#line 128 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 1813 "src/readstat_sav_parse.c"
	if ( (*p) == 61u )
		goto tr3;
	goto st0;
tr3:
#line 128 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = '\0'; }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 1825 "src/readstat_sav_parse.c"
	if ( 48u <= (*p) && (*p) <= 57u )
		goto tr10;
	goto st0;
tr10:
#line 130 "src/readstat_sav_parse.rl"
	{ temp_val = 0; }
#line 122 "src/readstat_sav_parse.rl"
	{
            if ((*p) != '\0') { 
                temp_val = 10 * temp_val + ((*p) - '0'); 
            }
        }
	goto st11;
tr12:
#line 122 "src/readstat_sav_parse.rl"
	{
            if ((*p) != '\0') { 
                temp_val = 10 * temp_val + ((*p) - '0'); 
            }
        }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 1851 "src/readstat_sav_parse.c"
	if ( (*p) == 0u )
		goto tr11;
	if ( 48u <= (*p) && (*p) <= 57u )
		goto tr12;
	goto st0;
tr11:
#line 115 "src/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                ctx->varinfo[found->index].string_length = temp_val;
            }
        }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 1870 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 0u: goto st12;
		case 9u: goto st13;
	}
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	if ( 64u <= (*p) && (*p) <= 90u )
		goto tr0;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 138 "src/readstat_sav_parse.rl"

    
    if (cs < 
#line 1905 "src/readstat_sav_parse.c"
12
#line 140 "src/readstat_sav_parse.rl"
 || p != pe) {
        dprintf(STDERR_FILENO, "Parsed %ld of %ld bytes\n", p - (u_char *)data, pe - (u_char *)data);
        dprintf(STDERR_FILENO, "Remaining bytes: %s\n", p);
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    return retval;
}
