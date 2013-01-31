/*
 * qminer_sqlite.h
 *
 * QMiner store, backed by SQLite database.
 * Model:
 *  - single table (TableNm)
 *  - primary key (PrimaryKeyField)
 *  - visual record identifed - name (NmField)
 *
 * No joins. To support them, do a "CREATE VIEW" within the db.
 * If multiple OgStores are used using the same database, sharing the PSQLConnection is recommended.
 *
 *  Created on: Nov 11, 2011
 *      Author: tadej
 */

#ifndef QMINER_SQLITE_H_
#define QMINER_SQLITE_H_



#include "../qminer.h"
#include <sqlitedb.h>

/**
 * Iterator over query results.
 */
class TSQLiteStoreIter : public TOgStoreIter {
protected:
	PSQLCommand Cmd;

public:
	TSQLiteStoreIter(PSQLCommand C);
	bool Next();
	uint64 GetRecId() const;
};


/**
 * OgStore, backed by an SQLite database table.
 */
class TSQLiteStore: public TOgStore {

protected:
	PSQLConnection Conn;
	TStr TableNm;
	TStr PrimaryKeyField;
	TStr NmField;

	TStr SelectById, SelectByNm, CountAll, SelectAll;

protected:
	PSQLCommand ExecuteScalar(const uint64& RecId, const TStr &FieldNm) const;
public:

	TSQLiteStore(uchar StoreId, PSQLConnection C, const TStr& Table, const TStr& Primary, const TStr& Name, const TOgFieldDescV& Fields);

	bool IsRecId(const uint64& RecId) const;
	bool IsRecNm(const TStr& RecNm) const;
	TStr GetRecNm(const uint64& RecId) const;
	uint64 GetRecId(const TStr& RecNm) const;
	uint64 GetRecs() const;
	POgStoreIter GetIter() const;
    int GetFieldInt(const uint64& RecId, const int& FieldId) const;
    uint64 GetFieldUInt64(const uint64& RecId, const int& FieldId) const;
	TStr GetFieldStr(const uint64& RecId, const int& FieldId) const;
	void GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const;
	bool GetFieldBool(const uint64& RecId, const int& FieldId) const;
    double GetFieldFlt(const uint64& RecId, const int& FieldId) const;
    TFltPr GetFieldFltPr(const uint64& RecId, const int& FieldId) const;
    void GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const;

};

#endif /* QMINER_SQLITE_H_ */
