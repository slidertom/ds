#ifndef __DS_DATABASE_H__
#define __DS_DATABASE_H__
#pragma once

#ifndef __DB_UTILS_IMPL_H__
    #include "DBUtilsImpl.h"
#endif

#ifndef __ROLES_H__
    #include "DesignPatterns/Roles.h"
#endif

#ifndef __DS_DATABASE_LISTENER_H__
    #include "dsDatabaseListener.h"
#endif

#ifndef __DS_TYPES_H__
    #include "dsTypes.h"
#endif

#ifndef __DS_OPEN_PARAMS_H__
    #include "dsOpenParams.h"
#endif

#ifndef FASTDELEGATE_H
	#include "MemFunc/FastDelegate.h"
#endif

#include "vector"

class CAbsDatabase;

// dsDatabase should not throw any exceptions
// if any error info is required, should be created separate interface:
// global or local function to handle error messages and/or throw app specific exceptions

// CRoleCore used to connect any object to the database (e.g.: data cache based objects as roles)

class DB_UTILS_API dsDatabase : public CRoleCore
{
    typedef fastdelegate::FastDelegate0<> FuncPostCommitTrans;

// Constuction/Destruction
public:
    dsDatabase();
    virtual ~dsDatabase();
    
// Operations
public:
    void RegisterListener(dsDatabaseListener *pListener) noexcept;
    void UnregisterListener(dsDatabaseListener *pListener) noexcept;

// Status functions
public:
    bool IsReadOnly() const noexcept;
    bool IsOpen() const noexcept;
    std::wstring GetName() const noexcept;
    
// Operators
public:
    dsDBType GetType() noexcept;
    
// Operations
public:
    void BeginTrans() noexcept;  
    void CommitTrans() noexcept; 
    void RollbackTrans() noexcept;    

    bool Execute(const wchar_t *sSQL) noexcept; 
    
    void Close() noexcept;

    // sKey - Database encryption key.
    bool OpenDB(const wchar_t *sPath, const dsOpenParams &params = dsOpenParams()) noexcept;

    // Do call this function if you want to commit all the data from the memory to file
    // possibly => everything works with out this function call.
    // This is DAO specific -> DAO fails to save data to file.
    // This is SqLite specific if m_bMemory flag in use -> commits memory database into file
    void CommitDatabase() noexcept; 

    typedef void (*dbErrorHandler)(const wchar_t *msg); 
    dbErrorHandler SetErrorHandler(dbErrorHandler newHandler) noexcept;
    dbErrorHandler GetErrorHandler();

    bool CompactDatabase() noexcept; // or VACUUM
    static bool CompactDatabase(const wchar_t *sPath) noexcept;
    static bool CompactDatabase(const wchar_t *sPath, dbErrorHandler newHandler) noexcept;
    static void SetLogPath(const wchar_t *sLogPath) noexcept;

    static bool IsSqLiteDB(const wchar_t *sPath) noexcept;
    static bool IsDaoDB(const wchar_t *sPath) noexcept;
    static bool IsMSSQLServerAdoDotNet(const wchar_t *sPath) noexcept;

    bool DropIndex(const wchar_t *sIndexName);
    std::wstring AddUniqueIndexNoCase(const wchar_t *sTableName, const wchar_t *sFieldName);

    void SetPostCommitHandler(const FuncPostCommitTrans &func);

// Operations
public:
    void Refresh() noexcept; // Refresh database related cache items

public:
    enum
    {
        eDBRelUpdateCascade    = 0x100,
        eDBRelDeleteCascade    = 0x1000
    };
        
// Database structure help functions
public:
    bool DoesTableExist(const wchar_t *sTable) const noexcept; 
    bool DoesTableExist(const char *sTable) const noexcept; 

    void DeleteRelation(const wchar_t *sRelation) noexcept; 
    bool CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, int32_t lAttr,
                        const wchar_t *sField, const wchar_t *sForeignField) noexcept;

// Attributes
private:
    friend class dsTable;
    friend class dsCopyTableData;
    CAbsDatabase *m_pDatabase {nullptr};
    std::vector<dsDatabaseListener *> m_listeners;
    dbErrorHandler m_pErrorHandler;
    FuncPostCommitTrans m_postCommitTrans;
};

#endif