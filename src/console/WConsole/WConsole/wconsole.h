
// wconsole.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CVarioConsoleApp:
// �� Ŭ������ ������ ���ؼ��� wconsole.cpp�� �����Ͻʽÿ�.
//

class CVarioConsoleApp : public CWinApp
{
public:
	CVarioConsoleApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CVarioConsoleApp theApp;