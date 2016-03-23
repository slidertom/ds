#include "StdAfx.h"
#include "dsCopyTableData.h"

#include "dsDatabase.h"
#include "dsTable.h"

#include "Dao/DaoDatabaseImpl.h"

#include "SqLite/sqlite_copy_table.h"
#include "SqLite/SqLiteDatabaseImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

dsCopyTableData::dsCopyTableData(dsDatabase *pSrcDB, dsDatabase *pDstDB)
: m_pSrcDB(pSrcDB), m_pDstDB(pDstDB)
{
    ASSERT(m_pSrcDB->IsOpen());
    ASSERT(m_pDstDB->IsOpen());

    m_bAttached = false;
}

dsCopyTableData::~dsCopyTableData()
{
    EndCopy(); // do deattach database by default 
}

bool dsCopyTableData::BeginCopy()
{
    ASSERT(!m_bAttached); // do call EndCopy
    const dsDBType nSrcTyle = m_pSrcDB->GetType();
    const dsDBType nDstTyle = m_pDstDB->GetType();
 
    if (nSrcTyle == dsType_SqLite && nDstTyle == dsType_SqLite)  {
        if ( !sqlite_util::sqlite_attach_database(m_pSrcDB, m_pDstDB) ) {
            return false;
        }
        m_bAttached = true;
    }

    return true;
}

bool dsCopyTableData::EndCopy()
{
    if ( m_bAttached ) {
        if ( !sqlite_util::sqlite_detach_database(m_pDstDB) ) {
            return false;
        }
        m_bAttached = false;
    }

    return true;
}

bool dsCopyTableData::CopyTableData(dsTable &src_table, dsTable &dst_table, const dsTableFieldInfo &union_info)
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
                case dsFieldType_Text:
                    {
                        const CStdString sValue = src_table.GetFieldString(sFieldName);
                        dst_table.SetFieldString(sFieldName, sValue.c_str());
                    }
                    break;
                case dsFieldType_Long:
                    {
                        const int nValue = src_table.GetFieldLong(sFieldName);
                        dst_table.SetFieldLong(sFieldName, nValue);
                    }
                    break;
                case dsFieldType_Double:
                    {
                        const double dValue = src_table.GetFieldDouble(sFieldName);
                        dst_table.SetFieldDouble(sFieldName, dValue);
                    }
                    break;
                case dsFieldType_DateTime:
                    {
                        const time_t nTime = src_table.GetFieldDateTime(sFieldName);
                        dst_table.SetFieldDateTime(sFieldName, nTime);
                    }
                    break;
                case dsFieldType_Binary:
                    {
                        unsigned char *pData = nullptr;
                        unsigned long nSize = 0;
                        src_table.GetFieldBinary(sFieldName, &pData, nSize);        
                        dst_table.SetFieldBinary(sFieldName, pData, nSize);
                    }
                    break;
                }
            }
        dst_table.Update();

        src_table.MoveNext();
    }

    return true;
}

bool dsCopyTableData::CopyTableData(LPCTSTR sTableName)
{
    return CopyTableData(sTableName, sTableName);
}

bool dsCopyTableData::CopyTableData(LPCTSTR sTableNameSrc, LPCTSTR sTableNameDst)
{
    ASSERT(m_pSrcDB);
	ASSERT(m_pDstDB);

    const dsDBType nSrcType = m_pSrcDB->GetType();
    const dsDBType nDstType = m_pDstDB->GetType();

    if ( nDstType == dsType_Dao && nSrcType == dsType_Dao )
    {
        CDaoDatabaseImpl *pDstDao = dynamic_cast<CDaoDatabaseImpl *>(m_pDstDB->m_pDatabase);
        CDaoDatabaseImpl *pSrcDao = dynamic_cast<CDaoDatabaseImpl *>(m_pSrcDB->m_pDatabase);
        return CDaoDatabaseImpl::CopyTableData(pSrcDao, pDstDao, sTableNameSrc, sTableNameDst);
    }

    dsTableFieldInfo union_info;

    if ( nDstType == dsType_SqLite )
    {  
        dsTableFieldInfo dst_info;
        VERIFY(m_pDstDB->m_pDatabase->GetTableFieldInfo(sTableNameDst, dst_info));
        dsTableFieldInfo src_info;
        m_pSrcDB->m_pDatabase->GetTableFieldInfo(sTableNameSrc, src_info);
        ds_table_field_info_util::fields_union(union_info, src_info, dst_info);

        // special case for the sqlite database 
        if (nSrcType == dsType_SqLite && m_bAttached ) 
        {
            // field count should match for the sqlite insert statement
            if ( dst_info.size() == union_info.size() && dst_info.size() > 0 )
            {
                if ( sqlite_util::sqlite_insert_table_from_attached_db(m_pDstDB, sTableNameSrc, sTableNameDst) ) {
                    return true;
                }
                return false;
            }
        }
        
        CSqLiteDatabaseImpl *pDstDBImpl = dynamic_cast<CSqLiteDatabaseImpl *>(m_pDstDB->m_pDatabase);
        return sqlite_util::ImportTableData(m_pSrcDB, pDstDBImpl, sTableNameSrc, sTableNameDst, union_info);
    }

    // default implementation
    if ( !ds_table_field_info_util::fields_union(union_info, m_pSrcDB->m_pDatabase, sTableNameSrc, m_pDstDB->m_pDatabase, sTableNameDst) ) {
        return false;
    }

    dsTable src_table(m_pSrcDB, sTableNameSrc);
    dsTable dst_table(m_pDstDB, sTableNameDst);
    return dsCopyTableData::CopyTableData(src_table, dst_table, union_info);
}

dsDatabase *dsCopyTableData::GetSrcDB()
{
    ASSERT(m_pSrcDB);
    return m_pSrcDB;
}

dsDatabase *dsCopyTableData::GetDstDB()
{
    ASSERT(m_pDstDB);
    return m_pDstDB;
}
