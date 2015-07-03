# ds
Sqlite, Ms SQL, DAO mbd databases access system. Recordset, table based interface.
'''
dsDatabase db;
db.OpenDB(_T("mdb.sqlite"));

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
'''
