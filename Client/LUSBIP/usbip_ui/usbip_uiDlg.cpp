
// usbip_uiDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "usbip_ui.h"
#include "usbip_uiDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
	
// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cusbip_uiDlg ��ȭ ����




Cusbip_uiDlg::Cusbip_uiDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(Cusbip_uiDlg::IDD, pParent)
	, m_strListData(_T(""))
{	
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cusbip_uiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS, m_edIpServerAddress);
	DDX_Text(pDX, IDC_EDIT3, m_strListData);
	DDX_Control(pDX, IDC_EDIT3, m_edListData);
	DDX_Control(pDX, IDC_BUSID, selBusid);

}

BEGIN_MESSAGE_MAP(Cusbip_uiDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(UM_ACCEPT_MESSAGE, &Cusbip_uiDlg::OnAcceptClient)
	ON_MESSAGE(UM_RECEIVE_MESSAGE, &Cusbip_uiDlg::OnReceiveData)
	ON_MESSAGE(UM_CLOSE_MESSAGE, &Cusbip_uiDlg::OnCloseSocket)
	ON_BN_CLICKED(IDC_LIST, &Cusbip_uiDlg::OnBnClickedList)
	ON_BN_CLICKED(IDC_CONNECT, &Cusbip_uiDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_DISCONNECT, &Cusbip_uiDlg::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_CLOSE, &Cusbip_uiDlg::OnBnClickedClose)
	ON_CBN_SELCHANGE(IDC_BUSID, &Cusbip_uiDlg::OnCbnSelchangeBusid)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// Cusbip_uiDlg �޽��� ó����

BOOL Cusbip_uiDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	GetDlgItem(IDC_CONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE);

	if(listenSocket.m_hSocket != INVALID_SOCKET)
	{
		AddMessage("�̹� ��� �����Դϴ�.");
	}

	if(!listenSocket.Create(PORT))
	{
		AddMessage("���� ���� ����");
	}

	if(!listenSocket.Listen())
	{
		AddMessage("��� ����");


		listenSocket.Close();
	}
	
	ShellExecute(NULL, _T("open") ,_T("C:/usbip/usbip.exe"),NULL ,NULL, SW_HIDE);

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void Cusbip_uiDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void Cusbip_uiDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR Cusbip_uiDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT Cusbip_uiDlg::OnAcceptClient(WPARAM wParam, LPARAM lParam)
{
	listenSocket.Accept(dataSocket);
	return 0;
}

LRESULT Cusbip_uiDlg::OnReceiveData(WPARAM wParam, LPARAM lParam)
{
	char Rcvdata[MAXLINE];
	CString id;
	CString name;
	CDataSocket* pDataSocket = (CDataSocket*)wParam;
	

	pDataSocket->Receive(Rcvdata, sizeof(Rcvdata));

	CString strMsg = Rcvdata;
	
	if(!strMsg.Compare("list")){
		pDataSocket->Receive(Rcvdata, sizeof(Rcvdata));
		id = Rcvdata;
		pDataSocket->Receive(Rcvdata, sizeof(Rcvdata));
		name = Rcvdata;

		id = id +"("+ name+")";
		selBusid.AddString(id);

		AddMessage("��� ���� ����");
		GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE);
	}else if(!strMsg.Compare("���� ����")){
		AddMessage("���� ����");
		GetDlgItem(IDC_LIST)->EnableWindow(TRUE);
		GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE);
	}else{
		AddMessage(strMsg);
	}

	return 0;
}

// �������� ���� ���� ó��
LRESULT Cusbip_uiDlg::OnCloseSocket(WPARAM wParam, LPARAM lParam)
{
	dataSocket.Close();
	return 0;
}

void Cusbip_uiDlg::AddMessage(CString strMsg)
{

	UpdateData();
	
	m_strListData += strMsg + "\r\n";

	UpdateData(FALSE);

	m_edListData.LineScroll(m_edListData.GetLineCount());
}

BOOL Cusbip_uiDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_RETURN)
		{
			// ��Ŀ���� �޼��� �Է�â�� ������
			return TRUE;
		}
		// ESC�� ������
		else if(pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


void Cusbip_uiDlg::OnBnClickedList()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	int ret = 0;
	CString listMsg;
	CString listAddress;
	m_edIpServerAddress.GetWindowText(listAddress);
	listMsg.Format("-l");
	
	sendMsg(listMsg);
	sendMsg(listAddress);

}


void Cusbip_uiDlg::OnBnClickedConnect()
{

	int ret = 0;
	CString strMsg;
	CString strAddress;
	
	m_edIpServerAddress.GetWindowText(strAddress);
	strMsg.Format("-a");

	sendMsg(strMsg);

	sendMsg(strAddress);

	sendMsg(selectId);

	GetDlgItem(IDC_CONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DISCONNECT)->EnableWindow(TRUE);
	GetDlgItem(IDC_LIST)->EnableWindow(FALSE);
}


void Cusbip_uiDlg::OnBnClickedDisconnect()
{
	int ret = 0;
	CString strMsg;
	strMsg = "-d";

	sendMsg(strMsg);

	GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE);
	GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_LIST)->EnableWindow(TRUE);
}

void Cusbip_uiDlg::OnBnClickedClose()
{
	CString strMsg;
	strMsg = "q";

	sendMsg(strMsg);
	dataSocket.Close();
	DestroyWindow();
}


void Cusbip_uiDlg::OnCbnSelchangeBusid()
{
	int index =0;
	CString pa = "(";
	CString id= NULL;
	int nIndex = selBusid.GetCurSel();
	if(nIndex == -1){
		return;
	}

	CString busid;
	selBusid.GetLBText(nIndex, busid);

	while(pa.Compare((CString)busid.GetAt(index))){
		id = id + busid.GetAt(index);
		index++;
	}
	selectId = id;
	UpdateData(FALSE);
}

void Cusbip_uiDlg::sendMsg(CString strMsg)
{
	int ret = 0;
	ret = dataSocket.Send(strMsg, strMsg.GetLength()+1);

	if(ret < 1){
		AddMessage("�޼��� ���� ����");
		return;
	}
}

void Cusbip_uiDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	CString strMsg;
	strMsg = "q";

	sendMsg(strMsg);
	dataSocket.Close();
	CDialogEx::OnClose();
}
