#pragma once
#define UM_ACCEPT_MESSAGE (WM_USER + 101)

// CListenSocket ��� ����Դϴ�.

class CListenSocket : public CSocket
{
public:
	CListenSocket();
	virtual ~CListenSocket();
	virtual void OnAccept(int nErrorCode);
};


