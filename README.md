#### ds - Data Access System
#####Sqlite (read-write), Ms DAO mdb (read-write), Ms SQL (read-only),
databases access system. 

DS is an easy to use C++ [**SQLite3**](https://sqlite.org/), **DAO**, **Ms SQL** C++ based wrapper.

Provides a **Recordset** (MFC: [CDaoRecordSet](https://msdn.microsoft.com/en-us/library/8wht5w3w.aspx), [CRecordset](https://msdn.microsoft.com/en-us/library/92bcy0kw.aspx)) like  interface for a SQLite3, MsSQL, DAO databases.

You can access sqlite database in the same way as mdb file with the minimal code changes inside your project.
 
Text, Long/Integer, Blob, Double/Real field types are supported.

Text field type addition: [**Json**](https://en.wikipedia.org/wiki/JSON) format support. [RapidJson](https://github.com/miloyip/rapidjson) used as backend.
(PicoJson, sajson also can be used as backends. RapidJson used as the fastest one.)

If you're a [**NoSQL**](https://en.wikipedia.org/wiki/NoSQL) type of person and SQL based database usage can not be avoided, then this might fit the bill perfectly.

DS is an exception free code.

#####Samples:
```C++
dsDatabase db;
db.OpenDB(_T("database.sqlite")); // sqlite database 
//db.OpenDB(_T("database.mdb")); // MS mdb database 

dsTable loader(&db, _T("Table_Name");
loader.AddNew();
    // after AddNew you can always retrieved new record key
    int nId = loader.GetFieldLong(_T("ID")); 
    loader.SetFieldString(_T("Code"), _T("My_Code"));
loader.Update();

loader.SeekIndex(_T("ID"), nId);
loader.GetFieldString(_T("Code"));
loader.Edit();
    loader.SetFieldString(_T("Code"), _T("New_Code"));
loader.Update();
```

```C++
class CCodeDescrLoader : public dsTable
{
public:
	CCodeDescrLoader(dsDatabase *pDatabase, LPCTSTR sTableName)
	: dsTable(pDatabase, sTableName) { }
	virtual ~CCodeDescrLoader() { }

public:
    KEY_LONG(Id,   _T("ID"));
    KEY_TEXT(Code, _T("CODE"));
    FIELD_TEXT(Descr, _T("DESCRIPTION"));
};

CCodeDescrLoader loader(&db, _T("Table1"));
loader.Flush(); // delete all recored from the Table1
loader.AddNew();
   int nNewId = loader.GetId();
   loader.SetCode(_T("New"));
   loader.SetDescr(_T("Descr"));
loader.Update();

loader.DeleteById(nNewId); // do delete one record ID=nNewId
// do delete all records from the Table1 with the code value: New
loader.DeleteAllByCode(_T("New")); 

// do iterate all Table1 records and do retrieve code values
if ( loader.MoveFirst() )
{
    while ( !loader.IsEOF() ) )
    {
        CStdString sCode = loader.GetCode(); 
        loader.MoveNext();
    }
}

```
```C++
void OnDatabaseError(LPCTSTR sError) 
{
    TRACE(sError); TRACE(_T("\n"));
}

dsDatabase db;
db.OpenDB(_T("database.mdb")); 
// Exception free code, do handle all errors through the defined function
db.SetErrorHandler(OnDatabaseError); 
```
