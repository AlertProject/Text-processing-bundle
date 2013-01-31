/*
 * qminer_sqlite.cpp
 *
 *  Created on: Nov 11, 2011
 *      Author: tadej
 */

#include "qminer_sqlite.h"

TSQLiteStoreIter::TSQLiteStoreIter(PSQLCommand C) :
		Cmd(C) {
}
bool TSQLiteStoreIter::Next() {
	return Cmd->ReadNext();
}
uint64 TSQLiteStoreIter::GetRecId() const {
	return Cmd->GetUInt64(0);
}

TSQLiteStore::TSQLiteStore(uchar StoreId, PSQLConnection C, const TStr& Table,
		const TStr& Primary, const TStr& Name, const TOgFieldDescV& Fields) :
		TOgStore(StoreId, Table), Conn(C), TableNm(Table), PrimaryKeyField(
				Primary), NmField(Name) {
	for (int i = 0; i < Fields.Len(); ++i) {
		AddFieldDesc(Fields[i]);
	}

	SelectById = TStr::Fmt("SELECT %s FROM %s WHERE %s = ?", NmField.CStr(),
			TableNm.CStr(), PrimaryKeyField.CStr());
	SelectByNm = TStr::Fmt("SELECT %s FROM %s WHERE %s = ?",
			PrimaryKeyField.CStr(), TableNm.CStr(), NmField.CStr());
	CountAll = TStr::Fmt("SELECT COUNT(%s) FROM %s", PrimaryKeyField.CStr(),
			TableNm.CStr());
	SelectAll = TStr::Fmt("SELECT %s FROM %s", PrimaryKeyField.CStr(),
			TableNm.CStr());
}

PSQLCommand TSQLiteStore::ExecuteScalar(const uint64& RecId,
		const TStr &FieldNm) const {
	PSQLParameter Par = new TSQLUInt64Parameter(RecId);
	TChA QChA;
	QChA += "SELECT ";
	QChA += FieldNm;
	QChA += " FROM ";
	QChA += TableNm;
	QChA += " WHERE ";
	QChA += PrimaryKeyField;
	QChA += " = ?";
	PSQLCommand Q = TSQLCommand::New(Conn, QChA,
			TVec < PSQLParameter > ::GetV(Par));
	Q->ExecuteQuery();
	Q->ReadNext();
	return Q;
}

bool TSQLiteStore::IsRecId(const uint64& RecId) const {
	PSQLParameter Par = new TSQLUInt64Parameter(RecId);
	PSQLCommand CountAllIds = TSQLCommand::New(Conn, SelectById,
			TVec < PSQLParameter > ::GetV(Par));
	CountAllIds->ExecuteQuery();
	return CountAllIds->ReadNext();
}

bool TSQLiteStore::IsRecNm(const TStr& RecNm) const {
	PSQLParameter Par = new TSQLStrParameter(RecNm);
	PSQLCommand CountAllIds = TSQLCommand::New(Conn, SelectByNm,
			TVec < PSQLParameter > ::GetV(Par));
	CountAllIds->ExecuteQuery();
	return CountAllIds->ReadNext();
}
TStr TSQLiteStore::GetRecNm(const uint64& RecId) const {
	PSQLParameter Par = new TSQLUInt64Parameter(RecId);
	PSQLCommand CountAllIds = TSQLCommand::New(Conn, SelectById,
			TVec < PSQLParameter > ::GetV(Par));
	CountAllIds->ExecuteQuery();
	CountAllIds->ReadNext();
	return CountAllIds->GetText(0);
}
uint64 TSQLiteStore::GetRecId(const TStr& RecNm) const {
	PSQLParameter Par = new TSQLStrParameter(RecNm);
	PSQLCommand CountAllIds = TSQLCommand::New(Conn, SelectByNm,
			TVec < PSQLParameter > ::GetV(Par));
	CountAllIds->ExecuteQuery();
	CountAllIds->ReadNext();
	return CountAllIds->GetUInt64(0);
}
uint64 TSQLiteStore::GetRecs() const {
	PSQLCommand CountAllIds = TSQLCommand::New(Conn, CountAll);
	CountAllIds->ExecuteQuery();
	CountAllIds->ReadNext();
	return CountAllIds->GetUInt64(0);
}
POgStoreIter TSQLiteStore::GetIter() const {
	PSQLCommand SelectAllIds = TSQLCommand::New(Conn, SelectAll);
	SelectAllIds->ExecuteQuery();
	return new TSQLiteStoreIter(SelectAllIds);
}

int TSQLiteStore::GetFieldInt(const uint64& RecId, const int& FieldId) const {
	return ExecuteScalar(RecId, GetFieldNm(FieldId))->GetInt(0);
}
uint64 TSQLiteStore::GetFieldUInt64(const uint64& RecId,
		const int& FieldId) const {
	return ExecuteScalar(RecId, GetFieldNm(FieldId))->GetUInt64(0);
}
TStr TSQLiteStore::GetFieldStr(const uint64& RecId, const int& FieldId) const {
	return ExecuteScalar(RecId, GetFieldNm(FieldId))->GetText(0);
}
void TSQLiteStore::GetFieldStrV(const uint64& RecId, const int& FieldId,
		TStrV& StrV) const {
	FieldError(FieldId, "StrV");
}
bool TSQLiteStore::GetFieldBool(const uint64& RecId, const int& FieldId) const {
	return ExecuteScalar(RecId, GetFieldNm(FieldId))->GetInt(0) != 0;
}
double TSQLiteStore::GetFieldFlt(const uint64& RecId,
		const int& FieldId) const {
	return ExecuteScalar(RecId, GetFieldNm(FieldId))->GetFloat(0);
}
TFltPr TSQLiteStore::GetFieldFltPr(const uint64& RecId,
		const int& FieldId) const {
	FieldError(FieldId, "FltPr");
	return TFltPr();
}
void TSQLiteStore::GetFieldTm(const uint64& RecId, const int& FieldId,
		TTm& Tm) const {
	uint64 Msec = GetFieldUInt64(RecId, FieldId);
	Tm = TTm::GetTmFromMSecs(Msec);
}
