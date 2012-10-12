/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 7 (PPL7).
 * Web: http://www.pfp.de/ppl/
 *
 * $Author: pafe $
 * $Revision: 327 $
 * $Date: 2012-02-11 10:18:23 +0000 (Sa, 11 Feb 2012) $
 * $Id: ppl7-inet.h 327 2012-02-11 10:18:23Z pafe $
 * $URL: https://pplib.svn.sourceforge.net/svnroot/pplib/lib/trunk/include/ppl7-inet.h $
 *
 *******************************************************************************
 * Copyright (c) 2012, Patrick Fedick <patrick@pfp.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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
		virtual int			fieldNum(const String &fieldname)=0;
		virtual String		fieldName(int field)=0;
		virtual Type		fieldType(int field)=0;
		virtual Type		fieldType(const String &fieldname)=0;
		virtual AssocArray	fetchArray()=0;
		virtual int			fetchArray(AssocArray &array)=0;
		virtual Array		fetchFields()=0;
		virtual int			fetchFields(Array &array)=0;
		virtual void		printResult() const=0;
		virtual ppluint64	currentRow()=0;
		virtual bool		next()=0;

};

ppluint64 loadResultSet(std::list<AssocArray> &list, ResultSet &res);


#ifdef DONE


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
		Result	*queryf(const char *query, ...);
		void	freeResult(Result *res);

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

		String	escape(const String &str);

		virtual int		connect(const AssocArray &params);
		virtual int     connectCreate(const AssocArray &params);
		virtual int		reconnect();
		virtual int		disconnect();
		virtual int		selectDB(const String &databasename);
		virtual int 	exec(const String &query);
		virtual Result	*query(const String &query);
		virtual void	setMaxRows(ppluint64 rows);
		virtual int     ping();
		virtual ppluint64	getInsertID();	// Returns ID from autoincrement field
		virtual pplint64	getAffectedRows();
		virtual int		startTransaction();
		virtual int		endTransaction();
		virtual int		cancelTransaction();
		virtual int		cancelTransactionComplete();
		virtual int		createDatabase(const String &name);
		virtual String	databaseType() const;

};

#endif


} // EOF namespace db
} // EOF namespace ppl7

#endif /* PPL7INET_H_ */
