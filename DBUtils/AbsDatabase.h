#ifndef __ABS_DATABASE_H__
#define __ABS_DATABASE_H__
#pragma once

#ifndef STDSTRING_H
    #include "Collections/StdString.h"
#endif

#ifndef __DATABASE_TYPE_H__
	#include "DatabaseType.h"
#endif

#include "unordered_map"

enum dsFieldType
{
    dsFieldType_Undefined = -1,
    dsFieldType_Text = 0,
    dsFieldType_Long,
    dsFieldType_Double,
    dsFieldType_DateTime,
    dsFieldType_Binary
};

class dsTableFieldInfo : public std::unordered_map<CStdString, dsFieldType, std::hash<std::basic_string<TCHAR> > > { };

class CAbsRecordset;

class CAbsDatabase
{
// Construction/Destruction
public:
	CAbsDatabase() { }
	virtual ~CAbsDatabase() { }

// Overrides
public:
	virtual bool BeginTrans()  = 0; 
	virtual bool CommitTrans() = 0; 
	virtual bool Rollback()    = 0; 

	virtual bool Execute(LPCTSTR lpszSQL) = 0; 

    virtual void Close() = 0;
    virtual bool OpenDB(LPCTSTR sPath, bool bReadOnly, LPCTSTR szPsw) = 0;
    
	virtual dsDBType GetType() = 0;

	virtual bool IsReadOnly() const = 0;
	virtual bool IsOpen()     const = 0;

	virtual CStdString GetName() = 0;

	virtual bool DoesTableExist(LPCTSTR sTable) = 0;

	virtual CAbsRecordset *CreateRecordset() = 0;

    virtual void CommitDatabase() = 0;

    virtual bool CompactDatabase() = 0;

	virtual void DeleteRelation(LPCTSTR sRelation) = 0;
	virtual bool CreateRelation(LPCTSTR sName, LPCTSTR sTable, LPCTSTR sForeignTable, long lAttr,
								LPCTSTR sField, LPCTSTR sForeignField) = 0;

	virtual void CreateIndex(LPCTSTR sTable, LPCTSTR sIndex) = 0;

    virtual bool GetTableFieldInfo(LPCTSTR sTable, dsTableFieldInfo &info) = 0;

    typedef void (*dbErrorHandler)(LPCTSTR msg); 
    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) = 0;
};

namespace ds_table_field_info_util
{
    inline void fields_union(dsTableFieldInfo &union_info, const dsTableFieldInfo &src_info, const dsTableFieldInfo &dst_info)
    {
        auto end_it = src_info.end();
        for (auto it = src_info.begin(); it != end_it; ++it) 
        {
            if ( dst_info.find(it->first.c_str()) == dst_info.end() ) {
                continue;
            }
            union_info[it->first] = it->second;
        }
    }    

    inline bool fields_union(dsTableFieldInfo &union_info, CAbsDatabase *pSrcDB, LPCTSTR sTableNameSrc, CAbsDatabase *pDstDB, LPCTSTR sTableNameDst) 
    {
        dsTableFieldInfo dst_info;
        if ( !pDstDB->GetTableFieldInfo(sTableNameDst, dst_info) ) {
            return false;
        }

        dsTableFieldInfo src_info;
        if ( !pSrcDB->GetTableFieldInfo(sTableNameSrc, src_info) ) {
            return false;
        }

        ds_table_field_info_util::fields_union(union_info, src_info, dst_info);
        return true;
    }
};

#endif