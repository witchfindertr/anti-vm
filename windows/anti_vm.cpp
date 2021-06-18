// anti_vm.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")


bool InitWMI(IWbemServices **pSvc, IWbemLocator **pLoc, const TCHAR* szNetworkResource)
{
	HRESULT hres;
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	hres = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(pLoc));
	BSTR strNetworkResource = SysAllocString(szNetworkResource);
	if (strNetworkResource) 
	{
		hres = (*pLoc)->ConnectServer(strNetworkResource, NULL, NULL, NULL, WBEM_FLAG_CONNECT_USE_MAX_WAIT, 0, 0, pSvc);
		SysFreeString(strNetworkResource);
	}
	hres = CoSetProxyBlanket(*pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
	return 1;
}
BOOL ExecWMIQuery(IWbemServices **pSvc, IWbemLocator **pLoc, IEnumWbemClassObject **pEnumerator, const TCHAR* szQuery)
{
	BSTR strQueryLanguage = SysAllocString(OLESTR("WQL"));
	BSTR strQuery = SysAllocString(szQuery);
	BOOL bQueryResult = TRUE;
	if (strQueryLanguage && strQuery) HRESULT hres = (*pSvc)->ExecQuery(strQueryLanguage, strQuery, WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, pEnumerator);
	if (strQueryLanguage) SysFreeString(strQueryLanguage);
	if (strQuery) SysFreeString(strQuery);
	return bQueryResult;
}

int wmi_query_count(const _TCHAR* query)
{
    IWbemServices *pSvc = NULL;
    IWbemLocator *pLoc = NULL;
    IEnumWbemClassObject* pEnumerator = NULL;
    BOOL bStatus = FALSE;
    HRESULT hRes;
    int count = 0;

    // Init WMI
    bStatus = InitWMI(&pSvc, &pLoc, _T("ROOT\\CIMV2"));
    if (bStatus)
    {
        // If success, execute the desired query
        bStatus = ExecWMIQuery(&pSvc, &pLoc, &pEnumerator, query);
        if (bStatus)
        {
            // Get the data from the query
            IWbemClassObject *pclsObj = NULL;
            ULONG uReturn = 0;

            // Iterate over our enumator
            while (pEnumerator)
            {
                hRes = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                if (0 == uReturn) break;
                count++;
                pclsObj->Release();
            }
		}
            pSvc->Release();
            pLoc->Release();
            CoUninitialize();        
    }
    else return -1;
    return count;
}

void _tmain(int argc, _TCHAR* argv[])
{
    std::cout << "You are using " << (wmi_query_count(_T("SELECT * FROM Win32_PortConnector")) == 0 ? "Virtual" : "Real") << " Environment"; 
}

