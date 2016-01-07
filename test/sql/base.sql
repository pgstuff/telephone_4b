\set ECHO none
\i sql/telephone_4b.sql
\set ECHO all

CREATE TABLE telephone_4bs(id serial primary key, telephone_4b telephone_4b unique);
INSERT INTO telephone_4bs(telephone_4b) VALUES('12345678');
INSERT INTO telephone_4bs(telephone_4b) VALUES('9*#');
INSERT INTO telephone_4bs(telephone_4b) VALUES('1 1');
INSERT INTO telephone_4bs(telephone_4b) VALUES('2p2');
INSERT INTO telephone_4bs(telephone_4b) VALUES('2w2');
INSERT INTO telephone_4bs(telephone_4b) VALUES('1/1');
INSERT INTO telephone_4bs(telephone_4b) VALUES('1,1');
INSERT INTO telephone_4bs(telephone_4b) VALUES('1;1');;
INSERT INTO telephone_4bs(telephone_4b) VALUES('10000003');
INSERT INTO telephone_4bs(telephone_4b) VALUES('20000002');
INSERT INTO telephone_4bs(telephone_4b) VALUES('30000001');
INSERT INTO telephone_4bs(telephone_4b) VALUES('4  4');
INSERT INTO telephone_4bs(telephone_4b) VALUES('4 / 4');
INSERT INTO telephone_4bs(telephone_4b) VALUES('4 ; 4');
INSERT INTO telephone_4bs(telephone_4b) VALUES('4 , 4');
INSERT INTO telephone_4bs(telephone_4b) VALUES('5 // 5');
INSERT INTO telephone_4bs(telephone_4b) VALUES('5 ;; 5');
INSERT INTO telephone_4bs(telephone_4b) VALUES('5 ,, 5');
INSERT INTO telephone_4bs(telephone_4b) VALUES('6 , / , 6');
INSERT INTO telephone_4bs(telephone_4b) VALUES('6 , ; , 6');
INSERT INTO telephone_4bs(telephone_4b) VALUES('7 , / , 7');
INSERT INTO telephone_4bs(telephone_4b) VALUES('7 , ; , 7');
INSERT INTO telephone_4bs(telephone_4b) VALUES(''); -- test limits
INSERT INTO telephone_4bs(telephone_4b) VALUES('########');

SELECT * FROM telephone_4bs ORDER BY telephone_4b;

SELECT MIN(telephone_4b) AS min FROM telephone_4bs;
SELECT MAX(telephone_4b) AS max FROM telephone_4bs;

-- index scan
TRUNCATE telephone_4bs;
INSERT INTO telephone_4bs(telephone_4b) SELECT '4'||id FROM generate_series(5678, 8000) id;
SELECT id,telephone_4b::text FROM telephone_4bs WHERE telephone_4b = '48000';

SET enable_seqscan = false;
SELECT id,telephone_4b::text FROM telephone_4bs WHERE telephone_4b = '46000';
SELECT id,telephone_4b FROM telephone_4bs WHERE telephone_4b >= '47000' LIMIT 5;
SELECT count(id) FROM telephone_4bs;
SELECT count(id) FROM telephone_4bs WHERE telephone_4b <> ('46500'::text)::telephone_4b;
RESET enable_seqscan;

-- operators and conversions
SELECT '0'::telephone_4b < '0'::telephone_4b;
SELECT '0'::telephone_4b > '0'::telephone_4b;
SELECT '0'::telephone_4b < '1'::telephone_4b;
SELECT '0'::telephone_4b > '1'::telephone_4b;
SELECT '0'::telephone_4b <= '0'::telephone_4b;
SELECT '0'::telephone_4b >= '0'::telephone_4b;
SELECT '0'::telephone_4b <= '1'::telephone_4b;
SELECT '0'::telephone_4b >= '1'::telephone_4b;
SELECT '0'::telephone_4b <> '0'::telephone_4b;
SELECT '0'::telephone_4b <> '1'::telephone_4b;
SELECT '0'::telephone_4b = '0'::telephone_4b;
SELECT '0'::telephone_4b = '1'::telephone_4b;

-- COPY FROM/TO
TRUNCATE telephone_4bs;
COPY telephone_4bs(telephone_4b) FROM STDIN;

########
\.
COPY telephone_4bs TO STDOUT;

-- clean up --
DROP TABLE telephone_4bs;

-- errors
SELECT '100000000'::telephone_4b;
SELECT '!'::telephone_4b;
