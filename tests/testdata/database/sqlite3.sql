drop table if exists user;


CREATE TABLE user (
	userid		INTEGER PRIMARY KEY AUTOINCREMENT,
	name		varchar(64) not null,
	surename	varchar(64) not null,
	age			int4 not null
);

insert into user (name,surename,age) values ('Fedick','Patrick',42);
insert into user (name,surename,age) values ('Mueller','Klaus',31);
insert into user (name,surename,age) values ('Schmitt','Frank',19);
insert into user (name,surename,age) values ('Johann','Paul',77);

drop table if exists test_insert;
CREATE TABLE test_insert (
    login		varchar(64) not null primary key,
	name		varchar(64) not null,
	surename	varchar(64) not null
);

drop table if exists test_insert_get_insert_id;
CREATE TABLE test_insert_get_insert_id (
    userid		INTEGER PRIMARY KEY AUTOINCREMENT,
	name		varchar(64) not null,
	surename	varchar(64) not null
);

drop table if exists test_update;
CREATE TABLE test_update (
	login		varchar(64) not null primary key,
	name		varchar(64) not null,
	surename	varchar(64) not null
);
insert into test_update (login,name,surename) values ('patrick','Fedick','Patrick');
insert into test_update (login,name,surename) values ('test1','Tester','Klaus');
insert into test_update (login,name,surename) values ('test2','Tester','Fritz');

drop table if exists test_delete;
CREATE TABLE test_delete (
	login		varchar(64) not null primary key,
	name		varchar(64) not null,
	surename	varchar(64) not null
);

insert into test_delete (login,name,surename) values ('patrick','Fedick','Patrick');
insert into test_delete (login,name,surename) values ('test1','Tester','Klaus');
insert into test_delete (login,name,surename) values ('test2','Tester','Fritz');

