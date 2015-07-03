#include "StdAfx.h"
#include "DBUtilsImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
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
		    TRACE(_T("DBUtilsImpl.DLL Initializing!\n"));
            ::DisableThreadLibraryCalls(hInstance); 
        }
        break;
    case DLL_PROCESS_DETACH:
        TRACE(_T("DBUtilsImpl.DLL Terminating!\n"));
        break;
	}
	
	return 1;   
}
