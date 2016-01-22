/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 *******************************************************************************
 * Copyright (c) 2016, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright notice, this
 *       list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice,
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <locale.h>
#include <ppl7.h>
#include <ppl7-db.h>
#include <gtest/gtest.h>
#include "ppl7-tests.h"
#include <list>

namespace {

// The fixture for testing class Foo.
class DBMySQLTest : public ::testing::Test {
	protected:
		DBMySQLTest() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~DBMySQLTest() {

	}
};


#ifdef TODO

TEST_F(DBMySQLTest, connect) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	try {
		ppl7::db::MySQL db;
		try {
			db.connect(params);
		} catch (const ppl7::Exception &e) {
			e.print();
			FAIL() << "connect to database failed";
		} catch (...) {
			printf ("Das ist unerwartet\n");
		}
		try {
			db.close();
		} catch (const ppl7::Exception &e) {
			e.print();
			FAIL() << "close on database failed";
		}
	} catch (const ppl7::Exception &e) {
		e.print();
		FAIL() << "destructor throwed an unxpected exception";
	}
}

TEST_F(DBMySQLTest, setParamAndConnect) {
	ppl7::db::MySQL db;
	db.setParam("filename","tmp/sqlite_test.db");
	try {
		db.connect();
	} catch (const ppl7::Exception &e) {
		e.print();
		FAIL() << "connect to database failed";
	} catch (...) {
		printf ("Das ist unerwartet\n");
	}
	try {
		db.close();
	} catch (const ppl7::Exception &e) {
		e.print();
		FAIL() << "close on database failed";
	}
}

TEST_F(DBMySQLTest, ping) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_TRUE(db.ping());
	db.close();
	ASSERT_FALSE(db.ping());
}


TEST_F(DBMySQLTest, reconnect) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_NO_THROW({
		db.reconnect();
	});
	ASSERT_TRUE(db.ping());
}

TEST_F(DBMySQLTest, closeAndReconnect) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	db.close();
	ASSERT_NO_THROW({
		db.reconnect();
	});
	ASSERT_TRUE(db.ping());
}

TEST_F(DBMySQLTest, selectDB) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_TRUE(db.ping());
	ASSERT_THROW({
		db.selectDB(PPL7TestConfig.getFromSection("postgres","dbname"));
	},ppl7::UnsupportedFeatureException);
	ASSERT_TRUE(db.ping());
}

TEST_F(DBMySQLTest, execTransactionalInsertAndCommit) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_NO_THROW({
		db.startTransaction();
	});
	ASSERT_NO_THROW({
		try {
			db.exec("insert into test_insert (login,name,surename) values ('test1','Tester','Fritz')");
		} catch (const ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
	ASSERT_EQ((ppluint64)1,db.getAffectedRows());
	ASSERT_NO_THROW({
		db.exec("insert into test_insert (login,name,surename) values ('test2','Tester','Hans')");
	});
	ASSERT_EQ((ppluint64)1,db.getAffectedRows());
	ASSERT_NO_THROW({
		db.endTransaction();
	});
	ASSERT_EQ((ppluint64)1,db.getAffectedRows());

}

TEST_F(DBMySQLTest, execCreateTable) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_NO_THROW({
		db.exec("drop table if exists testcreatetable");
	});
	ASSERT_NO_THROW({
		try {
			db.exec("create table testcreatetable (id int4 not null)");
		} catch (const ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(DBMySQLTest, queryExistingDataGetStringEofNextRow) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ppl7::db::ResultSet *res;
	ASSERT_NO_THROW({
		res=db.query("select userid,name,surename,age from user order by userid");
	});
	ASSERT_TRUE(res!=NULL);
	ASSERT_EQ((ppluint64)0,db.getAffectedRows());
	ASSERT_EQ((ppluint64)0,res->affected());	// bei Insert immer 0
	ASSERT_EQ((ppluint64)4,res->fields());
	ASSERT_EQ(ppl7::String("Fedick"),res->getString("name"));
	ASSERT_EQ(ppl7::String("Patrick"),res->getString("surename"));
	ASSERT_EQ(ppl7::String("42"),res->getString("age"));

	EXPECT_EQ(ppl7::String("1"),res->getString(0));
	EXPECT_EQ(ppl7::String("Fedick"),res->getString(1));
	EXPECT_EQ(ppl7::String("Patrick"),res->getString(2));
	EXPECT_EQ(ppl7::String("42"),res->getString(3));

	ASSERT_FALSE(res->eof());

	ASSERT_NO_THROW({
		res->nextRow();
	});
	ASSERT_EQ(ppl7::String("2"),res->getString(0));
	ASSERT_EQ(ppl7::String("Mueller"),res->getString(1));
	ASSERT_EQ(ppl7::String("Klaus"),res->getString(2));
	ASSERT_EQ(ppl7::String("31"),res->getString(3));

	ASSERT_NO_THROW({
			res->nextRow();
		});

	ASSERT_NO_THROW({
			res->nextRow();
		});
	ASSERT_EQ(ppl7::String("4"),res->getString(0));
	ASSERT_EQ(ppl7::String("Johann"),res->getString(1));
	ASSERT_EQ(ppl7::String("Paul"),res->getString(2));
	ASSERT_EQ(ppl7::String("77"),res->getString(3));
	ASSERT_NO_THROW({
			res->nextRow();
		});
	ASSERT_TRUE(res->eof());

	ASSERT_NO_THROW({
		delete res;
	});
}

TEST_F(DBMySQLTest, queryFieldNumFieldName) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ppl7::db::ResultSet *res;
	ASSERT_NO_THROW({
		res=db.query("select userid,name,surename,age from user order by userid");
	});
	ASSERT_TRUE(res!=NULL);
	ASSERT_EQ((int)0,res->fieldNum("userid"));
	ASSERT_EQ((int)1,res->fieldNum("name"));
	ASSERT_EQ((int)2,res->fieldNum("surename"));
	ASSERT_EQ((int)3,res->fieldNum("age"));

	ASSERT_EQ((ppl7::String("userid")),res->fieldName(0));
	ASSERT_EQ((ppl7::String("name")),res->fieldName(1));
	ASSERT_EQ((ppl7::String("surename")),res->fieldName(2));
	ASSERT_EQ((ppl7::String("age")),res->fieldName(3));

	ASSERT_NO_THROW({
		delete res;
	});
}


TEST_F(DBMySQLTest, queryFetchArray) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ppl7::db::ResultSet *res;
	ASSERT_NO_THROW({
		res=db.query("select userid,name,surename,age from user order by userid");
	});
	ASSERT_TRUE(res!=NULL);
	std::list<ppl7::AssocArray> resultset;
	ASSERT_NO_THROW({
		while (!res->eof()) {
			resultset.push_back(res->fetchArray());
		}
	});
	ASSERT_EQ((size_t)4,resultset.size());
	const ppl7::AssocArray &a=resultset.front();

	ASSERT_EQ((size_t)4,a.size());
	ASSERT_NO_THROW({
		ASSERT_EQ(ppl7::String("1"),a["userid"]);
		ASSERT_EQ(ppl7::String("Fedick"),a["name"]);
		ASSERT_EQ(ppl7::String("Patrick"),a["surename"]);
		ASSERT_EQ(ppl7::String("42"),a["age"]);
	});
	ASSERT_NO_THROW({
		delete res;
	});
}


TEST_F(DBMySQLTest, queryFetchFields) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ppl7::db::ResultSet *res;
	ASSERT_NO_THROW({
		res=db.query("select userid,name,surename,age from user order by userid");
	});
	ASSERT_TRUE(res!=NULL);
	std::list<ppl7::Array> resultset;
	ASSERT_NO_THROW({
		while (!res->eof()) {
			resultset.push_back(res->fetchFields());
		}
	});
	ASSERT_EQ((size_t)4,resultset.size());
	const ppl7::Array &a=resultset.front();
	ASSERT_EQ((size_t)4,a.size());
	ASSERT_NO_THROW({
		ASSERT_EQ(ppl7::String("1"),a[0]);
		ASSERT_EQ(ppl7::String("Fedick"),a[1]);
		ASSERT_EQ(ppl7::String("Patrick"),a[2]);
		ASSERT_EQ(ppl7::String("42"),a[3]);
	});
	ASSERT_NO_THROW({
		delete res;
	});
}

TEST_F(DBMySQLTest, count) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_NO_THROW({
		ASSERT_EQ((ppluint64)4, db.count("user"));
	});
	ASSERT_NO_THROW({
		try {
			ASSERT_EQ((ppluint64)2, db.count("user","userid<3"));
		} catch (const ppl7::Exception &e) {
			e.print();
			throw;
		}
	});
}

TEST_F(DBMySQLTest, execArray) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		db.execArray(a,"select * from user where userid=1");
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(ppl7::String("1"),a["userid"]);
		ASSERT_EQ(ppl7::String("Fedick"),a["name"]);
		ASSERT_EQ(ppl7::String("Patrick"),a["surename"]);
		ASSERT_EQ(ppl7::String("42"),a["age"]);
	});
	ASSERT_NO_THROW({
		a=db.execArray("select * from user where userid=1");
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(ppl7::String("1"),a["userid"]);
		ASSERT_EQ(ppl7::String("Fedick"),a["name"]);
		ASSERT_EQ(ppl7::String("Patrick"),a["surename"]);
		ASSERT_EQ(ppl7::String("42"),a["age"]);
	});
}

TEST_F(DBMySQLTest, execArrayf) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		db.execArrayf(a,"select * from user where userid=%d",1);
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(ppl7::String("1"),a["userid"]);
		ASSERT_EQ(ppl7::String("Fedick"),a["name"]);
		ASSERT_EQ(ppl7::String("Patrick"),a["surename"]);
		ASSERT_EQ(ppl7::String("42"),a["age"]);
	});
	ASSERT_NO_THROW({
		a=db.execArrayf("select * from user where userid=%d",1);
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(ppl7::String("1"),a["userid"]);
		ASSERT_EQ(ppl7::String("Fedick"),a["name"]);
		ASSERT_EQ(ppl7::String("Patrick"),a["surename"]);
		ASSERT_EQ(ppl7::String("42"),a["age"]);
	});
}

TEST_F(DBMySQLTest, execArrayAll) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		db.execArrayAll(a,"select * from user");
	});
	ASSERT_EQ((size_t)4,a.size());
	ASSERT_NO_THROW({
		ASSERT_EQ(ppl7::String("1"),a["0/userid"]);
		ASSERT_EQ(ppl7::String("Fedick"),a["0/name"]);
		ASSERT_EQ(ppl7::String("Patrick"),a["0/surename"]);
		ASSERT_EQ(ppl7::String("42"),a["0/age"]);
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(ppl7::String("4"),a["3/userid"]);
		ASSERT_EQ(ppl7::String("Johann"),a["3/name"]);
		ASSERT_EQ(ppl7::String("Paul"),a["3/surename"]);
		ASSERT_EQ(ppl7::String("77"),a["3/age"]);
	});
	ASSERT_NO_THROW({
		a=db.execArrayAll("select * from user");
	});
	ASSERT_EQ((size_t)4,a.size());
}

TEST_F(DBMySQLTest, execArrayAllf) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ppl7::AssocArray a;
	ASSERT_NO_THROW({
		db.execArrayAllf(a,"select * from user where userid>%d",0);
	});
	ASSERT_EQ((size_t)4,a.size());
	ASSERT_NO_THROW({
		ASSERT_EQ(ppl7::String("1"),a["0/userid"]);
		ASSERT_EQ(ppl7::String("Fedick"),a["0/name"]);
		ASSERT_EQ(ppl7::String("Patrick"),a["0/surename"]);
		ASSERT_EQ(ppl7::String("42"),a["0/age"]);
	});
	ASSERT_NO_THROW({
		ASSERT_EQ(ppl7::String("4"),a["3/userid"]);
		ASSERT_EQ(ppl7::String("Johann"),a["3/name"]);
		ASSERT_EQ(ppl7::String("Paul"),a["3/surename"]);
		ASSERT_EQ(ppl7::String("77"),a["3/age"]);
	});
	ASSERT_NO_THROW({
		a=db.execArrayAllf("select * from user where userid>%d",0);
	});
	ASSERT_EQ((size_t)4,a.size());
}

TEST_F(DBMySQLTest, execInsertAndCount) {
	ppl7::AssocArray params;
	params.set("filename","tmp/sqlite_test.db");
	ppl7::db::MySQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_NO_THROW({
		db.exec("insert into test_insert_get_insert_id (name,surename) values ('Tester','Klaus')");
		db.exec("insert into test_insert_get_insert_id (name,surename) values ('Tester','Hans')");
	});
	ASSERT_NO_THROW({
		ASSERT_EQ((ppluint64)2, db.count("test_insert_get_insert_id"));
	});
}

#endif

}	// EOF namespace

