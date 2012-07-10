#pragma once
#define UM_RECEIVE_MESSAGE (WM_USER+101)
#define UM_CLOSE_MESSAGE (WM_USER+102)
// CDataSocket 명령 대상입니다.

class CDataSocket : public CSocket
{
public:
	CDataSocket();
	virtual ~CDataSocket();
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
};


