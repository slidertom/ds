#include "stdafx.h"
#include "dsCopyTableData.h"

#include "dsDatabase.h"
#include "dsTable.h"

#ifndef __x86_64__ 
    #include "Dao/DaoDatabaseImpl.h"
#endif

#include "sqLite/sqlite_copy_table.h"
#include "sqLite/sqlite_database_impl.h"

#include "dsStrConv.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

dsCopyTableData::dsCopyTableData(dsDatabase *pSrcDB, dsDatabase *pDstDB)
: m_pSrcDB(pSrcDB), m_pDstDB(pDstDB)
{
    ASSERT(m_pSrcDB->IsOpen());
    ASSERT(m_pDstDB->IsOpen());

    m_bAttached  = false;
}

dsCopyTableData::~dsCopyTableData()
{
    EndCopy(); // do deattach database by default 
}

bool dsCopyTableData::BeginCopy() noexcept
{
    ASSERT(!m_bAttached); // do call EndCopy
    const dsDBType nSrcTyle = m_pSrcDB->GetType();
    const dsDBType nDstTyle = m_pDstDB->GetType();
 
    if (nSrcTyle == dsDBType::SqLite && nDstTyle == dsDBType::SqLite) 
    {
        CSqLiteDatabaseImpl *pDstSqlite = dynamic_cast<CSqLiteDatabaseImpl *>(m_pDstDB->m_pDatabase);
        CSqLiteDatabaseImpl *pSrcSqLite = dynamic_cast<CSqLiteDatabaseImpl *>(m_pSrcDB->m_pDatabase);
        const bool bTransMode = pDstSqlite->IsTransMode();
        if ( bTransMode ) { // SQL logic error or missing database[1]: cannot ATTACH database within transaction
            pDstSqlite->CommitTrans(); // maybe restore point should be used...
        }
        if ( !sqlite_util::sqlite_attach_database(pSrcSqLite, pDstSqlite) ) {
            return false;
        }
        if ( bTransMode ) {
            pDstSqlite->BeginTrans();
        }
        m_bAttached = true;
    }

    return true;
}

bool dsCopyTableData::EndCopy() noexcept
{
    if ( m_bAttached ) 
    {
        CSqLiteDatabaseImpl *pDstSqlite = dynamic_cast<CSqLiteDatabaseImpl *>(m_pDstDB->m_pDatabase);
        const bool bTransMode = pDstSqlite->IsTransMode();
        if ( bTransMode ) { // SQL logic error or missing database[1]: cannot DETACH database within transaction
            pDstSqlite->CommitTrans();
        }
        if ( !sqlite_util::sqlite_detach_database(pDstSqlite) ) {
            return false;
        }
        m_bAttached = false;
        if ( bTransMode ) { 
           pDstSqlite->BeginTrans();
        }
    }

    return true;
}

bool dsCopyTableData::CopyTableData(dsTable &src_table, dsTable &dst_table, const dsTableFieldInfo &union_info) noexcept
{
    if ( !src_table.MoveFirst() ) {
        return true; // empty table
    }
  
    while ( !src_table.IsEOF() )
    {
        dst_table.AddNew();
            auto end_it = union_info.end();
            for (auto it = union_info.begin(); it != end_it; ++it) 
            {
                const TCHAR *sFieldName = it->first.c_str();

                if ( src_table.IsFieldValueNull(sFieldName) )
                {
                    dst_table.SetFieldNull(sFieldName);
                    continue;
                }

                switch (it->second) 
                {
                case dsFieldType::Text:
                    {
                        const std::wstring sValue = src_table.GetFieldString(sFieldName);
                        dst_table.SetFieldString(sFieldName, sValue.c_str());
                    }
                    break;
                case dsFieldType::Integer:
                    {
                        const int nValue = src_table.GetFieldLong(sFieldName);
                        dst_table.SetFieldLong(sFieldName, nValue);
                    }
                    break;
                case dsFieldType::Double:
                    {
                        const double dValue = src_table.GetFieldDouble(sFieldName);
                        dst_table.SetFieldDouble(sFieldName, dValue);
                    }
                    break;
                case dsFieldType::DateTime:
                    {
                        const time_t nTime = src_table.GetFieldDateTime(sFieldName);
                        dst_table.SetFieldDateTime(sFieldName, nTime);
                    }
                    break;
                case dsFieldType::Blob:
                    {
                        unsigned char *pData = nullptr;
                        size_t nSize = 0;
                        src_table.GetFieldBinary(sFieldName, &pData, nSize);        
                        dst_table.SetFieldBinary(sFieldName, pData, nSize);
                    }
                    break;
                }
            }
        VERIFY(dst_table.Update());

        src_table.MoveNext();
    }

    return true;
}

bool dsCopyTableData::CopyTableData(const wchar_t *sTableName) noexcept
{
    return CopyTableData(sTableName, sTableName);
}

bool dsCopyTableData::CopyTableData(const wchar_t *sTableNameSrc, const wchar_t *sTableNameDst) noexcept
{
    ASSERT(m_pSrcDB);
    ASSERT(m_pDstDB);

    const dsDBType nSrcType = m_pSrcDB->GetType();
    const dsDBType nDstType = m_pDstDB->GetType();

#ifndef __x86_64__ 
    if ( nDstType == dsDBType::Dao && nSrcType == dsDBType::Dao )
    {
        CDaoDatabaseImpl *pDstDao = dynamic_cast<CDaoDatabaseImpl *>(m_pDstDB->m_pDatabase);
        CDaoDatabaseImpl *pSrcDao = dynamic_cast<CDaoDatabaseImpl *>(m_pSrcDB->m_pDatabase);
        return CDaoDatabaseImpl::CopyTableData(pSrcDao, pDstDao, sTableNameSrc, sTableNameDst);
    }
#endif

    dsTableFieldInfo union_info;

    dsTableFieldInfo dst_info;
    const std::string sTableNameDstUTF8 = ds_str_conv::ConvertToUTF8(sTableNameDst);
    VERIFY(m_pDstDB->m_pDatabase->GetTableFieldInfo(sTableNameDstUTF8.c_str(), dst_info));
  
    dsTableFieldInfo src_info;
    const std::string sTableNameSrcUTF8 = ds_str_conv::ConvertToUTF8(sTableNameSrc);
    m_pSrcDB->m_pDatabase->GetTableFieldInfo(sTableNameSrcUTF8.c_str(), src_info);
    ds_table_field_info_util::fields_union(union_info, src_info, dst_info);

    if ( nDstType == dsDBType::SqLite )
    {  
        CSqLiteDatabaseImpl *pDstDBImpl = dynamic_cast<CSqLiteDatabaseImpl *>(m_pDstDB->m_pDatabase);

        if ( dst_info.size() == 0 ) {
            std::string sTableNameDstUTF8 = ds_str_conv::ConvertToUTF8(sTableNameDst);
            std::string sError = "Destination table ";
                        sError += sTableNameDstUTF8; 
                        sError += " there are no fields defined. ";
            pDstDBImpl->OnError(sError.c_str(), "sqlite_util::ImportTableData");
            return false;
        }

        // special case for the sqlite database 
        if (nSrcType == dsDBType::SqLite && m_bAttached ) 
        {
            // field count and field names should match for the sqlite insert statement
            if ( dst_info == src_info )
            {
                CSqLiteDatabaseImpl *pDstSqlite = dynamic_cast<CSqLiteDatabaseImpl *>(m_pDstDB->m_pDatabase);
                if ( sqlite_util::sqlite_insert_table_from_attached_db(pDstSqlite, 
                                                                       ds_str_conv::ConvertToUTF8(sTableNameSrc).c_str(), 
                                                                       ds_str_conv::ConvertToUTF8(sTableNameDst).c_str()) ) {
                    return true;
                }
                return false;
            }
        }
       
        return sqlite_util::ImportTableData(m_pSrcDB, pDstDBImpl, sTableNameSrc, sTableNameDst, union_info);
    }

    dsTable src_table(m_pSrcDB, sTableNameSrc);
    dsTable dst_table(m_pDstDB, sTableNameDst);
    return dsCopyTableData::CopyTableData(src_table, dst_table, union_info);
}

bool dsCopyTableData::CopyTableDataEx(const wchar_t *sTableName) noexcept
{
    ASSERT(m_pSrcDB);
    ASSERT(m_pDstDB);

    dsTableFieldInfo union_info;

    // default implementation
    const std::string sTableNameUTF8 = ds_str_conv::ConvertToUTF8(sTableName);
    if ( !ds_table_field_info_util::fields_union(union_info, m_pSrcDB->m_pDatabase, sTableNameUTF8.c_str(), m_pDstDB->m_pDatabase, sTableNameUTF8.c_str()) ) {
        return false;
    }

    dsTable src_table(m_pSrcDB, sTableName);
    dsTable dst_table(m_pDstDB, sTableName);

    dst_table.Flush();

    return dsCopyTableData::CopyTableData(src_table, dst_table, union_info);
}

dsDatabase *dsCopyTableData::GetSrcDB() noexcept
{
    ASSERT(m_pSrcDB);
    return m_pSrcDB;
}

dsDatabase *dsCopyTableData::GetDstDB() noexcept
{
    ASSERT(m_pDstDB);
    return m_pDstDB;
}
