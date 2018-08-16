/*******************************************************************************
 * This file is part of "Patrick's Programming Library", Version 6 (PPL6).
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

#ifndef PPL6_DB_H_
#define PPL6_DB_H_

#ifndef _PPL6_INCLUDE
    #ifdef PPL6LIB
        #include "ppl6.h"
    #else
        #include <ppl6.h>
    #endif
#endif

#include <queue>

namespace ppl6 {

namespace db {

class Pool;
class PoolEx;

class Result
{
	public:
		enum Type {
			Error=-1,
			Unknown=0,
			Integer,
			Decimal,
			Bit,
			Timestamp,
			Date,
			Time,
			DateTime,
			String,
			Binary,
			Enum
		};

		Result();
		virtual ~Result();

		int Export(CAssocArray &array);

		virtual	void Clear();
		virtual pplint64 Rows();
		virtual pplint64 Affected();
		virtual int Fields();
		virtual const char *Get(pplint64 row, const char *fieldname);
		virtual const char *Get(pplint64 row, int field);
		virtual int FieldNum(const char *fieldname);
		virtual const char *FieldName(int field);
		virtual Result::Type	FieldType(int field);
		virtual Result::Type	FieldType(const char *fieldname);
		virtual CAssocArray  FetchArray(pplint64 row=-1);
		virtual int		FetchArray(CAssocArray &array, pplint64 row=-1);
		virtual CArray  FetchFields(pplint64 row=-1);
		virtual int		FetchFields(CArray &array, pplint64 row=-1);
		virtual int		Seek(pplint64 row);
		virtual void    PrintResult();

};

class GenericResult : public Result
{
	private:
		pplint64	rows;			//!\brief Anzahl Zeilen im Ergebnis
		pplint64 	lastinsertid;	//!\brief Falls es sich um einen Insert mit einem Autoincrement-Index handelte, steht hier die vergebene ID
		pplint64	affectedrows;	//!\brief Falls es sich um ein Update/Insert/Replace handelte, steht hier die Anzahl betroffender Datensätze
		int			num_fields;		//!\brief Anzahl Spalten im Ergebnis
		CMemMan		*mem;
		void *fieldnames;
		void *firstrow;
		void *lastrow;
		void *index;
		void *row;
		pplint64 rowpointer;
		pplint64 res_lastrow;
		CAssocArray *res_lastfetch;
		CArray *res_lastfieldfetch;
		int ToString(CString &res, void *field, Result::Type type);


	public:
		GenericResult();
		~GenericResult();
		int			Import(CAssocArray &array);
		void		SetAffectedRows(pplint64 rows);
		int			SetNumFields(int num);
		int			SetFieldName(int num, const char* name, Result::Type type);
		int			SetFieldName(int num, const char* name, int namelength, Result::Type type);
		int			NewRow();
		int			StoreField(int num, void *data, int size);
		int			BuildIndex();
		void		*Malloc(size_t size);
		void		*Calloc(size_t size);
		void		Free(void *ptr);
		ppluint64	GetUsedMem();
		void		SetBufferGrowSize(size_t bytes);

		virtual	void Clear();
		virtual pplint64 Rows();
		virtual pplint64 Affected();
		virtual int Fields();
		virtual const char *Get(pplint64 row, const char *fieldname);
		virtual const char *Get(pplint64 row, int field);
		virtual int FieldNum(const char *fieldname);
		virtual const char *FieldName(int field);
		virtual Result::Type	FieldType(int field);
		virtual Result::Type	FieldType(const char *fieldname);
		virtual CAssocArray  FetchArray(pplint64 row=-1);
		virtual int		FetchArray(CAssocArray &array, pplint64 row=-1);
		virtual CArray  FetchFields(pplint64 row=-1);
		virtual int		FetchFields(CArray &array, pplint64 row=-1);
		virtual int		Seek(pplint64 row);
		virtual void    PrintResult();

};

class QueryLogEntry
{
public:
	QueryLogEntry(const char *query, float duration);
	CDateTime	timestamp;
	float 		duration;
	CString		query;
};

class Database
{
	friend class Pool;
	friend class PoolEx;

	private:
		CAssocArray	ConnectParam;
		ppluint64	lastuse;
		ppluint64	lastping;
		CString		Hash;		// Wird vom Pool verwendet
		CLog		*Log;
		Pool		*pool;
		PoolEx		*poolex;
		bool		poollock;

		std::queue<QueryLogEntry> QueryLog;

	protected:
		void    LogQuery(const char *query, float duration);
		void	UpdateLastPing();
		void	UpdateLastUse();
		void	ClearLastUse();

	public:
		Database();
		virtual ~Database();

		void	SetLogfile(CLog *log=NULL);
		void	SetLogfile(CLog &log);
		CLog	*GetLogfile();

		void	SetParam(const char *name, const char *value);
		void	SetParam(const char *name, int value);
		int		Connect();
		int		Close();

		int		Execf(const char *query, ...);
		Result	*Queryf(const char *query, ...);
		void	FreeResult(Result *res);

		std::queue<QueryLogEntry> &getQueryLog();
		void clearQuerylog();

		CAssocArray ExecArrayf(const char *query, ...);
		CAssocArray ExecArray(const CString &query);
		CAssocArray ExecArrayAllf(const char *query, ...);
		CAssocArray ExecArrayAll(const CString &query);
		int ExecArrayf(CAssocArray &result, const char *query, ...);
		int ExecArray(CAssocArray &result, const CString &query);
		int ExecArrayAllf(CAssocArray &result, const char *query, ...);
		int ExecArrayAll(CAssocArray &result, const CString &query);
		int Save(const char *method, const char *table, CAssocArray &a, const char *clause=NULL, const CAssocArray &exclude=CAssocArray(), const CAssocArray &types=CAssocArray());
		int SaveGenQuery(CString &Query, const char *method, const char *table, CAssocArray &a, const char *clause=NULL, const CAssocArray &exclude=CAssocArray(), const CAssocArray &types=CAssocArray());
		int	ReadKeyValue(CAssocArray &res, const char *query, const char *keyname, const char *valname=NULL);
		ppluint64 InsertKey(const char *table, CAssocArray &a, const char *keyname, const CAssocArray &exclude=CAssocArray());

		CString	GetEscaped(const CString &str);

		virtual int		Connect(const CAssocArray &params);
		virtual int     ConnectCreate(const CAssocArray &params);
		virtual int		Reconnect();
		virtual int		Disconnect();
		virtual int		SelectDB(const char *databasename);
		virtual int 	Exec(const CString &query);
		virtual Result	*Query(const CString &query);
		virtual void	SetMaxRows(ppluint64 rows);
		virtual int     Ping();
		virtual int		Escape(CString &str) const;
		virtual ppluint64	GetInsertID();	// Returns ID from autoincrement field
		virtual pplint64	GetAffectedRows();
		virtual int		StartTransaction();
		virtual int		EndTransaction();
		virtual int		CancelTransaction();
		virtual int		CancelTransactionComplete();
		virtual int		CreateDatabase(const char *name);
		virtual CString	databaseType() const;
		virtual CString getQuoted(const CString &value, const CString &type=CString()) const;

};

class MySQL : public Database
{
	private:
		void    *mysql;
		ppluint64 	lastinsertid;
		pplint64	affectedrows;
		ppluint64	maxrows;
		int			transactiondepth;
		CAssocArray condata;

		int	Mysql_Query(const CString &query);

	public:
		MySQL();
		virtual ~MySQL();

		static int LibraryInit(int argc=0, char **argv=NULL, char **groups=NULL);


		virtual int		Connect(const CAssocArray &params);
		virtual int		Reconnect();
		virtual int		Disconnect();
		virtual int		SelectDB(const char *databasename);
		virtual int 	Exec(const CString &query);
		virtual Result	*Query(const CString &query);
		virtual void	SetMaxRows(ppluint64 rows);
		virtual int     Ping();
		virtual int		Escape(CString &str) const;
		virtual ppluint64	GetInsertID();
		virtual pplint64	GetAffectedRows();
		virtual int		StartTransaction();
		virtual int		EndTransaction();
		virtual int		CancelTransaction();
		virtual int		CancelTransactionComplete();
		virtual int		CreateDatabase(const char *name);
		virtual CString	databaseType() const;
};

class Sybase : public ppl6::db::Database
{
	private:
		CAssocArray condata;
		void		*conn;
		ppluint64	lastinsertid;
		ppluint64	maxrows;
		pplint64	rows_affected;
		int			transactiondepth;
		bool		connected;


	public:
		Sybase();
		virtual ~Sybase();
		int		lastServerMsgNumber;
		int		lastClientMsgNumber;

		CString	syberror;
		CString	syberrorLong;

		virtual int		Connect(const CAssocArray &params);
		virtual int		Reconnect();
		virtual int		Disconnect();
		virtual int		SelectDB(const char *databasename);
		virtual int 	Exec(const CString &query);
		virtual ppl6::db::Result	*Query(const CString &query);
		virtual void	SetMaxRows(ppluint64 rows);
		virtual int     Ping();
		virtual int		Escape(CString &str) const;
		virtual ppluint64	GetInsertID();
		virtual pplint64	GetAffectedRows();
		virtual int		StartTransaction();
		virtual int		EndTransaction();
		virtual int		CancelTransaction();
		virtual int		CancelTransactionComplete();
		virtual int		CreateDatabase(const char *name);
		virtual CString	databaseType() const;

		static int	GetMaxConnects();
		static int	SetMaxConnects(int max);
		static int SetVersion(int version);
		static int SetLocale(const char *locale, const char *dateformat);
		static void SetResultBufferGrowSize(size_t bytes);
};


class Postgres : public Database
{
	private:
		void 	   *conn;
		ppluint64 	lastinsertid;
		pplint64	affectedrows;
		ppluint64	maxrows;
		int			transactiondepth;
		CAssocArray condata;

		void *Pgsql_Query(const CString &query);

	public:
		Postgres();
		virtual ~Postgres();

		virtual int		Connect(const CAssocArray &params);
		virtual int     ConnectCreate(const CAssocArray &params);
		virtual int		Reconnect();
		virtual int		Disconnect();
		virtual int		SelectDB(const char *databasename);
		virtual int 	Exec(const CString &query);
		virtual Result	*Query(const CString &query);
		virtual void	SetMaxRows(ppluint64 rows);
		virtual int     Ping();
		virtual int		Escape(CString &str) const;
		virtual ppluint64	GetInsertID();
		virtual pplint64	GetAffectedRows();
		virtual int		StartTransaction();
		virtual int		EndTransaction();
		virtual int		CancelTransaction();
		virtual int		CancelTransactionComplete();
		virtual int		CreateDatabase(const char *name);
		virtual CString	databaseType() const;
		virtual CString getQuoted(const CString &value, const CString &type=CString()) const;
};

class SQLite : public Database
{
	private:
		void    *sql;
		ppluint64 	lastinsertid;
		pplint64	affectedrows;
		ppluint64	maxrows;
		int			transactiondepth;
		CAssocArray condata;
		CMutex		mutex;

		int	Mysql_Query(const CString &query);

	public:
		SQLite();
		virtual ~SQLite();

		virtual int		Connect(const CAssocArray &params);
		virtual int		Reconnect();
		virtual int		Disconnect();
		virtual int     Ping();
		virtual int		SelectDB(const char *databasename);
		virtual int		CreateDatabase(const char *name);
		virtual int 	Exec(const CString &query);
		virtual Result	*Query(const CString &query);
		virtual ppluint64	GetInsertID();
		virtual pplint64	GetAffectedRows();
		virtual int		StartTransaction();
		virtual int		EndTransaction();
		virtual int		CancelTransaction();
		virtual int		CancelTransactionComplete();
		virtual void	SetMaxRows(ppluint64 rows);
		virtual int		Escape(CString &str) const;
		virtual CString	databaseType() const;
};

Database *Connect(const CAssocArray &params);
void GetSupportedDatabases(CAssocArray &a);

class Pool
{
	friend class PoolEx;
	friend class PoolTree;
	private:
		CMutex			Mutex;
		CGenericList	Used, Free;
		CAssocArray		ConnectParam;
		CString			Name;
		CString			Hash;
		CLog			*Log;
		int				Id;
		int				Min, Max;
		int				MinSpare, MaxSpare;
		int				Grow;
		int				Timeout;
		int				KeepAlive;
		bool			IsInit;
		double			LastCheck;
		Database		*New();
		int CalcHash(CString &hash, CAssocArray &param);

		void checkUsedPool();

	public:
		Pool();
		~Pool();

		int SetConnectParams(CAssocArray &connect);
		int SetOptions(CAssocArray &options);
		int SetOption(const CString &Name, const CString &Value);
		int SetName(const CString &Name);

		Database *Get(bool wait=false, int ms=0);
		int Release(Database *db);
		int Destroy(Database *db);
		void CheckPool();
		int ClearUsedPool(bool destroy=false);
		int ClearFreePool(bool destroy=false);
		int GetStatus(CAssocArray &status);
		void SetLogfile(CLog *log);
};

class PoolTree : public CAVLTree
{
	public:
		virtual int Compare (const void *value1, const void *value2) const;
		virtual int DestroyValue (void *item) const;
		virtual int GetValue (const void *item, CString &buffer) const;
};

class PoolEx
{
	private:
		CMutex		Mutex;
		PoolTree	Pools;
		CLog		*Log;
		CThread		*CheckThread;
		int			DefaultTimeout;
		int			DefaultKeepAlive;
		int			DefaultMax;
		int			DefaultMin;
		int			DefaultMinSpare;
		int			DefaultMaxSpare;
		int			DefaultGrow;

		Pool		*FindPoolByName(const char *name);
	public:
		PoolEx();
		~PoolEx();

		void ClearPools(bool free=true, bool used=false);
		int SetDefaultOption(const CString &Name, const CString &Value);
		int SetDefaultOptions(CAssocArray &options);
		int SetDefaultTimeout(int seconds);
		int SetDefaultKeepAlive(int seconds);
		int SetDefaultMin(int nr);
		int SetDefaultMax(int nr);
		int SetDefaultMinSpare(int nr);
		int SetDefaultMaxSpare(int nr);
		int SetDefaultGrow(int nr);


		int CreatePool(int id, const char *name, CAssocArray &connect);
		int CreatePool(int id, const char *name, CAssocArray &connect, CAssocArray &options);
		int DeletePool(int id);
		int SetPoolOption(int id, const CString &Name, const CString &Value);
		int SetPoolOptions(int id, CAssocArray &options);

		Database *Get(const char *name, bool wait=false, int ms=0);
		Database *Get(int id=0, bool wait=false, int ms=0);
		int Release(Database *db);
		int Destroy(Database *db);

		Pool *GetPool(int id);
		Pool *GetPool(const char *name);


		void CheckPool();
		int GetStatus(CAssocArray &status);
		void SetLogfile(CLog *log);

		int StartPoolCheck(int intervall=10000);
		int StopPoolCheck();
};


}	// EOF namespace db
}	// EOF namespace ppl6

#endif	// PPL6_DB_H_
