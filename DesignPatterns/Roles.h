#ifndef __ROLES_H__
#define __ROLES_H__
#pragma once

/*
    Copyright (C) by Tomas Rapkauskas
    All rights reserved.
    Author: Tomas Rapkauskas (slidertom@gmail.com) 

    This program is free  software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 2.1 of the License, or
    (at your option) any later version.

    This program  is  distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should  have received a copy of the GNU Lesser General Public License
    along with  this program; If not, see <http://www.gnu.org/licenses/>.
*/

#include "string"
#include "unordered_map"

class CRoleCore;

class CRole
{
// Contruction/Destruction
public:
    CRole(const std::string &sName, CRoleCore *pRoleCore) : m_sName(sName), m_pRoleCore(pRoleCore) { }
    CRole(const char *sName,        CRoleCore *pRoleCore) : m_sName(sName), m_pRoleCore(pRoleCore) { }

protected:
    friend class CRoleCore;

    virtual ~CRole() { } 

// Attributes
public:
    const char *GetName() const { return m_sName.c_str(); }

    static bool DeleteRole(CRole *pRole);

protected:
    virtual CRole *Clone(CRoleCore *pTarget) const { ASSERT(false); return new CRole(m_sName, pTarget); } // Implementation should be done outside

// Atributes
protected:
    CRoleCore *m_pRoleCore;

private:
    std::string m_sName;
};

class CRoleCore
{
// Contruction/Destruction
public:
    typedef std::unordered_map<std::string, CRole*> CRolesMap;
    CRoleCore() { }
    CRoleCore(const CRoleCore &r) { *this = r; }
    CRoleCore(CRoleCore &&r)  { 
        CRole *pRole = nullptr;
        for (auto &elem : r.m_roles_map) {
            pRole  = elem.second;
            ASSERT(pRole);
            pRole->m_pRoleCore = this;
            this->AddRole(pRole);
        }
        r.m_roles_map.clear();
    }
    virtual ~CRoleCore() {  DeleteAllRoles(); }

// Operations
public:
    void DeleteAllRoles() 
    { 
        CRolesMap::iterator beg_it = m_roles_map.begin();
        CRolesMap::iterator end_it = m_roles_map.end();
        for (CRolesMap::iterator it = beg_it; it != end_it; ++it) {
            if ( it->second ) {
                delete it->second;
            }
        }
        m_roles_map.clear();
    }
    
    bool AddRole(CRole *pRole);

    CRole *GetRole(const char *sName);
    CRole *GetRole(const char *sName) const;

    bool RemoveRole(const char *sName);
    void Clone(CRoleCore *pTarget) const;

    size_t size() const { return m_roles_map.size(); }

    typedef CRolesMap::iterator       iterator;
    typedef CRolesMap::const_iterator const_iterator;

          iterator begin()       { return m_roles_map.begin(); }
          iterator end()         { return m_roles_map.end();   }
    const_iterator begin() const { return m_roles_map.begin(); }
    const_iterator end()   const { return m_roles_map.end();   }

// Operators
public:
    void operator=(const CRoleCore &r) {
        r.Clone(this);
    }

    CRoleCore &operator=(CRoleCore &&r) {
        DeleteAllRoles(); // do delete all roles
        CRole *pRole = nullptr;
        for (auto &elem : r.m_roles_map) {
            pRole  = elem.second;
            ASSERT(pRole);
            pRole->m_pRoleCore = this;
            this->AddRole(pRole);
        }
        r.m_roles_map.clear();
        return *this;
    }

// Attributes
private:
    CRolesMap m_roles_map;
};

template <class role_class>
class CGetRoleImpl {
// Static operations
public:
    static const role_class *GetRoleImpl(const CRoleCore *pCore) {
        ASSERT(pCore);
        const CRole *pRole = pCore->GetRole(role_class::GetRoleName());
        if ( !pRole ) {
            return nullptr;
        }
        const role_class *pHoles = (const role_class *)pRole;
        return pHoles;
    }

    static role_class *GetRoleImpl(CRoleCore *pCore) {
        ASSERT(pCore);
        CRole *pRole = pCore->GetRole(role_class::GetRoleName());
        if ( !pRole ) {
            return nullptr;
        }
        role_class *pHoles = (role_class *)pRole;
        return pHoles;
    }
};

////////////////////////////////////////////////////////////////////////////////////////
// Inline functions definitions
////////////////////////////////////////////////////////////////////////////////////////

inline bool CRole::DeleteRole(CRole *pRole)
{
    ASSERT(pRole);
    return pRole->m_pRoleCore->RemoveRole(pRole->m_sName.c_str());
}

inline bool CRoleCore::AddRole(CRole *pRole)
{
    CRolesMap::const_iterator found = m_roles_map.find(pRole->m_sName.c_str());
    if ( found != m_roles_map.end() ) {
        ASSERT(false);
        return false;
    }
    m_roles_map[pRole->m_sName.c_str()] = pRole;
    return true;
}

inline bool CRoleCore::RemoveRole(const char *sName)
{
    CRolesMap::iterator found = m_roles_map.find(sName);
    if ( found != m_roles_map.end() ) {
        delete found->second;
        m_roles_map.erase(found);
        return true;
    }	
    return false;
}

inline CRole *CRoleCore::GetRole(const char *sName)
{
    CRolesMap::iterator found = m_roles_map.find(sName);
    if ( found != m_roles_map.end() ) {
        return found->second;
    }
    return nullptr;
}

inline CRole *CRoleCore::GetRole(const char *sName) const
{
    CRolesMap::const_iterator found = m_roles_map.find(sName);
    if ( found != m_roles_map.end() ) {
        return found->second;
    }
    return nullptr;
}

inline void CRoleCore::Clone(CRoleCore *pTarget) const
{
    pTarget->DeleteAllRoles(); // do delete all target roles
    CRole *pClone = nullptr;
    auto end_it = m_roles_map.end();
    for (auto it = m_roles_map.begin(); it != end_it; ++it)
    {
        pClone = it->second->Clone(pTarget);
        // Possibility to clone only needed info
        if ( pClone )  {
            pTarget->AddRole(pClone);
        }
    }
}

#endif
