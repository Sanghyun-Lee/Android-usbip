
// usbip_uiDlg.h : ��� ����
//

#pragma once

#include "DataSocket.h"
#include "ListenSocket.h"
#include "afxcmn.h"
#include "afxwin.h"

#define PORT 3700
#define MAXLINE 1024
#define PATH "C:/WinDDK/7600.16385.1/src/usb/usbip/Debug/usbip.exe."
// Cusbip_uiDlg ��ȭ ����
class Cusbip_uiDlg : public CDialogEx
{
// �����Դϴ�.
public:
	Cusbip_uiDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.
	CDataSocket dataSocket;
	CListenSocket listenSocket;
// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_USBIP_UI_DIALOG };

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

	DECLARE_MESSAGE_MAP()
public:
	CIPAddressCtrl m_edIpServerAddress;
	CString m_strListData;
	CEdit m_edListData;
	void AddMessage(CString strMsg);
	afx_msg LRESULT OnCloseSocket(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnReceiveData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAcceptClient(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedList();
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedDisconnect();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnBnClickedClose();
	CComboBox selBusid;
	CString selectId;
	afx_msg void OnCbnSelchangeBusid();
	void Cusbip_uiDlg::sendMsg(CString strMsg);
//	virtual BOOL DestroyWindow();
//	virtual void OnOK();
	afx_msg void OnClose();
};
