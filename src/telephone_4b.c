#include "postgres.h"
#include "fmgr.h"
#include "libpq/pqformat.h"
#include "utils/builtins.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

typedef uint32 telephone_4b_t;

// base 10 = 9 fully usable digits (4,294,967,295)
// base 11 = 9 fully usable digits (1,904,440,553)
// base 12 = 8 fully usable digits (  9BA,461,593)
// base 15 = 8 fully usable digits (  1A2,0DC,D80)
// base 16 = 8 fully usable digits (   FF,FFF,FFF)

#define DIGIT_SPACE     0
#define DIGIT_SLASH     1 // Indicates alternative numbers.  Only valid in digits format or in the extension.
#define DIGIT_CONFIRM   2 // Prompt (confirmation pause) using ";"  Phones also use w.
#define DIGIT_PAUSE     3 // w = .5 sec pause in PBXs and prompt in phones.  Phones also use p and , for a 2 sec pause.

static char base16_chars[16] = " /;,0123456789*#";
static char char_to_num_b16[] = {
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    0,127,127,15,127,127,127,127,127,127,14,127,3,127,127,1,4,5,6,7,8,9,10,11,12,13,127,2,127,127,127,127,127,6,6,6,7,7,7,8,8,8,9,9,
    9,10,10,10,11,11,11,11,12,12,12,13,13,13,13,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    3,127,127,127,127,127,127,2,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,
    127,127,127,127,127,127,127,127,127,127,127,127,127,127,127};

#define BASE164BGetDatum(x)	 UInt32GetDatum(x)
#define DatumGetBASE164B(x)	 DatumGetUInt32(x)
#define PG_GETARG_BASE164B(n) DatumGetBASE164B(PG_GETARG_DATUM(n))
#define PG_RETURN_BASE164B(x) return BASE164BGetDatum(x)

Datum telephone_4b_in(PG_FUNCTION_ARGS);
Datum telephone_4b_out(PG_FUNCTION_ARGS);
Datum telephone_4b_to_text(PG_FUNCTION_ARGS);
Datum text_to_telephone_4b(PG_FUNCTION_ARGS);
Datum telephone_4b_send(PG_FUNCTION_ARGS);
Datum telephone_4b_recv(PG_FUNCTION_ARGS);
Datum telephone_4b_lt(PG_FUNCTION_ARGS);
Datum telephone_4b_le(PG_FUNCTION_ARGS);
Datum telephone_4b_eq(PG_FUNCTION_ARGS);
Datum telephone_4b_ne(PG_FUNCTION_ARGS);
Datum telephone_4b_ge(PG_FUNCTION_ARGS);
Datum telephone_4b_gt(PG_FUNCTION_ARGS);
Datum telephone_4b_cmp(PG_FUNCTION_ARGS);

static telephone_4b_t cstring_to_telephone_4b(char *telephone_4b_string);
static char *telephone_4b_to_cstring(telephone_4b_t telephone_4b);



/* generic input/output functions */
PG_FUNCTION_INFO_V1(telephone_4b_in);
Datum
telephone_4b_in(PG_FUNCTION_ARGS)
{
	telephone_4b_t	result;

	char   *telephone_4b_str = PG_GETARG_CSTRING(0);
	result = cstring_to_telephone_4b(telephone_4b_str);

	PG_RETURN_BASE164B(result);
}

PG_FUNCTION_INFO_V1(telephone_4b_out);
Datum
telephone_4b_out(PG_FUNCTION_ARGS)
{
	telephone_4b_t	packed_telephone_4b;
	char   *telephone_4b_string;

	packed_telephone_4b = PG_GETARG_BASE164B(0);
	telephone_4b_string = telephone_4b_to_cstring(packed_telephone_4b);

	PG_RETURN_CSTRING(telephone_4b_string);
}

/* type to/from text conversion routines */
PG_FUNCTION_INFO_V1(telephone_4b_to_text);
Datum
telephone_4b_to_text(PG_FUNCTION_ARGS)
{
	char	*telephone_4b_string;
	text	*telephone_4b_text;

	telephone_4b_t	packed_telephone_4b =  PG_GETARG_BASE164B(0);

	telephone_4b_string = telephone_4b_to_cstring(packed_telephone_4b);
	telephone_4b_text = DatumGetTextP(DirectFunctionCall1(textin, CStringGetDatum(telephone_4b_string)));

	PG_RETURN_TEXT_P(telephone_4b_text);
}

PG_FUNCTION_INFO_V1(text_to_telephone_4b);
Datum
text_to_telephone_4b(PG_FUNCTION_ARGS)
{
	text  *telephone_4b_text = PG_GETARG_TEXT_P(0);
	char  *telephone_4b_str = DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(telephone_4b_text)));
	telephone_4b_t telephone_4b = cstring_to_telephone_4b(telephone_4b_str);

	PG_RETURN_BASE164B(telephone_4b);
}

/* Functions to convert to/from bytea */
PG_FUNCTION_INFO_V1(telephone_4b_send);
Datum
telephone_4b_send(PG_FUNCTION_ARGS)
{
	StringInfoData buffer;
	telephone_4b_t telephone_4b = PG_GETARG_BASE164B(0);

	pq_begintypsend(&buffer);
	pq_sendint(&buffer, (uint32)telephone_4b, 4);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buffer));
}

PG_FUNCTION_INFO_V1(telephone_4b_recv);
Datum telephone_4b_recv(PG_FUNCTION_ARGS)
{
	telephone_4b_t	telephone_4b;
	StringInfo	buffer = (StringInfo) PG_GETARG_POINTER(0);

	telephone_4b = pq_getmsgint(buffer, 4);
	PG_RETURN_BASE164B(telephone_4b);
}

/* functions to support btree opclass */
PG_FUNCTION_INFO_V1(telephone_4b_lt);
Datum
telephone_4b_lt(PG_FUNCTION_ARGS)
{
	telephone_4b_t a = PG_GETARG_BASE164B(0);
	telephone_4b_t b = PG_GETARG_BASE164B(1);
	PG_RETURN_BOOL(a < b);
}

PG_FUNCTION_INFO_V1(telephone_4b_le);
Datum
telephone_4b_le(PG_FUNCTION_ARGS)
{
	telephone_4b_t a = PG_GETARG_BASE164B(0);
	telephone_4b_t b = PG_GETARG_BASE164B(1);
	PG_RETURN_BOOL (a <= b);
}

PG_FUNCTION_INFO_V1(telephone_4b_eq);
Datum
telephone_4b_eq(PG_FUNCTION_ARGS)
{
	telephone_4b_t a = PG_GETARG_BASE164B(0);
	telephone_4b_t b = PG_GETARG_BASE164B(1);
	PG_RETURN_BOOL(a == b);
}

PG_FUNCTION_INFO_V1(telephone_4b_ne);
Datum
telephone_4b_ne(PG_FUNCTION_ARGS)
{
	telephone_4b_t a = PG_GETARG_BASE164B(0);
	telephone_4b_t b = PG_GETARG_BASE164B(1);
	PG_RETURN_BOOL(a != b);
}

PG_FUNCTION_INFO_V1(telephone_4b_ge);
Datum
telephone_4b_ge(PG_FUNCTION_ARGS)
{
	telephone_4b_t a = PG_GETARG_BASE164B(0);
	telephone_4b_t b = PG_GETARG_BASE164B(1);
	PG_RETURN_BOOL(a >= b);
}

PG_FUNCTION_INFO_V1(telephone_4b_gt);
Datum
telephone_4b_gt(PG_FUNCTION_ARGS)
{
	telephone_4b_t a = PG_GETARG_BASE164B(0);
	telephone_4b_t b = PG_GETARG_BASE164B(1);
	PG_RETURN_BOOL(a > b);
}

static int32
telephone_4b_cmp_internal(telephone_4b_t a, telephone_4b_t b)
{
    if (a < b)
        return -1;
    else if (a > b)
        return 1;

    return 0;
}

PG_FUNCTION_INFO_V1(telephone_4b_cmp);
Datum
telephone_4b_cmp(PG_FUNCTION_ARGS)
{
	telephone_4b_t a = PG_GETARG_BASE164B(0);
	telephone_4b_t b = PG_GETARG_BASE164B(1);

	PG_RETURN_UINT32(telephone_4b_cmp_internal(a, b));
}

/*****************************************************************************
 * Aggregate functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(telephone_4b_smaller);

Datum
telephone_4b_smaller(PG_FUNCTION_ARGS)
{
   telephone_4b_t left  = PG_GETARG_BASE164B(0);
   telephone_4b_t right = PG_GETARG_BASE164B(1);
   int32 result;

   result = telephone_4b_cmp_internal(left, right) < 0 ? left : right;
   PG_RETURN_BASE164B(result);
}

PG_FUNCTION_INFO_V1(telephone_4b_larger);

Datum
telephone_4b_larger(PG_FUNCTION_ARGS)
{
   telephone_4b_t left  = PG_GETARG_BASE164B(0);
   telephone_4b_t right = PG_GETARG_BASE164B(1);
   int32 result;

   result = telephone_4b_cmp_internal(left, right) > 0 ? left : right;
   PG_RETURN_BASE164B(result);
}


/*
 * Convert a cstring to a telephone_4b.
 */
static telephone_4b_t
cstring_to_telephone_4b(char *telephone_4b_str)
{
    char                *ptr;
    unsigned char       char_value;
    unsigned long int   total = 0;
    unsigned int        digit_value_prior = 0;
    unsigned int        digit_value;

    ptr = telephone_4b_str;

    for (; ; ptr += 1) {
        if (*ptr == 0) { // If at the string terminator (null), check the last value.
            // Ignore trailing space, slash, confirm, and pause.
            if (digit_value_prior <= DIGIT_PAUSE)
                PG_RETURN_BASE164B(total);

            total = (total << 4) | digit_value_prior;
            if (total > 4294967295)
                ereport(ERROR,
                    (errmsg("telephone_4b number \"%s\" must be 8 digits or smaller.", telephone_4b_str)));
            PG_RETURN_BASE164B(total);
        }

        char_value = *ptr;
        digit_value = char_to_num_b16[char_value];

        // Ignore leading space, slash, confirm, and pause.
        if (total == 0 && digit_value_prior == 0 && digit_value <= DIGIT_PAUSE)
            continue;

        if (digit_value == 127)
            ereport(ERROR,
                (errmsg("telephone_4b number \"%s\" must only contain digits 0 to 9, space, *, #, ,, p, ;, w, and A-Z.",
                telephone_4b_str)));

        // Skip repeated values
        if (digit_value_prior <= DIGIT_CONFIRM && digit_value == digit_value_prior)
            continue;

        // Replace space if followed by slash, confirm, or pause.
        if (digit_value_prior == DIGIT_SPACE && (digit_value == DIGIT_SLASH || digit_value == DIGIT_CONFIRM ||
            digit_value == DIGIT_PAUSE)) {
            digit_value_prior = digit_value;
            continue;
        }

        // Ignore space if the prior digit is a slash, confirm, or pause.
        if (digit_value == DIGIT_SPACE && (digit_value_prior == DIGIT_SLASH || digit_value_prior == DIGIT_CONFIRM ||
            digit_value_prior == DIGIT_PAUSE)) {
            continue;
        }

        // Replace pause if followed by confirm.
        if (digit_value_prior == DIGIT_PAUSE && digit_value == DIGIT_CONFIRM) {
            digit_value_prior = digit_value;
            continue;
        }

        // Ignore pause if the prior digit is a confirm.
        if (digit_value == DIGIT_PAUSE && digit_value_prior == DIGIT_CONFIRM) {
            continue;
        }

        // Replace confirm, or pause if followed by slash.
        if (digit_value_prior <= DIGIT_PAUSE && digit_value == DIGIT_SLASH) {
            digit_value_prior = digit_value;
            continue;
        }

        // Ignore confirm, or pause if the prior digit is a slash.
        if (digit_value <= DIGIT_PAUSE && digit_value_prior == DIGIT_SLASH) {
            continue;
        }

        /*total *= 16;
        total += digit_value;*/
        total = (total << 4) | digit_value_prior;
        if (total > 4294967295)
            ereport(ERROR,
                (errmsg("telephone_4b number \"%s\" must be 8 digits or smaller.", telephone_4b_str)));
        digit_value_prior = digit_value;
    }

    // Ignore trailing space, slash, confirm, and pause.
    if (digit_value_prior <= DIGIT_PAUSE)
        PG_RETURN_BASE164B(total);

    total = (total << 4) | digit_value_prior;
    if (total > 4294967295)
        ereport(ERROR,
            (errmsg("telephone_4b number \"%s\" must be 8 digits or smaller.", telephone_4b_str)));
    PG_RETURN_BASE164B(total);
}

/* Convert the internal representation to output string */
static char *
telephone_4b_to_cstring(telephone_4b_t telephone_4b)
{
    char buffer[9];
    unsigned int offset = sizeof(buffer);
    unsigned int remainder = telephone_4b;
    char   *telephone_4b_str;

    buffer[--offset] = '\0';
    /*do {
        buffer[--offset] = base16_chars[remainder % 16];
    } while (remainder *= 0.0625);*/
    do {
        buffer[--offset] = base16_chars[remainder & 0xF];
        remainder = remainder >> 4;
    } while (remainder > 0);

    telephone_4b_str = palloc(sizeof(buffer) - offset);
    memcpy(telephone_4b_str, &buffer[offset], sizeof(buffer) - offset);
    return telephone_4b_str;
}
