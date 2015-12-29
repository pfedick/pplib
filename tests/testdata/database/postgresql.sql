CREATE SCHEMA ppl7_test;
set search_path = ppl7_test;

CREATE TABLE ppl7_test.user (
	userid		SERIAL,
	name		varchar(64) not null,
	surename	varchar(64) not null,
	age			int4 not null,
	PRIMARY KEY(userid)
)
WITHOUT OIDS 
TABLESPACE pg_default;
	

insert into ppl7_test.user (name,surename,age) values ('Fedick','Patrick',42);
insert into ppl7_test.user (name,surename,age) values ('Mueller','Klaus',31);
insert into ppl7_test.user (name,surename,age) values ('Schmitt','Frank',19);
insert into ppl7_test.user (name,surename,age) values ('Johann','Paul',77);

CREATE TABLE ppl7_test.test_insert (
    login		varchar(64) not null primary key,
	name		varchar(64) not null,
	surename	varchar(64) not null
)
WITHOUT OIDS 
TABLESPACE pg_default;

CREATE TABLE ppl7_test.test_insert_get_insert_id (
    userid		SERIAL,
	name		varchar(64) not null,
	surename	varchar(64) not null
)
WITHOUT OIDS 
TABLESPACE pg_default;


CREATE TABLE ppl7_test.test_update (
	login		varchar(64) not null primary key,
	name		varchar(64) not null,
	surename	varchar(64) not null
)
WITHOUT OIDS 
TABLESPACE pg_default;

insert into ppl7_test.test_update (login,name,surename) values ('patrick','Fedick','Patrick');
insert into ppl7_test.test_update (login,name,surename) values ('test1','Tester','Klaus');
insert into ppl7_test.test_update (login,name,surename) values ('test2','Tester','Fritz');

CREATE TABLE ppl7_test.test_delete (
	login		varchar(64) not null primary key,
	name		varchar(64) not null,
	surename	varchar(64) not null
)
WITHOUT OIDS 
TABLESPACE pg_default;

insert into ppl7_test.test_delete (login,name,surename) values ('patrick','Fedick','Patrick');
insert into ppl7_test.test_delete (login,name,surename) values ('test1','Tester','Klaus');
insert into ppl7_test.test_delete (login,name,surename) values ('test2','Tester','Fritz');

