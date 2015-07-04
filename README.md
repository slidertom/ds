#### ds - Data Access System
#####Sqlite (read-write), Ms DAO mbd (read-write), Ms SQL (read-only),
databases access system. 

Recordset (MFC: CDaoRecordSet, CRecordset), data table based interface.

Samples:
```C++
dsDatabase db;
db.OpenDB(_T("database.sqlite"));

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
// Contruction/Destruction
public:
	CCodeDescrLoader(dsDatabase *pDatabase, LPCTSTR sTableName)
	: dsTable(pDatabase, sTableName) { }
	virtual ~CIdLoader() { }

// Attributes
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
