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

//PPLPARAMETERISEDEXCEPTION(NetworkException);
//PPLPARAMETERISEDEXCEPTION(IdnConversionException);

class ResultSet
{
	public:
		enum Type {
			ERROR=-1,
			UNKNOWN=0,
			INTEGER,
			LONGINTEGER,
			FLOAT,
			DOUBLE,
			BOOLEAN,
			STRING,
			DATETIME,
			BINARY
		};
		virtual ~ResultSet()=0;


		virtual	void		clear()=0;
		virtual ppluint64	rows() const=0;
		virtual ppluint64	affected() const=0;
		virtual int			fields() const=0;
		virtual String		getString(const String &fieldname)=0;
		virtual String		getString(int field)=0;
		virtual DateTime	getDateTime(const String &fieldname)=0;
		virtual DateTime	getDateTime(int field)=0;
		virtual int			getInt(const String &fieldname)=0;
		virtual int			getInt(int field)=0;
		virtual pplint64	getLongInt(const String &fieldname)=0;
		virtual pplint64	getLongInt(int field)=0;
		virtual int			fieldNum(const String &fieldname)=0;
		virtual String		fieldName(int field)=0;
		virtual Type		fieldType(int field)=0;
		virtual Type		fieldType(const String &fieldname)=0;
		virtual AssocArray	fetchArray()=0;
		virtual int			fetchArray(AssocArray &array)=0;
		virtual Array		fetchFields()=0;
		virtual int			fetchFields(Array &array)=0;
		virtual void		printRow() const=0;
		virtual void		printAll() const=0;
		virtual ppluint64	currentRow()=0;
		virtual bool		next()=0;

};

ppluint64 loadResultSet(std::list<AssocArray> &list, ResultSet &res);
Database *Connect(const AssocArray &params);

class Database
{
	friend class Pool;
	friend class PoolEx;

	private:
		AssocArray	ConnectParam;
		ppluint64	lastuse;
		ppluint64	lastping;
		String		Hash;		// Wird vom Pool verwendet
		Pool		*pool;
		PoolEx		*poolex;
		bool		poollock;

	protected:
		void    logQuery(const char *query, float duration);
		void	updateLastPing();
		void	updateLastUse();
		void	clearLastUse();

	public:
		Database();
		virtual ~Database();
		void	setParam(const String &name, const String &value);
		void	setParam(const String &name, int value);
		int		connect();
		int		close();

		int		execf(const char *query, ...);
		ResultSet	*queryf(const char *query, ...);
		void	freeResult(ResultSet *res);

		AssocArray execArrayf(const char *query, ...);
		AssocArray execArray(const String &query);
		AssocArray execArrayAllf(const char *query, ...);
		AssocArray execArrayAll(const String &query);
		int execArrayf(AssocArray &result, const char *query, ...);
		int execArray(AssocArray &result, const String &query);
		int execArrayAllf(AssocArray &result, const char *query, ...);
		int execArrayAll(AssocArray &result, const String &query);
		int save(const String &method, const String &table, const AssocArray &a, const String &clause="", const AssocArray &exclude=AssocArray(), const AssocArray &types=AssocArray());
		int saveGenQuery(String &Query, const String &method, const String &table, const AssocArray &a, const String &clause="", const AssocArray &exclude=AssocArray(), const AssocArray &types=AssocArray());
		int	readKeyValue(AssocArray &res, const String &query, const String &keyname, const String &valname=String());
		ppluint64 insertKey(const String &table, const AssocArray &a, const String &keyname, const AssocArray &exclude=AssocArray(), const AssocArray &types=AssocArray());


		virtual void		connect(const AssocArray &params);
		virtual void		connectCreate(const AssocArray &params);
		virtual void		reconnect();
		virtual void		disconnect();
		virtual void		selectDB(const String &databasename);
		virtual void 		exec(const String &query);
		virtual ResultSet	*query(const String &query);
		virtual void		setMaxRows(ppluint64 rows);
		virtual int			ping();
		virtual String		escape(const String &str) const;
		virtual ppluint64	getInsertID();	// Returns ID from autoincrement field
		virtual ppluint64	getAffectedRows();
		virtual int			startTransaction();
		virtual int			endTransaction();
		virtual int			cancelTransaction();
		virtual int			cancelTransactionComplete();
		virtual int			createDatabase(const String &name);
		virtual String		databaseType() const;
		virtual String		getQuoted(const String &value, const String &type=String()) const;

};


class PostgreSQL: public Database
{
	private:
		void		*conn;
		ppluint64	lastinsertid;
		pplint64	affectedrows;
		ppluint64	maxrows;
		int			transactiondepth;
		AssocArray	condata;

		void *Pgsql_Query(const String &query);

	public:
		PostgreSQL();
		virtual ~PostgreSQL();

		virtual void		connect(const AssocArray &params);
		virtual void		connectCreate(const AssocArray &params);
		virtual void		reconnect();
		virtual void		disconnect();
		virtual void		selectDB(const String &databasename);
		virtual void		exec(const String &query);
		virtual ResultSet	*query(const String &query);
		virtual void		setMaxRows(ppluint64 rows);
		virtual int			ping();
		virtual String		escape(const String &str) const;
		virtual ppluint64	getInsertID();
		virtual ppluint64	getAffectedRows();
		virtual int			startTransaction();
		virtual int			endTransaction();
		virtual int			cancelTransaction();
		virtual int			cancelTransactionComplete();
		virtual int			createDatabase(const String &namee);
		virtual String		databaseType() const;
		virtual String		getQuoted(const String &value, const String &type=String()) const;
};



} // EOF namespace db
} // EOF namespace ppl7

#endif /* PPL7INET_H_ */
