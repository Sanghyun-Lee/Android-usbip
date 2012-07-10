#pragma once
#define UM_ACCEPT_MESSAGE (WM_USER + 101)

// CListenSocket 명령 대상입니다.

class CListenSocket : public CSocket
{
public:
	CListenSocket();
	virtual ~CListenSocket();
	virtual void OnAccept(int nErrorCode);
};


