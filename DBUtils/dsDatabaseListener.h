#ifndef __DS_DATABASE_LISTENER_H__
#define __DS_DATABASE_LISTENER_H__
#pragma once

class dsDatabaseListener
{
// Construction/Destruction
public:
    dsDatabaseListener() { }
    virtual ~dsDatabaseListener() { }

// Overrides
public:
    virtual void OnDatabaseClose() = 0;
};

#endif
