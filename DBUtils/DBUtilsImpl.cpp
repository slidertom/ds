#include "stdafx.h"
#include "DBUtilsImpl.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
#endif

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    // Remove this if you use lpReserved
    UNREFERENCED_PARAMETER(lpReserved);

    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        {
            TRACE(L"DBUtilsImpl.DLL Initializing!\n");
            ::DisableThreadLibraryCalls(hInstance);
        }
        break;
    case DLL_PROCESS_DETACH:
        TRACE(L"DBUtilsImpl.DLL Terminating!\n");
        break;
    }
    
    return 1;   
}
