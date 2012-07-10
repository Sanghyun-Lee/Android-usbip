// ListenSocket.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "usbip_ui.h"
#include "ListenSocket.h"


// CListenSocket

CListenSocket::CListenSocket()
{
}

CListenSocket::~CListenSocket()
{
}


// CListenSocket 멤버 함수


void CListenSocket::OnAccept(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	AfxGetMainWnd()->SendMessage(UM_ACCEPT_MESSAGE, (WPARAM)this);

	CSocket::OnAccept(nErrorCode);
}
