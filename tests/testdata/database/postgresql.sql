CREATE SCHEMA ppl7_test;
set search_path = ppl7_test;

CREATE SEQUENCE userid_seq;

CREATE TABLE user (
	userid		int4 NOT NULL DEFAULT nextval(userid_seq)
	name		varchar(64) not null,
	surename	varchar(64) not null,
	age			int4 not null,
	PRIMARY KEY(userid)
)
WITHOUT OIDS 
TABLESPACE pg_default;
	
ALTER SEQUENCE userid_seq OWNED BY user.userid;


insert into user (name,surename,age) values ('Fedick','Patrick',42);
insert into user (name,surename,age) values ('Mueller','Klaus',31);
insert into user (name,surename,age) values ('Schmitt','Frank',19);
insert into user (name,surename,age) values ('Johann','Paul',77);
