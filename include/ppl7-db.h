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

#ifndef PPL7DB_H_
#define PPL7DB_H_

#ifndef _PPL7_INCLUDE
    #ifdef PPL7LIB
        #include "ppl7.h"
    #else
        #include <ppl7.h>
    #endif
#endif

#include <list>


namespace ppl7 {
namespace db {

class Pool;
class PoolEx;
class Database;

class ConnectParameter
{
	public:
		ConnectParameter();
		String	Hostname;
		int		Port;
		String	DatabaseName;
		String	UserName;
		String	Password;
};

class ResultSet
{
	public:
		enum FieldType {
			TYPE_UNKNOWN=0,
			TYPE_INTEGER,
			TYPE_LONGINTEGER,
			TYPE_FLOAT,
			TYPE_DOUBLE,
			TYPE_BOOLEAN,
			TYPE_STRING,
			TYPE_DATETIME,
			TYPE_BINARY
		};
		virtual ~ResultSet() {};
		virtual	void		clear()=0;
		virtual ppluint64	affected() const=0;
		virtual int			fields() const=0;
		virtual String		getString(const String &fieldname)=0;
		virtual String		getString(int field)=0;
		virtual int			fieldNum(const String &fieldname)=0;
		virtual String		fieldName(int field)=0;
		virtual FieldType	fieldType(int field)=0;
		virtual FieldType	fieldType(const String &fieldname)=0;
		virtual AssocArray	fetchArray()=0;
		virtual void		fetchArray(AssocArray &array)=0;
		virtual Array		fetchFields()=0;
		virtual void		fetchFields(Array &array)=0;
		virtual void		nextRow()=0;
		virtual bool		eof()=0;
};

Database *Connect(const AssocArray &params);
void copyResultToAssocArray(ResultSet *res, AssocArray &array);

class Database
{
	friend class Pool;
	friend class PoolEx;

	private:
		AssocArray	ConnectParam;
		Logger		*Log;
		ppluint64	lastuse;
		ppluint64	lastping;

	protected:
		void    logQuery(const String &query, float duration);
		void	updateLastPing();
		void	updateLastUse();

	public:
		Database();
		virtual ~Database();
		void	setLogger(Logger &logger);
		void	removeLogger();
		Logger	*getLogger();
		void	setParam(const String &name, const String &value);
		void	setParam(const String &name, int value);
		void	setParam(const AssocArray &params);

		void		execf(const char *query, ...);
		ResultSet	*queryf(const char *query, ...);

		AssocArray execArrayf(const char *query, ...);
		AssocArray execArray(const String &query);
		AssocArray execArrayAllf(const char *query, ...);
		AssocArray execArrayAll(const String &query);
		void execArrayf(AssocArray &result, const char *query, ...);
		void execArray(AssocArray &result, const String &query);
		void execArrayAllf(AssocArray &result, const char *query, ...);
		void execArrayAll(AssocArray &result, const String &query);

		ppluint64 count(const String &table, const String &where=String());
		/*
		void save(const String &method, const String &table, const AssocArray &a, const String &clause="", const AssocArray &exclude=AssocArray(), const AssocArray &types=AssocArray());
		String genQuery(const String &method, const String &table, const AssocArray &a, const String &clause="", const AssocArray &exclude=AssocArray(), const AssocArray &types=AssocArray());
		void readKeyValue(AssocArray &res, const String &query, const String &keyname, const String &valname=String());
		*/
		virtual void		connect();
		virtual void		connect(const AssocArray &params);
		virtual void		connectCreate(const AssocArray &params);
		virtual void		reconnect();
		virtual void		close();
		virtual void		selectDB(const String &databasename);
		virtual void 		exec(const String &query);
		virtual ResultSet	*query(const String &query);
		virtual bool		ping();
		virtual String		escape(const String &str) const;
		virtual ppluint64	getAffectedRows();
		virtual void		startTransaction();
		virtual void		endTransaction();
		virtual void		cancelTransaction();
		virtual void		cancelTransactionComplete();
		virtual void		createDatabase(const String &name);
		virtual String		databaseType() const;
		virtual String		getQuoted(const String &value, const String &type=String()) const;
		/*
		virtual void        prepare(const String &preparedStatementName, const String &query);
		virtual ResultSet	*execute(const String &preparedStatementName, const Array &params);
		virtual void		deallocate(const String &preparedStatementName);
		*/
};

class PostgreSQL: public Database
{
	private:
		void		*conn;
		ppluint64	affectedrows;
		int			transactiondepth;
		AssocArray	condata;

		void *pgsqlQuery(const String &query);

	public:
		PostgreSQL();
		virtual ~PostgreSQL();

		virtual void		connect();
		virtual void		connect(const AssocArray &params);
		virtual void		connectCreate(const AssocArray &params);
		virtual void		reconnect();
		virtual void		close();
		virtual void		selectDB(const String &databasename);
		virtual void		exec(const String &query);
		virtual ResultSet	*query(const String &query);
		virtual bool		ping();
		virtual String		escape(const String &str) const;
		virtual ppluint64	getAffectedRows();
		virtual void		startTransaction();
		virtual void		endTransaction();
		virtual void		cancelTransaction();
		virtual void		cancelTransactionComplete();
		virtual void		createDatabase(const String &name);
		virtual String		databaseType() const;
		virtual String		getQuoted(const String &value, const String &type=String()) const;
		/*
		virtual void        prepare(const String &preparedStatementName, const String &query);
		virtual ResultSet	*execute(const String &preparedStatementName, const Array &params);
		virtual void		deallocate(const String &preparedStatementName);
		*/
};

class MySQL: public Database
{
	private:
		void		*conn;
		ppluint64	affectedrows;
		int			transactiondepth;
		AssocArray	condata;

		void *mysqlQuery(const String &query);

	public:
		MySQL();
		virtual ~MySQL();

		virtual void		connect();
		virtual void		connect(const AssocArray &params);
		virtual void		connectCreate(const AssocArray &params);
		virtual void		reconnect();
		virtual void		close();
		virtual void		selectDB(const String &databasename);
		virtual void		exec(const String &query);
		virtual ResultSet	*query(const String &query);
		virtual bool		ping();
		virtual String		escape(const String &str) const;
		virtual ppluint64	getAffectedRows();
		virtual void		startTransaction();
		virtual void		endTransaction();
		virtual void		cancelTransaction();
		virtual void		cancelTransactionComplete();
		virtual void		createDatabase(const String &name);
		virtual String		databaseType() const;
		virtual String		getQuoted(const String &value, const String &type=String()) const;
};

class SQLite: public Database
{
	private:
		void		*conn;
		ppluint64	affectedrows;
		int			transactiondepth;
		AssocArray	condata;

		void *sqliteQuery(const String &query);

	public:
		SQLite();
		virtual ~SQLite();

		virtual void		connect();
		virtual void		connect(const AssocArray &params);
		virtual void		connectCreate(const AssocArray &params);
		virtual void		reconnect();
		virtual void		close();
		virtual void		selectDB(const String &databasename);
		virtual void		exec(const String &query);
		virtual ResultSet	*query(const String &query);
		virtual bool		ping();
		virtual String		escape(const String &str) const;
		virtual ppluint64	getAffectedRows();
		virtual void		startTransaction();
		virtual void		endTransaction();
		virtual void		cancelTransaction();
		virtual void		cancelTransactionComplete();
		virtual void		createDatabase(const String &name);
		virtual String		databaseType() const;
		virtual String		getQuoted(const String &value, const String &type=String()) const;
};


} // EOF namespace db
} // EOF namespace ppl7

#endif /* PPL7INET_H_ */
