// DataSocket.cpp : 구현 파일입니다.
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


// CDataSocket 멤버 함수

void CDataSocket::OnReceive(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	(*AfxGetMainWnd()).SendMessage(UM_RECEIVE_MESSAGE, (WPARAM)this);

	CSocket::OnReceive(nErrorCode);
}

void CDataSocket::OnClose(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	(*AfxGetMainWnd()).SendMessage(UM_CLOSE_MESSAGE, (WPARAM)this);

	CSocket::OnClose(nErrorCode);
}