
#line 1 "src/readstat_sav_parse.rl"

#include "readstat_sav.h"
#include "readstat_sav_parse.h"
#include <stdlib.h>

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


#line 27 "src/readstat_sav_parse.c"
static const int sav_long_variable_parse_start = 1;
static const int sav_long_variable_parse_first_final = 11;
static const int sav_long_variable_parse_error = 0;

static const int sav_long_variable_parse_en_main = 1;


#line 27 "src/readstat_sav_parse.rl"


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

    
#line 61 "src/readstat_sav_parse.c"
	{
	cs = sav_long_variable_parse_start;
	}

#line 66 "src/readstat_sav_parse.c"
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
#line 64 "src/readstat_sav_parse.rl"
	{ key_offset = 0; }
#line 64 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 92 "src/readstat_sav_parse.c"
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
#line 64 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 118 "src/readstat_sav_parse.c"
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
#line 64 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 144 "src/readstat_sav_parse.c"
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
#line 64 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 170 "src/readstat_sav_parse.c"
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
#line 64 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 196 "src/readstat_sav_parse.c"
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
#line 64 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 222 "src/readstat_sav_parse.c"
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
#line 64 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 248 "src/readstat_sav_parse.c"
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
#line 64 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 274 "src/readstat_sav_parse.c"
	if ( (*p) == 61u )
		goto tr3;
	goto st0;
tr3:
#line 64 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = '\0'; }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 286 "src/readstat_sav_parse.c"
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr10;
tr10:
#line 66 "src/readstat_sav_parse.rl"
	{ val_offset = 0; }
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 303 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr12;
tr11:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = '\0'; }
#line 52 "src/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                memcpy(ctx->varinfo[found->index].longname, temp_val, val_offset);
            } else {
                printf("Failed to find %s\n", temp_key);
            }
        }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 329 "src/readstat_sav_parse.c"
	if ( (*p) > 90u ) {
		if ( 192u <= (*p) && (*p) <= 223u )
			goto tr0;
	} else if ( (*p) >= 64u )
		goto tr0;
	goto st0;
tr12:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 344 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr13;
tr13:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 361 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr14;
tr14:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st15;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
#line 378 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr15;
tr15:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st16;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
#line 395 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr16;
tr16:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st17;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
#line 412 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr17;
tr17:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st18;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
#line 429 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr18;
tr18:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st19;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
#line 446 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr19;
tr19:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st20;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
#line 463 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr20;
tr20:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st21;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
#line 480 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr21;
tr21:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st22;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
#line 497 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr22;
tr22:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st23;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
#line 514 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr23;
tr23:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st24;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
#line 531 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr24;
tr24:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st25;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
#line 548 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr25;
tr25:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st26;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
#line 565 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr26;
tr26:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st27;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
#line 582 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr27;
tr27:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st28;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
#line 599 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr28;
tr28:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st29;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
#line 616 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr29;
tr29:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st30;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
#line 633 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr30;
tr30:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st31;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
#line 650 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr31;
tr31:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st32;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
#line 667 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr32;
tr32:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st33;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
#line 684 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr33;
tr33:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st34;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
#line 701 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr34;
tr34:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st35;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
#line 718 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr35;
tr35:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 735 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr36;
tr36:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st37;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
#line 752 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr37;
tr37:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st38;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
#line 769 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr38;
tr38:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st39;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
#line 786 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr39;
tr39:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st40;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
#line 803 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr40;
tr40:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st41;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
#line 820 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr41;
tr41:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st42;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
#line 837 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr42;
tr42:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st43;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
#line 854 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr43;
tr43:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st44;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
#line 871 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr44;
tr44:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st45;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
#line 888 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr45;
tr45:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st46;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
#line 905 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr46;
tr46:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st47;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
#line 922 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr47;
tr47:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st48;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
#line 939 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr48;
tr48:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st49;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
#line 956 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr49;
tr49:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st50;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
#line 973 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr50;
tr50:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st51;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
#line 990 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr51;
tr51:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st52;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
#line 1007 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr52;
tr52:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st53;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
#line 1024 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr53;
tr53:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st54;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
#line 1041 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr54;
tr54:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st55;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
#line 1058 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr55;
tr55:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st56;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
#line 1075 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr56;
tr56:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st57;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
#line 1092 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr57;
tr57:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st58;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
#line 1109 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr58;
tr58:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st59;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
#line 1126 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr59;
tr59:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st60;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
#line 1143 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr60;
tr60:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st61;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
#line 1160 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr61;
tr61:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st62;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
#line 1177 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr62;
tr62:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st63;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
#line 1194 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr63;
tr63:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st64;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
#line 1211 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr64;
tr64:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st65;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
#line 1228 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr65;
tr65:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st66;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
#line 1245 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr66;
tr66:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st67;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
#line 1262 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr67;
tr67:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st68;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
#line 1279 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr68;
tr68:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st69;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
#line 1296 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr69;
tr69:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st70;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
#line 1313 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr70;
tr70:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st71;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
#line 1330 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr71;
tr71:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st72;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
#line 1347 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr72;
tr72:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st73;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
#line 1364 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr73;
tr73:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st74;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
#line 1381 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr11;
	if ( (*p) > 32u ) {
		if ( 127u <= (*p) && (*p) <= 191u )
			goto st0;
	} else
		goto st0;
	goto tr74;
tr74:
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = (*p); }
	goto st75;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
#line 1398 "src/readstat_sav_parse.c"
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
#line 66 "src/readstat_sav_parse.rl"
	{ temp_val[val_offset++] = '\0'; }
#line 52 "src/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                memcpy(ctx->varinfo[found->index].longname, temp_val, val_offset);
            } else {
                printf("Failed to find %s\n", temp_key);
            }
        }
	break;
#line 1558 "src/readstat_sav_parse.c"
	}
	}

	_out: {}
	}

#line 74 "src/readstat_sav_parse.rl"


    if (cs < 
#line 1569 "src/readstat_sav_parse.c"
11
#line 76 "src/readstat_sav_parse.rl"
|| p != pe) {
        printf("Error parsing string \"%s\" around byte #%ld/%d, character %c\n", 
                (char *)data, p - (u_char *)data, count, *p);
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    return retval;
}


#line 1584 "src/readstat_sav_parse.c"
static const int sav_very_long_string_parse_start = 1;
static const int sav_very_long_string_parse_first_final = 12;
static const int sav_very_long_string_parse_error = 0;

static const int sav_very_long_string_parse_en_main = 1;


#line 91 "src/readstat_sav_parse.rl"


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
    
    
#line 1616 "src/readstat_sav_parse.c"
	{
	cs = sav_very_long_string_parse_start;
	}

#line 1621 "src/readstat_sav_parse.c"
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
#line 127 "src/readstat_sav_parse.rl"
	{ key_offset = 0; }
#line 127 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 1644 "src/readstat_sav_parse.c"
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
#line 127 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st3;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
#line 1668 "src/readstat_sav_parse.c"
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
#line 127 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st4;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
#line 1692 "src/readstat_sav_parse.c"
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
#line 127 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st5;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
#line 1716 "src/readstat_sav_parse.c"
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
#line 127 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st6;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
#line 1740 "src/readstat_sav_parse.c"
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
#line 127 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st7;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
#line 1764 "src/readstat_sav_parse.c"
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
#line 127 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st8;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
#line 1788 "src/readstat_sav_parse.c"
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
#line 127 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = (*p); }
	goto st9;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
#line 1812 "src/readstat_sav_parse.c"
	if ( (*p) == 61u )
		goto tr3;
	goto st0;
tr3:
#line 127 "src/readstat_sav_parse.rl"
	{ temp_key[key_offset++] = '\0'; }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 1824 "src/readstat_sav_parse.c"
	if ( 48u <= (*p) && (*p) <= 57u )
		goto tr10;
	goto st0;
tr10:
#line 129 "src/readstat_sav_parse.rl"
	{ temp_val = 0; }
#line 121 "src/readstat_sav_parse.rl"
	{
            if ((*p) != '\0') { 
                temp_val = 10 * temp_val + ((*p) - '0'); 
            }
        }
	goto st11;
tr12:
#line 121 "src/readstat_sav_parse.rl"
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
#line 1850 "src/readstat_sav_parse.c"
	if ( (*p) == 0u )
		goto tr11;
	if ( 48u <= (*p) && (*p) <= 57u )
		goto tr12;
	goto st0;
tr11:
#line 114 "src/readstat_sav_parse.rl"
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
#line 1869 "src/readstat_sav_parse.c"
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

#line 137 "src/readstat_sav_parse.rl"

    
    if (cs < 
#line 1904 "src/readstat_sav_parse.c"
12
#line 139 "src/readstat_sav_parse.rl"
 || p != pe) {
        printf("Parsed %ld of %ld bytes\n", p - (u_char *)data, pe - (u_char *)data);
        printf("Remaining bytes: %s\n", p);
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    return retval;
}
