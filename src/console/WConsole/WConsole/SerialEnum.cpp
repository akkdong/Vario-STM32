//
// SerialEnum.cpp
//
// implementation of the serial enumerator class
//
// Martin Jansen
//

#include "stdafx.h"
#include "SerialEnum.h"

#include <TCHAR.H> 
#include <stdlib.h>

#ifndef _WINSPOOL_
   #include <winspool.h>
#endif

#ifndef _INC_SETUPAPI
	#include <setupapi.h>
#endif

using namespace std; // needed for the STL list library functions

void CSerialEnum::CheckNamePort( LPTSTR lpstrPortName, list<int> &nPortArray )
{
	int nLen;
	if( (nLen = (int)_tcslen( lpstrPortName )) > 3 )
    {
        if( _tcsnicmp(lpstrPortName, _T("COM"), 3) == 0) 
        {
			int nPortNr = _ttoi( lpstrPortName + 3 );
			if( nPortNr != 0 )
			{
				lpstrPortName[ nLen -1 ] = '\0';
				//
				// This is a real registered serial port number,
				// All we now have to do is check if it really exists with the 
				// slow GetDefaultCommConfig
				COMMCONFIG cc;
				DWORD dwSize = sizeof(COMMCONFIG);
				if( GetDefaultCommConfig( lpstrPortName, &cc, &dwSize))
				{
					nPortArray.push_back( nPortNr );
				}
			}
		}
	}
}

//
// Retrieve the available serial ports
//
short CSerialEnum::GetSerialPortsEnum( list<int> &nPortArray )
{
	nPortArray.clear(); // empty the list

	//
	//Call the first time to determine the size of the buffer to allocate
	//
	DWORD dwNeeded = 0;
	DWORD dwPorts = 0;
	EnumPorts(NULL, 1, NULL, 0, &dwNeeded, &dwPorts);

	//
	// Allocate the buffer and call the enumports function again
	//
	BYTE* pPorts = (BYTE*) new BYTE[ dwNeeded ]; 
	//
	// Use enumports for a fast search of all registered serial ports;
	// if parameter 2 is changed for 1 to 2 a more extensive description 
	// can be obtained by using the EnumPorts function
	//
	if( EnumPorts( NULL, 1, pPorts, dwNeeded, &dwNeeded, &dwPorts ))
	{
		PORT_INFO_1 *pPortInfo = (PORT_INFO_1*) pPorts;

		for( DWORD dwCount = 0; dwCount < dwPorts; dwCount++, pPortInfo++ )
		{
			//
			// Check if the name holds the COM part and look if 
			// it is really available and store port number in array
			//
			CheckNamePort( pPortInfo->pName, nPortArray);
		}
	}

	// Delete the allocated memory for the pPorts array
	delete [] pPorts; 

 	//
	// Sort the array
	//
	nPortArray.sort();

	return (short)nPortArray.size(); // 0 means no serial ports found
}


// Needed for the SetupApi class (see DDK for description)
typedef HKEY (__stdcall SETUPDIOPENDEVREGKEY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, DWORD, REGSAM);
typedef BOOL (__stdcall SETUPDICLASSGUIDSFROMNAME)(LPCTSTR, LPGUID, DWORD, PDWORD);
typedef BOOL (__stdcall SETUPDIDESTROYDEVICEINFOLIST)(HDEVINFO);
typedef BOOL (__stdcall SETUPDIENUMDEVICEINFO)(HDEVINFO, DWORD, PSP_DEVINFO_DATA);
typedef HDEVINFO (__stdcall SETUPDIGETCLASSDEVS)(LPGUID, LPCTSTR, HWND, DWORD);
typedef BOOL (__stdcall SETUPDIGETDEVICEREGISTRYPROPERTY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, PDWORD, PBYTE, DWORD, PDWORD);

short CSerialEnum::GetSerialPorts( std::list<int> &nPortArray )
{
	nPortArray.clear(); // empty the list

	// Use the loading with the setupapi.dll instead of the lib to make this work on Windows 95
	HINSTANCE hInstSetupAPI = LoadLibrary(_T("SETUPAPI.DLL"));
	if( hInstSetupAPI == NULL)
	{
		// Cannot load the SetupApi.DLL, maybe this windows version does not support it e.g Win95
		// Use the normal GetSerialPorts() function
		return GetSerialPortsEnum( nPortArray );
	}
	
	// Get the function pointers in the setupapi.dll
	// SetupDiOpenDevRegKey, SetupDiClassGuidsFromName, SetupDiGetDeviceRegistryProperty, SetupDiGetDeviceRegistryPropertyW
	// SetupDiDestroyDeviceInfoList, SetupDiEnumDeviceInfo
	SETUPDIOPENDEVREGKEY* lpfnLPSETUPDIOPENDEVREGKEY = (SETUPDIOPENDEVREGKEY*) GetProcAddress( hInstSetupAPI, "SetupDiOpenDevRegKey");
#ifdef _UNICODE
	SETUPDICLASSGUIDSFROMNAME* lpfnSETUPDICLASSGUIDSFROMNAME = (SETUPDICLASSGUIDSFROMNAME*) GetProcAddress( hInstSetupAPI, "SetupDiClassGuidsFromNameW");
	SETUPDIGETCLASSDEVS* lpfnSETUPDIGETCLASSDEVS = (SETUPDIGETCLASSDEVS*) GetProcAddress( hInstSetupAPI, "SetupDiGetClassDevsW");
	SETUPDIGETDEVICEREGISTRYPROPERTY* lpfnSETUPDIGETDEVICEREGISTRYPROPERTY = (SETUPDIGETDEVICEREGISTRYPROPERTY*) GetProcAddress( hInstSetupAPI, "SetupDiGetDeviceRegistryPropertyW");
#else
	SETUPDICLASSGUIDSFROMNAME* lpfnSETUPDICLASSGUIDSFROMNAME = (SETUPDICLASSGUIDSFROMNAME*) GetProcAddress( hInstSetupAPI, "SetupDiClassGuidsFromNameA");
	SETUPDIGETCLASSDEVS* lpfnSETUPDIGETCLASSDEVS = (SETUPDIGETCLASSDEVS*) GetProcAddress( hInstSetupAPI, "SetupDiGetClassDevsA");
	SETUPDIGETDEVICEREGISTRYPROPERTY* lpfnSETUPDIGETDEVICEREGISTRYPROPERTY = (SETUPDIGETDEVICEREGISTRYPROPERTY*) GetProcAddress( hInstSetupAPI, "SetupDiGetDeviceRegistryPropertyA");
#endif
	SETUPDIDESTROYDEVICEINFOLIST* lpfnSETUPDIDESTROYDEVICEINFOLIST = (SETUPDIDESTROYDEVICEINFOLIST*) GetProcAddress( hInstSetupAPI, "SetupDiDestroyDeviceInfoList");
	SETUPDIENUMDEVICEINFO* lpfnSETUPDIENUMDEVICEINFO = (SETUPDIENUMDEVICEINFO*) GetProcAddress( hInstSetupAPI, "SetupDiEnumDeviceInfo");

	if ((lpfnLPSETUPDIOPENDEVREGKEY == NULL) || (lpfnSETUPDICLASSGUIDSFROMNAME == NULL) || (lpfnSETUPDIDESTROYDEVICEINFOLIST == NULL) ||
		(lpfnSETUPDIENUMDEVICEINFO == NULL) || (lpfnSETUPDIGETCLASSDEVS == NULL) || (lpfnSETUPDIGETDEVICEREGISTRYPROPERTY == NULL))
	{
		// Unload the setupapi.dll
		// One of more of the function pointer could not be cought
		// Use the Normal GetSerialPorts() function
		FreeLibrary( hInstSetupAPI );
		return GetSerialPortsEnum( nPortArray );
	}
  
	// Get the Ports GUID by using the SetupDiClassGuidsFromName
	// Normally the GUID should be {4D36E978-E325-11CE-BFC1-08002BE10318}
	// First get the amount of space needed for the guid
	DWORD dwGuids = 0;
	lpfnSETUPDICLASSGUIDSFROMNAME(_T("Ports"), NULL, 0, &dwGuids);
	if (dwGuids == 0)
	{
		// Unable to get the needed guid size
		// Unload the setupapi.dll
		// Use the Normal GetSerialPorts() function
		FreeLibrary( hInstSetupAPI);
		return GetSerialPortsEnum( nPortArray );
	}

	// Allocate the needed memory
	GUID* pGuids = new GUID[ dwGuids ];

	// Call the function again, to finally get the guid
	if( !lpfnSETUPDICLASSGUIDSFROMNAME( _T("Ports"), pGuids, dwGuids, &dwGuids))
	{
		// Unable to get the needed guid size
		// Unload the setupapi.dll
		// Use the Normal GetSerialPorts() function
		// Free the memory allocatyed for the GUID
	    delete [] pGuids;
		FreeLibrary( hInstSetupAPI);
		return GetSerialPortsEnum( nPortArray );
	}

	// Now create a "device information set" which is required to enumerate all the ports
	HDEVINFO hDevInfoSet = lpfnSETUPDIGETCLASSDEVS( pGuids, NULL, NULL, DIGCF_PRESENT);
	if( hDevInfoSet == INVALID_HANDLE_VALUE )
	{
		// Unable to get the needed guid size
		// Unload the setupapi.dll
		// Use the Normal GetSerialPorts() function
		// Free the memory allocatyed for the GUID
		delete [] pGuids;
		FreeLibrary( hInstSetupAPI);
		return GetSerialPortsEnum( nPortArray );
	}

	// Finished with the Guids by this time
	delete [] pGuids;
	pGuids = NULL;

	//Finally do the enumeration
	BOOL bFoundPort;
	int nIndex = 0;
	SP_DEVINFO_DATA devInfo;
	
	do
	{
		// Enumerate the current device
		devInfo.cbSize = sizeof( SP_DEVINFO_DATA);
		bFoundPort = lpfnSETUPDIENUMDEVICEINFO( hDevInfoSet, nIndex, &devInfo);
		if( bFoundPort)
		{
			// Get the registry key which stores the ports settings
			HKEY hDeviceKey = lpfnLPSETUPDIOPENDEVREGKEY( hDevInfoSet, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
			if( hDeviceKey)
			{
				// Read in the name of the port
				TCHAR pszPortName[20];
				DWORD dwSize = sizeof(pszPortName);
				DWORD dwType = 0;
  				if( (RegQueryValueEx(hDeviceKey, _T("PortName"), NULL, &dwType, (LPBYTE) pszPortName, &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
				{
					// Check if it really is a com port
					if( _tcsnicmp( pszPortName, _T("COM"), 3) == 0) 
					{
						int nPortNr = _ttoi( pszPortName + 3 );
						if( nPortNr != 0 )
						{
							// Add the port number to the back of the list
							nPortArray.push_back( nPortNr );
						}
					}
				}
				// Close the key now that we are finished with it
				RegCloseKey(hDeviceKey);
			}
		}
		++nIndex;
	}while( bFoundPort );

	//Free up the "device information set" now that we are finished with it
	lpfnSETUPDIDESTROYDEVICEINFOLIST( hDevInfoSet);

	//Unload the setup dll
	FreeLibrary( hInstSetupAPI);

	//
	// Sort the array, I do not think it is needed but just to be sure
	//
	nPortArray.sort();

	return (short)nPortArray.size(); // 0 means no serial ports found
}

// FindMenuItem() will find a menu item string from the specified
// popup menu and returns its position (0-based) in the specified 
// popup menu. It returns -1 if no such menu item string is found.
int CSerialEnum::FindMenuItem(CMenu* Menu, LPCTSTR MenuString)
{
#if 0
   int count = Menu->GetMenuItemCount();
   for (int i = 0; i < count; i++)
   {
      CString str;
      if (Menu->GetMenuString(i, str, MF_BYPOSITION) && (strcmp(str, MenuString) == 0))
         return i;
   }
#endif
   return -1;
}

void CSerialEnum::UpdatePortMenuItem(CMenu *menu, int com1_id, int selection)
{
#if 0
	std::list< int> listPorts;
    listPorts.clear();
	if( GetSerialPorts( listPorts ))
	{
		CString str;
		std::list<int>::iterator it;
		int pos;

		if ((pos = FindMenuItem(menu, "&Settings")) == -1)
			return;

		menu = menu->GetSubMenu(pos);
		if ((pos = FindMenuItem(menu, "&Port")) == -1)
			return;

		menu = menu->GetSubMenu(pos);
		while(menu->RemoveMenu(0,MF_BYPOSITION) != 0)		// Remove all items
			continue;

		for(pos=0,it=listPorts.begin(); it!=listPorts.end(); it++ )
		{
			str.Format("COM%d", *it);
			menu->InsertMenu(pos,MF_BYPOSITION,com1_id+*it-1,str);
			if (*it == selection)
				menu->CheckMenuItem(pos, MF_CHECKED | MF_BYPOSITION);
			else
				menu->CheckMenuItem(pos, MF_UNCHECKED | MF_BYPOSITION);
			pos++;
		}
	}
#endif
}

