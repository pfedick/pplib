/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author$
 * $Revision$
 * $Date$
 * $Id$
 *
 *******************************************************************************
 * Copyright (c) 2013, Patrick Fedick <patrick@pfp.de>
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

namespace {

// The fixture for testing class Foo.
class DBPostgreSQL : public ::testing::Test {
	protected:
	DBPostgreSQL() {
		if (setlocale(LC_CTYPE,DEFAULT_LOCALE)==NULL) {
			printf ("setlocale fehlgeschlagen\n");
			throw std::exception();
		}
	}
	virtual ~DBPostgreSQL() {

	}
};

TEST_F(DBPostgreSQL, connect) {
	//ppl7::db::PostgreSQL db;
	ppl7::AssocArray params;
	PPL7TestConfig.copySection(params, "postgres");
	//params.list();
	try {
		ppl7::db::PostgreSQL db;
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

TEST_F(DBPostgreSQL, setParamAndConnect) {
	//ppl7::db::PostgreSQL db;
	ppl7::AssocArray params;
	PPL7TestConfig.copySection(params, "postgres");
	ppl7::db::PostgreSQL db;
	db.setParam("host",params["host"]);
	db.setParam("port",params["port"]);
	db.setParam("dbname",params["dbname"]);
	db.setParam("user",params["user"]);
	db.setParam("password",params["password"]);
	db.setParam("timeout",params["timeout"]);

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


TEST_F(DBPostgreSQL, ping) {
	//ppl7::db::PostgreSQL db;
	ppl7::AssocArray params;
	PPL7TestConfig.copySection(params, "postgres");
	ppl7::db::PostgreSQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_TRUE(db.ping());
	db.close();
	ASSERT_FALSE(db.ping());
}

TEST_F(DBPostgreSQL, reconnect) {
	//ppl7::db::PostgreSQL db;
	ppl7::AssocArray params;
	PPL7TestConfig.copySection(params, "postgres");
	ppl7::db::PostgreSQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_NO_THROW({
		db.reconnect();
	});
	ASSERT_TRUE(db.ping());
}

TEST_F(DBPostgreSQL, closeAndReconnect) {
	//ppl7::db::PostgreSQL db;
	ppl7::AssocArray params;
	PPL7TestConfig.copySection(params, "postgres");
	ppl7::db::PostgreSQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	db.close();
	ASSERT_NO_THROW({
		db.reconnect();
	});
	ASSERT_TRUE(db.ping());
}

TEST_F(DBPostgreSQL, connectToPostgres) {
	//ppl7::db::PostgreSQL db;
	ppl7::AssocArray params;
	PPL7TestConfig.copySection(params, "postgres");
	params.set("dbname","postgres");
	ppl7::db::PostgreSQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_TRUE(db.ping());
}

TEST_F(DBPostgreSQL, selectDB) {
	ppl7::AssocArray params;
	PPL7TestConfig.copySection(params, "postgres");
	params.set("dbname","postgres");
	ppl7::db::PostgreSQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_TRUE(db.ping());
	ASSERT_NO_THROW({
		db.selectDB(PPL7TestConfig.getFromSection("postgres","dbname"));
	});
	ASSERT_TRUE(db.ping());
}

TEST_F(DBPostgreSQL, execCreateTable) {
	ppl7::AssocArray params;
	PPL7TestConfig.copySection(params, "postgres");
	ppl7::db::PostgreSQL db;
	ASSERT_NO_THROW({
		db.connect(params);
	});
	ASSERT_NO_THROW({
		//db.exec("create schema if not exists ppl7");
	});
	ASSERT_NO_THROW({
		//db.exec("drop table if exists ppl7.testcreatetable");
	});
	try {
		//db.exec("create table ppl7.testcreatetable (id int4 not null)");
	} catch (const ppl7::Exception &e) {
		e.print();
		FAIL()<<"unexpected exception";
	}

}




}	// EOF namespace

