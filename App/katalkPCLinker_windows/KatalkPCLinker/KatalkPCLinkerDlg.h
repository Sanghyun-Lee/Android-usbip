
// KatalkPCLinkerDlg.h : ��� ����
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


// CKatalkPCLinkerDlg ��ȭ ����
class CKatalkPCLinkerDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CKatalkPCLinkerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.
	CDataSocket dataSocket;
	CString m_strServerAddress;
// ��ȭ ���� �������Դϴ�.

	enum { IDD = IDD_KATALKPCLINKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
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
