/*
 * Author: The maintainer's name
 * Created at: Wed Oct 14 23:12:59 -0400 2015
 *
 */

SET client_min_messages = warning;

-- SQL definitions for USAEIN type
CREATE TYPE telephone_4b;

-- basic i/o functions
CREATE OR REPLACE FUNCTION telephone_4b_in(cstring) RETURNS telephone_4b AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION telephone_4b_out(telephone_4b) RETURNS cstring AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION telephone_4b_send(telephone_4b) RETURNS bytea AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION telephone_4b_recv(internal) RETURNS telephone_4b AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE telephone_4b (
	input = telephone_4b_in,
	output = telephone_4b_out,
	send = telephone_4b_send,
	receive = telephone_4b_recv,
	internallength = 4,
	passedbyvalue
);

-- functions to support btree opclass
CREATE OR REPLACE FUNCTION telephone_4b_lt(telephone_4b, telephone_4b) RETURNS bool AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION telephone_4b_le(telephone_4b, telephone_4b) RETURNS bool AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION telephone_4b_eq(telephone_4b, telephone_4b) RETURNS bool AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION telephone_4b_ne(telephone_4b, telephone_4b) RETURNS bool AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION telephone_4b_ge(telephone_4b, telephone_4b) RETURNS bool AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION telephone_4b_gt(telephone_4b, telephone_4b) RETURNS bool AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION telephone_4b_cmp(telephone_4b, telephone_4b) RETURNS int4 AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;

-- to/from text conversion
CREATE OR REPLACE FUNCTION telephone_4b_to_text(telephone_4b) RETURNS text AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION text_to_telephone_4b(text) RETURNS telephone_4b AS '$libdir/telephone_4b'
LANGUAGE C IMMUTABLE STRICT;

-- operators
CREATE OPERATOR < (
	leftarg = telephone_4b, rightarg = telephone_4b, procedure = telephone_4b_lt,
	commutator = >, negator = >=,
	restrict = scalarltsel, join = scalarltjoinsel
);
CREATE OPERATOR <= (
	leftarg = telephone_4b, rightarg = telephone_4b, procedure = telephone_4b_le,
	commutator = >=, negator = >,
	restrict = scalarltsel, join = scalarltjoinsel
);
CREATE OPERATOR = (
	leftarg = telephone_4b, rightarg = telephone_4b, procedure = telephone_4b_eq,
	commutator = =, negator = <>,
	restrict = eqsel, join = eqjoinsel,
	merges
);
CREATE OPERATOR <> (
	leftarg = telephone_4b, rightarg = telephone_4b, procedure = telephone_4b_ne,
	commutator = <>, negator = =,
	restrict = neqsel, join = neqjoinsel
);
CREATE OPERATOR > (
	leftarg = telephone_4b, rightarg = telephone_4b, procedure = telephone_4b_gt,
	commutator = <, negator = <=,
	restrict = scalargtsel, join = scalargtjoinsel
);
CREATE OPERATOR >= (
	leftarg = telephone_4b, rightarg = telephone_4b, procedure = telephone_4b_ge,
	commutator = <=, negator = <,
	restrict = scalargtsel, join = scalargtjoinsel
);

-- aggregates
CREATE OR REPLACE FUNCTION telephone_4b_smaller(telephone_4b, telephone_4b)
RETURNS telephone_4b
AS '$libdir/telephone_4b'
    LANGUAGE C IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION telephone_4b_larger(telephone_4b, telephone_4b)
RETURNS telephone_4b
AS '$libdir/telephone_4b'
    LANGUAGE C IMMUTABLE STRICT;

CREATE AGGREGATE min(telephone_4b)  (
    SFUNC = telephone_4b_smaller,
    STYPE = telephone_4b,
    SORTOP = <
);

CREATE AGGREGATE max(telephone_4b)  (
    SFUNC = telephone_4b_larger,
    STYPE = telephone_4b,
    SORTOP = >
);

-- btree operator class
CREATE OPERATOR CLASS telephone_4b_ops DEFAULT FOR TYPE telephone_4b USING btree AS
	OPERATOR 1 <,
	OPERATOR 2 <=,
	OPERATOR 3 =,
	OPERATOR 4 >=,
	OPERATOR 5 >,
	FUNCTION 1 telephone_4b_cmp(telephone_4b, telephone_4b);
-- cast from/to text
CREATE CAST (telephone_4b AS text) WITH FUNCTION telephone_4b_to_text(telephone_4b) AS ASSIGNMENT;
CREATE CAST (text AS telephone_4b) WITH FUNCTION text_to_telephone_4b(text) AS ASSIGNMENT;
