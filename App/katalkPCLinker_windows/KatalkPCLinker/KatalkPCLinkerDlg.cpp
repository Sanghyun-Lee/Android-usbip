
// KatalkPCLinkerDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "KatalkPCLinker.h"
#include "KatalkPCLinkerDlg.h"
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


// CKatalkPCLinkerDlg ��ȭ ����




CKatalkPCLinkerDlg::CKatalkPCLinkerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CKatalkPCLinkerDlg::IDD, pParent)
	, m_strListData(_T(""))
	, m_strSendData(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CKatalkPCLinkerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_LIST, m_strListData);
	DDX_Text(pDX, IDC_EDIT_SEND_DATA, m_strSendData);
	DDX_Control(pDX, IDC_EDIT_LIST, m_edListData);
	DDX_Control(pDX, IDC_IPADDRESS, m_edIpServerAddress);
	DDX_Control(pDX, IDC_OPENPACK, selOpenPack);
}

BEGIN_MESSAGE_MAP(CKatalkPCLinkerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CKatalkPCLinkerDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CKatalkPCLinkerDlg::OnBnClickedButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CKatalkPCLinkerDlg::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_UP, &CKatalkPCLinkerDlg::OnBnClickedUp)
	ON_BN_CLICKED(IDC_LEFT, &CKatalkPCLinkerDlg::OnBnClickedLeft)
	ON_BN_CLICKED(IDC_DOWN, &CKatalkPCLinkerDlg::OnBnClickedDown)
	ON_BN_CLICKED(IDC_RIGHT, &CKatalkPCLinkerDlg::OnBnClickedRight)
	ON_BN_CLICKED(IDC_ENTER, &CKatalkPCLinkerDlg::OnBnClickedEnter)
	ON_CBN_SELCHANGE(IDC_OPENPACK, &CKatalkPCLinkerDlg::OnCbnSelchangeOpenpack)
	ON_BN_CLICKED(IDC_BACK, &CKatalkPCLinkerDlg::OnBnClickedBack)
END_MESSAGE_MAP()


// CKatalkPCLinkerDlg �޽��� ó����

BOOL CKatalkPCLinkerDlg::OnInitDialog()
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

	// ��ó��Ʈ���� ũ�⸦ tmpRect�� ����
	CRect tmpRect;
	GetDlgItem(IDC_PICVIEW)->MoveWindow(10, 10, 300, 370);
	GetDlgItem(IDC_PICVIEW)->GetClientRect(tmpRect);
	// ��ó��Ʈ���� tmpRect�� ũ��� �׸��� �Ǹ� 0,0���� �����ǹǷ� x, y�� 10�� �߰�
	imgViewerRect.SetRect(tmpRect.left+10, tmpRect.top+10, tmpRect.right+10, tmpRect.bottom+10);

	// ī�� �̹����� �ε�, ���� ������ �̹����� ������ �̹��� �ε尡 �ȵ�
	img.Load(CString("katalk.jpg"));
	Invalidate();

	// �޽����� IDC_EDIT_LIST�� ���

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CKatalkPCLinkerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CKatalkPCLinkerDlg::OnPaint()
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
		// ī�� �̹����� ��ó��Ʈ�ѿ� ���
		CPaintDC dc(this); 
	
		img.Draw(dc.m_hDC, imgViewerRect);

		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CKatalkPCLinkerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CKatalkPCLinkerDlg::OnBnClickedButtonConnect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData();

	if(dataSocket.m_hSocket != INVALID_SOCKET)
	{
		AddMessage("�̹� ������ ���� �Ǿ��ֽ��ϴ�.");
		return;
	}
	
	// IP�ּҸ� �Է������� �Էµ� IP�ּҷ� ����
	if(m_strServerAddress != "")
	{
		m_edIpServerAddress.GetWindowText(m_strServerAddress);
	}
	// IP�ּҸ� �Է����� ������ �⺻ IP 192.168.0.11�� ����
	else
	{
		m_strServerAddress = "192.168.0.11";
	}

	// SOCKET m_hSocket�� CAsyncSocket Ŭ������ �ִ� ����

	if(!dataSocket.Create())
	{
		AddMessage("���� ���� ����");
		return;
	}

	// IP�� ��Ʈ�� ���ῡ �����ϴ� ���
	if(!dataSocket.Connect(m_strServerAddress, PORT))
	{
		dataSocket.Close();
		AddMessage("���� ���� ����");
	}
	// ���� ������ ����� IP�� ��Ʈ��ȣ ���
	else
	{
		AddMessage("������ ���� �ƽ��ϴ�.");
		
		// ���Ӳ��� ��ư Ȱ��ȭ, �������� ��ư ��Ȱ��ȭ
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(FALSE);
		// ������ ��ư Ȱ��ȭ, �޼��� �Է�â�� ��Ŀ���� ��
		GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_SEND_DATA)->SetFocus();
	}
}

void CKatalkPCLinkerDlg::OnBnClickedButtonStop()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	// dataSocket�� ��ȿ�� ������ ��쿡 ���� ������ ��ħ
	if(dataSocket.m_hSocket != INVALID_SOCKET)
	{
		dataSocket.Close();
		AddMessage("�������� ������ �����մϴ�.");
		// ���Ӳ��� ��ư ��Ȱ��ȭ, �������� ��ư Ȱ��ȭ, ������ ��ư ��Ȱ��ȭ
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(FALSE);
	}
	else
	{
		AddMessage("�̹� ����Ǿ����ϴ�.");
		return;
	}
}


void CKatalkPCLinkerDlg::OnBnClickedButtonSend()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	
	// �Է��� �޼����� strSend�� �Ҵ�
	UpdateData();
	CString strSend;
	strSend.Format("%s", m_strSendData);

	// Ansi�� UTF8�� �����ϰ� CString���� ���ϵ� ���� ����
	sendMsg(strSend);
	
	GetDlgItem(IDC_EDIT_SEND_DATA)->SetFocus();
	m_strSendData = "";
	UpdateData(FALSE);
}

// Ű���� ��� �̸� ����ä�� ó�����ִ� �޼ҵ�(��������)
BOOL CKatalkPCLinkerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	
	// Ű�� ��������
	if(pMsg->message == WM_KEYDOWN)
	{
		// ����Ű�� ��������
		if(pMsg->wParam == VK_RETURN)
		{
			// ��Ŀ���� �޼��� �Է�â�� ������
			
		}
		// ESC�� ������
		else if(pMsg->wParam == VK_ESCAPE)
		{
		
		}

		// ����Ű�� ������ ������ ���� ��� ����
		switch(pMsg->wParam){
		case VK_RETURN:
			if(GetFocus()->GetDlgCtrlID() == IDC_EDIT_SEND_DATA)
			{
				// �޼��� ���� ����
				OnBnClickedButtonSend();
				// EnterŰ �Է½� ���� â�� ����ǹǷ� �� ������ �������� return TRUE�� ����ߵ�
				return TRUE;
			}
			else
			{
				// �׳� ���͸� ������ ���� ��� ����
				OnBnClickedEnter();
				return TRUE;
			}
		case VK_ESCAPE:
			// �ڷΰ��� ��� ����
			OnBnClickedBack();
			// ESCŰ�� ���� ���Ḧ �ϱ⶧���� �� ������ �������� retrun TRUE�� ����ߵ�
			return TRUE; 
		case VK_LEFT:
			if(GetFocus()->GetDlgCtrlID() == IDC_EDIT_SEND_DATA){
					
			}else{
				OnBnClickedLeft();
			}
			break;
		case VK_RIGHT:
			if(GetFocus()->GetDlgCtrlID() == IDC_EDIT_SEND_DATA){
					
			}else{
				OnBnClickedRight();
			}
			break;
		case VK_UP:
			OnBnClickedUp();
			break;
		case VK_DOWN:
			OnBnClickedDown();
			break;
		default:
			// no effect
			break;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

// ���� ����Ʈ ��Ʈ�ѿ� �޽����� ������ִ� �޼ҵ�
void CKatalkPCLinkerDlg::AddMessage(CString strMsg)
{

	UpdateData();
	
	m_strListData += strMsg + "\r\n";

	UpdateData(FALSE);

	m_edListData.LineScroll(m_edListData.GetLineCount());
}

// �����κ��� �� �޼����� �޾Ƽ� ����� ��Ʈ�ѿ� ���
LRESULT CKatalkPCLinkerDlg::OnReceiveData(WPARAM wParam, LPARAM lParam)
{
	char Rcvdata[MAXLINE];

	CDataSocket* pDataSocket = (CDataSocket*)wParam;

	pDataSocket->Receive(Rcvdata, sizeof(Rcvdata));

	CString strMsg = Rcvdata;

	AddMessage(strMsg);

	return 0;
}

// �������� ���� ���� ó��
LRESULT CKatalkPCLinkerDlg::OnCloseSocket(WPARAM wParam, LPARAM lParam)
{
	dataSocket.Close();

	AddMessage("������ ������ ���� �߽��ϴ�.");
	// ���Ӳ��� ��ư ��Ȱ��ȭ, �������� ��ư Ȱ��ȭ, ������ ��ư ��Ȱ��ȭ
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(FALSE);

	return 0;
}

// Ansi�� UTF8 �������� ��ȯ�Ͽ� CString���� ����
CString CKatalkPCLinkerDlg::AnsiToUTF8RetCString(CString inputStr)
{
	WCHAR szUnicode[MAXLINE];
	char szUTF8char[MAXLINE];

	CString strConvert;
	char* szSrc = (LPSTR)(LPCTSTR)inputStr;

	char szRetValue[MAXLINE] = "";

	int unicodeSize = MultiByteToWideChar(CP_ACP, 0,
		szSrc, (int)strlen(szSrc), 
		szUnicode, sizeof(szUnicode));

	int UTF8CodeSize = WideCharToMultiByte(CP_UTF8, 0, 
		szUnicode, unicodeSize, szUTF8char,
		sizeof(szUTF8char), NULL, NULL);

	memcpy(szRetValue, szUTF8char, UTF8CodeSize);
	strConvert = szRetValue;

	return strConvert;

}

void CKatalkPCLinkerDlg::sendMsg(CString strMsg)
{
	int ret = 0;
	ret = dataSocket.Send(AnsiToUTF8RetCString(strMsg), 
		AnsiToUTF8RetCString(strMsg).GetLength()+1);

	if(ret < 1){
		AddMessage("�޼��� ���� ����");
		return;
	}
}
void CKatalkPCLinkerDlg::inputDirectioinKey(int key)
{
	CString strCmd;
	switch(key){
	case INPUT_UP:
		strCmd.Format("%s", "\\\\CONTROL_U");
		break;
	case INPUT_DOWN:
		strCmd.Format("%s", "\\\\CONTROL_D");
		break;
	case INPUT_RIGHT:
		strCmd.Format("%s", "\\\\CONTROL_R");
		break;
	case INPUT_LEFT:
		strCmd.Format("%s", "\\\\CONTROL_L");
		break;
	case INPUT_ENTER:
		strCmd.Format("%s", "\\\\CONTROL_E");
		break;
	case INPUT_BACK:
		strCmd.Format("%s", "\\\\CONTROL_B");
		break;
	default:
		// no-effect
		break;
	}	

	sendMsg(strCmd);
	
	GetDlgItem(IDC_IPADDRESS)->SetFocus();
	GetDlgItem(IDC_IPADDRESS)->SendMessage(WM_KILLFOCUS, NULL); 
	GetDlgItem(IDC_UP)->SendMessage(WM_KILLFOCUS, NULL); 
}
// �� �������� ó��
void CKatalkPCLinkerDlg::OnBnClickedUp()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	inputDirectioinKey(INPUT_UP);
}

// �� �������� ó��
void CKatalkPCLinkerDlg::OnBnClickedDown()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	inputDirectioinKey(INPUT_DOWN); 
}

// �� �������� ó��
void CKatalkPCLinkerDlg::OnBnClickedLeft()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	inputDirectioinKey(INPUT_LEFT);
}

// �� �������� ó��
void CKatalkPCLinkerDlg::OnBnClickedRight()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	inputDirectioinKey(INPUT_RIGHT);
}

// ����Ű ����� �����Ҷ� ó��
void CKatalkPCLinkerDlg::OnBnClickedEnter()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	inputDirectioinKey(INPUT_ENTER);
}

// ESC ����� �����Ҷ� ó��
void CKatalkPCLinkerDlg::OnBnClickedBack()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	inputDirectioinKey(INPUT_BACK);
}

// �������α׷��� �����Ҷ� ó���ϴ� �κ�
void CKatalkPCLinkerDlg::OnCbnSelchangeOpenpack()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	int nIndex = selOpenPack.GetCurSel();
	if(nIndex == -1){
		return;
	}

	CString str, cmd;
	selOpenPack.GetLBText(nIndex, str);

	// ī���� �����ϸ�
	if(str == "ī��")
	{
		// ī���� �����϶�� ����� ����
		cmd.Format("%s", "\\\\OPENPACK_com.kakao.talk");

		AddMessage("īī���� ����");	
	}
	// ������ �����ϸ�
	else if(str == "����")
	{
		// ������ �����϶�� ����� ����
		cmd.Format("%s", "\\\\OPENPACK_jp.naver.line.android");

		AddMessage("���� ����");	
	}
	

	GetDlgItem(IDC_IPADDRESS)->SetFocus();
	GetDlgItem(IDC_IPADDRESS)->SendMessage(WM_KILLFOCUS, NULL); 
	sendMsg(cmd);
	UpdateData(FALSE);
}
