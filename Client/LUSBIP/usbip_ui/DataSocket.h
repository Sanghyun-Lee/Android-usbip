#pragma once
#define UM_RECEIVE_MESSAGE (WM_USER+102)
#define UM_CLOSE_MESSAGE (WM_USER+103)
// CDataSocket ��� ����Դϴ�.

class CDataSocket : public CSocket
{
public:
	CDataSocket();
	virtual ~CDataSocket();
//	virtual void OnReceive(int nErrorCode);
//	virtual void OnClose(int nErrorCode);
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
};


