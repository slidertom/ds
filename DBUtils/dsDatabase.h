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

#ifndef __DATABASE_TYPE_H__
	#include "DatabaseType.h"
#endif

#include "vector"

class CAbsDatabase;

// dsDatabase should not throw any exceptions
// if any error info is required, should be created separate interface:
// global or local function to handle error messages and/or throw app specific exceptions

// CRoleCore used to connect any object to the database (e.g.: cache objects)
class DB_UTILS_API dsDatabase : public CRoleCore
{
// Constuction/Destruction
public:
    dsDatabase();
	virtual ~dsDatabase();
	
// Operations
public:
    void RegisterListener(dsDatabaseListener *pListner);
    void UnregisterListener(dsDatabaseListener *pListner);

// Status functions
public:
	bool IsReadOnly() const;
	bool IsOpen() const;
	std::wstring GetName() const;
	
// Operators
public:
	dsDBType GetType();
	
// Operations
public:
	void BeginTrans();  
	void CommitTrans(); 
	void RollbackTrans();    

	bool Execute(const wchar_t *lpszSQL); 
	
    void Close();

	class dsParams
	{
	public:
		dsParams() : m_bReadOnly(false), m_bMultiUser(false) {};
		~dsParams() {};

	public:
		bool m_bReadOnly;
		bool m_bMultiUser;
		std::wstring m_sKey;
	};

    // sKey - Database encryption key.
    bool OpenDB(const wchar_t *sPath, const dsParams &params = dsParams());

    // Do call this function if you want to commit all the data from the memory to file
    // if possible do not use this function.
    // possible => everything works with out this function call.
    // This is DAO specific -> DAO fails to save data to file.
    void CommitDatabase(); 

    bool CompactDatabase(); // or VACUUM
    static bool CompactDatabase(const wchar_t *sPath);

// Operations
public:
	void Refresh(); // Refresh database related cache items

public:
	enum
	{
		eDBRelUpdateCascade	= 0x100,
		eDBRelDeleteCascade	= 0x1000
	};
		
// Database structure help functions
public:
	bool DoesTableExist(const wchar_t *sTable) const; 
    bool DoesTableExist(const char *sTable) const; 

	void DeleteRelation(const wchar_t *sRelation); 
	bool CreateRelation(const wchar_t *sName, const wchar_t *sTable, const wchar_t *sForeignTable, long lAttr,
						const wchar_t *sField, const wchar_t *sForeignField);

    typedef void (*dbErrorHandler)(const wchar_t *msg); 
    dbErrorHandler SetErrorHandler(dbErrorHandler newHandler);

// Attributes
private:
	friend class dsTable;
    friend class dsCopyTableData;
	CAbsDatabase *m_pDatabase;
    std::vector<dsDatabaseListener *> m_listners;
    dbErrorHandler m_pErrorHandler;
};

#endif