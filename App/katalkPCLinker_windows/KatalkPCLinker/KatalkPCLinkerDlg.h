
// KatalkPCLinkerDlg.h : 헤더 파일
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "DataSocket.h"
#include "atlimage.h"

#define PORT 3600
#define MAXLINE 1024
#define INPUT_UP 1
#define INPUT_DOWN 2
#define INPUT_LEFT 3
#define INPUT_RIGHT 4
#define INPUT_ENTER 5
#define INPUT_BACK 6


// CKatalkPCLinkerDlg 대화 상자
class CKatalkPCLinkerDlg : public CDialogEx
{
// 생성입니다.
public:
	CKatalkPCLinkerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	CDataSocket dataSocket;
	CString m_strServerAddress;
// 대화 상자 데이터입니다.

	enum { IDD = IDD_KATALKPCLINKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnCloseSocket(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceiveData(WPARAM wParam, LPARAM lParam);
	void AddMessage(CString strMsg);
	DECLARE_MESSAGE_MAP()
public:
	CString m_strListData;
	CString m_strSendData;
	CEdit m_edListData;
	CIPAddressCtrl m_edIpServerAddress;
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonSend();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CImage img;
	CString AnsiToUTF8RetCString(CString inputStr);
	CRect imgViewerRect;
	afx_msg void OnBnClickedUp();
	afx_msg void OnBnClickedLeft();
	afx_msg void OnBnClickedDown();
	afx_msg void OnBnClickedRight();
	afx_msg void OnBnClickedEnter();
	afx_msg void OnCbnSelchangeOpenpack();
	CComboBox selOpenPack;
	afx_msg void OnBnClickedBack();
	void inputDirectioinKey(int key);
	void sendMsg(CString strMsg);
};
