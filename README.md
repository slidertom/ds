## ds - Data Access System

Sqlite (read-write), Ms DAO mdb (read-write), Ms SQL (read-only),
databases access system. 

DS is an easy to use C++ [**SQLite3**](https://sqlite.org/), [**DAO**](https://msdn.microsoft.com/en-us/library/aa984815(v=vs.71).aspx), [**MsSQL**](https://en.wikipedia.org/wiki/Microsoft_SQL_Server) C++ based wrapper.

Provides a **Recordset** (MFC: [CDaoRecordSet](https://msdn.microsoft.com/en-us/library/8wht5w3w.aspx), [CRecordset](https://msdn.microsoft.com/en-us/library/92bcy0kw.aspx)) like  interface for a SQLite3, MsSQL, DAO databases.

You can access sqlite database in the same way as mdb file with the minimal code changes inside your project.
 
Text, Long/Integer, Blob, Double/Real field types are supported.

Text field type addition: [**Json**](https://en.wikipedia.org/wiki/JSON) format support. [RapidJson](https://github.com/miloyip/rapidjson) used as backend.
(PicoJson, sajson also can be used as backends. RapidJson used as the fastest one.)

New backend to read json files: [simdjson](https://github.com/simdjson/simdjson): Parsing gigabytes of JSON per second.

Use namespace ds_json::read to acceess simdjson.

```C++
dsTable loader(&db, L"Table_Name");
loader.MoveFirst(); // take the first record
ds_json::read::object obj;
ds_json::str2obj(std::move(loader.GetFieldStringUTF8("json_data")), obj);

ds_json::read::array obj_array;
obj.GetArray("json_array", obj_array);
for (const ds_json::read::object &obj : obj_array) {
	obj.GetTextUtf8("json_field");
}
```

If you're a [**NoSQL**](https://en.wikipedia.org/wiki/NoSQL) type of person and SQL based database usage can not be avoided, then this might fit the bill perfectly.

DS is an **exception free** code.

Visual Studio Workspace Generation
```
cd DBUtils
cmake.exe" CMakeLists.txt
```

Samples: 
```C++
dsDatabase db;
db.OpenDB(L"database.sqlite"); // sqlite database 
//db.OpenDB(L"database.mdb"); // MS mdb database 

dsTable loader(&db, L"Table_Name");
loader.AddNew();
    // after AddNew you can always retrieved new record key
    int nId = loader.GetFieldLong(L"ID"); 
    loader.SetFieldString(L"Code", L"My_Code");
loader.Update();

loader.SeekIndex(L"ID", nId);
loader.GetFieldString(L"Code");
loader.Edit();
    loader.SetFieldString(L"Code", L"New_Code");
loader.Update();
```

```C++
class CCodeDescrLoader : public dsTable
{
public:
    CCodeDescrLoader(dsDatabase *pDatabase)
       : dsTable(pDatabase, L"MyTableName") { }
    virtual ~CCodeDescrLoader() { }

public:
    KEY_LONG(Id,      "ID");
    KEY_TEXT(Code,    "CODE");
    FIELD_TEXT(Descr, "DESCRIPTION");
    FIELD_JSON(Data,  "DATA"); // Json based field
	 JSON_TEXT(Remark, "Remark"); // Data.Remark
	 JSON_INT32(Order,  "Order"); // Data.Order
	 // NOTE: new fields can be always added, old fields can be always deleted
	 // no changes are required in the sqlite or mdb file/database structure.
};

CCodeDescrLoader loader(&db);
loader.Flush(); // delete all records from the MyTableName
loader.AddNew();
   int nNewId = loader.GetId();
   loader.SetCode(L"New");
   loader.SetDescr(L"Descr");
   ds_json::object obj;
   	CCodeDescrLoader::SetRemark(obj, "MyRemark");
	CCodeDescrLoader::SetOrder(obj,  2);
   loader.SetData(obj); // do store json data
VERIFY(loader.Update());

loader.DeleteById(nNewId); // do delete one record ID=nNewId
// do delete all records from the Table1 with the code value: New
loader.DeleteAllByCode(L"New"); 

// do iterate all Table1 records and do retrieve code values
std::vector<std::wstring> codes;
if ( loader.MoveFirst() )
{
    while ( !loader.IsEOF() ) )
    {
        std::wstring sCode = loader.GetCode(); 
	codes.push_back(sCode); 
        loader.MoveNext();
    }
}

```
```C++
void OnDatabaseError(const wchar_t* sError) 
{
    TRACE(sError); TRACE(L"\n");
}

dsDatabase db;
db.OpenDB(_T("database.mdb")); 
// Exception free code, do handle all errors through the defined function
db.SetErrorHandler(OnDatabaseError); 
```
