
// KatalkPCLinker.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CKatalkPCLinkerApp:
// �� Ŭ������ ������ ���ؼ��� KatalkPCLinker.cpp�� �����Ͻʽÿ�.
//

class CKatalkPCLinkerApp : public CWinApp
{
public:
	CKatalkPCLinkerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CKatalkPCLinkerApp theApp;