
// usbip_ui.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// Cusbip_uiApp:
// �� Ŭ������ ������ ���ؼ��� usbip_ui.cpp�� �����Ͻʽÿ�.
//

class Cusbip_uiApp : public CWinApp
{
public:
	Cusbip_uiApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern Cusbip_uiApp theApp;