# ds
Sqlite, Ms SQL, DAO mbd databases access system. Recordset, table based interface.


dsDatabase db;
db.OpenDB(_T("mdb.sqlite"));

dsTable loader(&db, _T("Table_Name");
loader.AddNew();
    int nId = loader.GetFieldLong(_T("ID")); // after AddNew you can always retrieved new record key
    loader.SetFieldString(_T("Code"), _T("My_Code"));
loader.Update();

loader.SeekIndex(_T("ID"), nId);
loader.GetFieldString(_T("Code"));
