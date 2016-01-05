CREATE SCHEMA ppl7_test;
set search_path = ppl7_test;


drop table if exists ppl7_test.user;


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

drop table if exists ppl7_test.test_insert;
CREATE TABLE ppl7_test.test_insert (
    login		varchar(64) not null primary key,
	name		varchar(64) not null,
	surename	varchar(64) not null
)
WITHOUT OIDS 
TABLESPACE pg_default;

drop table if exists ppl7_test.test_insert_get_insert_id;
CREATE TABLE ppl7_test.test_insert_get_insert_id (
    userid		SERIAL,
	name		varchar(64) not null,
	surename	varchar(64) not null
)
WITHOUT OIDS 
TABLESPACE pg_default;

drop table if exists ppl7_test.test_update;
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

drop table if exists ppl7_test.test_delete;
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

