// DataSocket.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "usbip_ui.h"
#include "DataSocket.h"


// CDataSocket

CDataSocket::CDataSocket()
{
}

CDataSocket::~CDataSocket()
{
}


// CDataSocket ��� �Լ�

void CDataSocket::OnReceive(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	(*AfxGetMainWnd()).SendMessage(UM_RECEIVE_MESSAGE, (WPARAM)this);

	CSocket::OnReceive(nErrorCode);
}

void CDataSocket::OnClose(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	(*AfxGetMainWnd()).SendMessage(UM_CLOSE_MESSAGE, (WPARAM)this);

	CSocket::OnClose(nErrorCode);
}