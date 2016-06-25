
#line 1 "src/readstat_por_parse.rl"

#include "readstat.h"
#include "readstat_por_parse.h"


#line 9 "src/readstat_por_parse.c"
static const char _por_field_parse_actions[] = {
	0, 1, 0, 1, 2, 1, 4, 1, 
	7, 1, 8, 1, 9, 2, 1, 0, 
	2, 2, 9, 2, 4, 9, 2, 6, 
	9, 3, 3, 1, 0, 3, 5, 1, 
	0
};

static const char _por_field_parse_key_offsets[] = {
	0, 0, 8, 9, 14, 18, 23, 31, 
	35, 40, 44, 48, 55, 55
};

static const char _por_field_parse_trans_keys[] = {
	32, 42, 45, 46, 48, 57, 65, 84, 
	46, 46, 48, 57, 65, 84, 48, 57, 
	65, 84, 47, 48, 57, 65, 84, 43, 
	45, 46, 47, 48, 57, 65, 84, 48, 
	57, 65, 84, 47, 48, 57, 65, 84, 
	48, 57, 65, 84, 48, 57, 65, 84, 
	43, 45, 47, 48, 57, 65, 84, 0
};

static const char _por_field_parse_single_lengths[] = {
	0, 4, 1, 1, 0, 1, 4, 0, 
	1, 0, 0, 3, 0, 0
};

static const char _por_field_parse_range_lengths[] = {
	0, 2, 0, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 0, 0
};

static const char _por_field_parse_index_offsets[] = {
	0, 0, 7, 9, 13, 16, 20, 27, 
	30, 34, 37, 40, 46, 47
};

static const char _por_field_parse_trans_targs[] = {
	1, 2, 3, 4, 6, 6, 0, 12, 
	0, 4, 6, 6, 0, 5, 5, 0, 
	13, 5, 5, 0, 7, 9, 10, 13, 
	6, 6, 0, 8, 8, 0, 13, 8, 
	8, 0, 8, 8, 0, 11, 11, 0, 
	7, 9, 13, 11, 11, 0, 0, 0, 
	0
};

static const char _por_field_parse_trans_actions[] = {
	0, 0, 0, 0, 13, 13, 0, 11, 
	0, 7, 25, 25, 0, 13, 13, 0, 
	16, 1, 1, 0, 5, 5, 5, 19, 
	1, 1, 0, 13, 13, 0, 22, 1, 
	1, 0, 29, 29, 0, 13, 13, 0, 
	3, 3, 16, 1, 1, 0, 0, 0, 
	0
};

static const char _por_field_parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 9, 0
};

static const int por_field_parse_start = 1;

static const int por_field_parse_en_main = 1;


#line 8 "src/readstat_por_parse.rl"


int readstat_por_parse_double(const char *data, size_t len, double *result, 
        readstat_error_handler error_cb, void *user_ctx) {
    int retval = 0;
    double val = 0.0;
    long num = 0;
    long frac = 0;
    long exp = 0;
    
    long temp_val = 0;
    long frac_len = 0;
    
    const unsigned char *val_start = NULL;
    
    const unsigned char *p = (const unsigned char *)data;
    const unsigned char *eof = p + len;
    
    int cs;
    int is_negative = 0, exp_is_negative = 0;
    int success = 0;
    
    
#line 102 "src/readstat_por_parse.c"
	{
	cs = por_field_parse_start;
	}

#line 107 "src/readstat_por_parse.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _por_field_parse_trans_keys + _por_field_parse_key_offsets[cs];
	_trans = _por_field_parse_index_offsets[cs];

	_klen = _por_field_parse_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _por_field_parse_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	cs = _por_field_parse_trans_targs[_trans];

	if ( _por_field_parse_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _por_field_parse_actions + _por_field_parse_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	break;
	case 1:
#line 39 "src/readstat_por_parse.rl"
	{ temp_val = 0; val_start = p; }
	break;
	case 2:
#line 41 "src/readstat_por_parse.rl"
	{ frac = temp_val; frac_len = (p - val_start); }
	break;
	case 3:
#line 43 "src/readstat_por_parse.rl"
	{ is_negative = 1; }
	break;
	case 4:
#line 43 "src/readstat_por_parse.rl"
	{ num = temp_val; }
	break;
	case 5:
#line 44 "src/readstat_por_parse.rl"
	{ exp_is_negative = 1; }
	break;
	case 6:
#line 44 "src/readstat_por_parse.rl"
	{ exp = temp_val; }
	break;
	case 7:
#line 46 "src/readstat_por_parse.rl"
	{ is_negative = 1; }
	break;
	case 9:
#line 50 "src/readstat_por_parse.rl"
	{ success = 1; {p++; goto _out; } }
	break;
#line 220 "src/readstat_por_parse.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	p += 1;
	goto _resume;
	if ( p == eof )
	{
	const char *__acts = _por_field_parse_actions + _por_field_parse_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 8:
#line 48 "src/readstat_por_parse.rl"
	{ val = NAN; }
	break;
#line 239 "src/readstat_por_parse.c"
		}
	}
	}

	_out: {}
	}

#line 54 "src/readstat_por_parse.rl"


    val = 1.0 * num;
    if (frac_len)
        val += frac / pow(30.0, frac_len);
    if (exp_is_negative)
        exp *= -1;
    if (exp) {
        val *= pow(10.0, exp);
    }
    if (is_negative)
        val *= -1;
    
    if (!success) {
        retval = -1;
        if (error_cb) {
            char error_buf[1024];
            snprintf(error_buf, sizeof(error_buf), "Read bytes: %ld Ending state: %d\n", (long)(p - (const unsigned char *)data), cs);
            error_cb(error_buf, user_ctx);
        }
    }
    
    if (retval == 0) {
        if (result)
            *result = val;
        
        retval = (p - (const unsigned char *)data);
    }

    /* suppress warning */
    (void)por_field_parse_en_main;

    return retval;
}
