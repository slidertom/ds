#ifndef __ABS_DATABASE_H__
#define __ABS_DATABASE_H__
#pragma once

#ifndef __DS_TYPES_H__
    #include "dsTypes.h"
#endif

#ifndef __DS_TABLE_FIELD_INFO_H__
    #include "dsTableFieldInfo.h"
#endif

class CAbsRecordset;
class dsOpenParams;

class CAbsDatabase
{
// Construction/Destruction
public:
    CAbsDatabase() { }
    virtual ~CAbsDatabase() { }

// Overrides
public:
    virtual bool BeginTrans()                       = 0; 
    virtual bool CommitTrans()                      = 0; 
    virtual bool Rollback()                         = 0; 
    virtual bool Backup(const char *sBackupFile)    = 0;

    virtual bool Execute(const wchar_t *sSQL) = 0; 
    virtual bool Execute(const char *sSQL)    = 0; 

    virtual bool OpenDB(const wchar_t *sPath, const dsOpenParams &open_params) = 0;
        
    virtual dsDBType GetType() = 0;

    virtual bool IsReadOnly() const = 0;
    virtual bool IsOpen()     const = 0;

    virtual std::wstring GetName() = 0;
    // do leave possibility for the realization to use the best encoding system
    virtual bool DoesTableExist(const wchar_t *sTable) = 0;
    virtual bool DoesTableExistUTF8(const char *sTable) = 0;

    virtual CAbsRecordset *CreateRecordset() = 0;

    virtual void CommitDatabase() = 0;

    virtual bool CompactDatabase() = 0;

    virtual void DeleteRelation(const wchar_t *sRelation) = 0;
    virtual bool CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, int32_t lAttr,
                                const wchar_t *sField, const wchar_t *sForeignField) = 0;

    virtual bool GetTableFieldInfo(const char *sTable, dsTableFieldInfo &info) = 0;

    typedef void (*dbErrorHandler)(const wchar_t *msg); 
    virtual dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) = 0;

    virtual std::vector<std::string> GetTableList() = 0;

    virtual bool DropColumn(const wchar_t *sTableName, const wchar_t *sColumnName) = 0;
    virtual bool RemoveColumnCollateNoCase(const wchar_t *sTableName, const wchar_t *sColumnName) = 0;
    virtual bool DropTable(const wchar_t *sTableName) = 0;
    virtual bool DropTrigger(const wchar_t *sTriggerName) = 0;
    virtual bool DropIndex(const wchar_t *sIndexName) = 0;

    virtual bool CreateTable(const char *sTableName, const dsTableFieldInfo &info) = 0;
    virtual bool CreateTables(const std::vector<std::pair<std::string, dsTableFieldInfo>> &tables_info) = 0;

    virtual bool CreateDB(const wchar_t *sPath) = 0;
};

namespace ds_table_field_info_util
{
    inline void fields_union(dsTableFieldInfo &union_info, const dsTableFieldInfo &src_info, const dsTableFieldInfo &dst_info)
    {
        for (const auto &src_elem : src_info) {
            const auto dst_found = dst_info.find(src_elem.first.c_str());
            if ( dst_found == dst_info.end() ) {
                continue;
            }
            union_info[src_elem.first] = dst_found->second;
        }
    }    

    inline bool fields_union(dsTableFieldInfo &union_info, CAbsDatabase *pSrcDB,
                             const char *sTableNameSrc, CAbsDatabase *pDstDB, const char *sTableNameDst) 
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