#ifndef __DS_OPEN_PARAMS_H__
#define __DS_OPEN_PARAMS_H__
#pragma once

class dsOpenParams final
{
// Constuction/Destruction
public:
    dsOpenParams() { }
    ~dsOpenParams() { }

// Attributes
public:
    bool m_bReadOnly  {false};
    bool m_bMultiUser {false};
    bool m_bMemory    {false}; // sqlite: in-memory databases always uses exclusive locking mode
    // m_bExclusive currently supported only by the sqlite only
    // if m_bExclusive == true database can be opened only by the one process 
    // (sqlite: The number of system calls for filesystem operations is reduced) 
    // if m_bExclusive == true => m_bMultiUser setting is ignored
    bool m_bExclusive {false}; 
    std::wstring m_sKey;
};

#endif