
// KatalkPCLinkerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "KatalkPCLinker.h"
#include "KatalkPCLinkerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CKatalkPCLinkerDlg 대화 상자




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


// CKatalkPCLinkerDlg 메시지 처리기

BOOL CKatalkPCLinkerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	// 픽처컨트롤의 크기를 tmpRect에 저장
	CRect tmpRect;
	GetDlgItem(IDC_PICVIEW)->MoveWindow(10, 10, 300, 370);
	GetDlgItem(IDC_PICVIEW)->GetClientRect(tmpRect);
	// 픽처컨트롤이 tmpRect의 크기로 그리게 되면 0,0부터 설정되므로 x, y에 10씩 추가
	imgViewerRect.SetRect(tmpRect.left+10, tmpRect.top+10, tmpRect.right+10, tmpRect.bottom+10);

	// 카톡 이미지를 로드, 같은 폴더에 이미지가 없으면 이미지 로드가 안됨
	img.Load(CString("katalk.jpg"));
	Invalidate();

	// 메시지를 IDC_EDIT_LIST에 출력

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CKatalkPCLinkerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		// 카톡 이미지를 픽처컨트롤에 출력
		CPaintDC dc(this); 
	
		img.Draw(dc.m_hDC, imgViewerRect);

		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CKatalkPCLinkerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CKatalkPCLinkerDlg::OnBnClickedButtonConnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();

	if(dataSocket.m_hSocket != INVALID_SOCKET)
	{
		AddMessage("이미 서버에 접속 되어있습니다.");
		return;
	}
	
	// IP주소를 입력했으면 입력된 IP주소로 연결
	if(m_strServerAddress != "")
	{
		m_edIpServerAddress.GetWindowText(m_strServerAddress);
	}
	// IP주소를 입력하지 않으면 기본 IP 192.168.0.11로 연결
	else
	{
		m_strServerAddress = "192.168.0.11";
	}

	// SOCKET m_hSocket는 CAsyncSocket 클래스에 있는 변수

	if(!dataSocket.Create())
	{
		AddMessage("소켓 생성 실패");
		return;
	}

	// IP와 포트로 연결에 실패하는 경우
	if(!dataSocket.Connect(m_strServerAddress, PORT))
	{
		dataSocket.Close();
		AddMessage("서버 접속 실패");
	}
	// 연결 성공시 연결된 IP와 포트번호 출력
	else
	{
		AddMessage("서버에 접속 됐습니다.");
		
		// 접속끊기 버튼 활성화, 서버접속 버튼 비활성화
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(FALSE);
		// 보내기 버튼 활성화, 메세지 입력창에 포커스를 줌
		GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_SEND_DATA)->SetFocus();
	}
}

void CKatalkPCLinkerDlg::OnBnClickedButtonStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// dataSocket이 유효한 소켓인 경우에 종료 절차를 거침
	if(dataSocket.m_hSocket != INVALID_SOCKET)
	{
		dataSocket.Close();
		AddMessage("서버와의 접속을 종료합니다.");
		// 접속끊기 버튼 비활성화, 서버접속 버튼 활성화, 보내기 버튼 비활성화
		GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(FALSE);
	}
	else
	{
		AddMessage("이미 종료되었습니다.");
		return;
	}
}


void CKatalkPCLinkerDlg::OnBnClickedButtonSend()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	
	// 입력한 메세지를 strSend에 할당
	UpdateData();
	CString strSend;
	strSend.Format("%s", m_strSendData);

	// Ansi를 UTF8로 변경하고 CString으로 리턴된 것을 전송
	sendMsg(strSend);
	
	GetDlgItem(IDC_EDIT_SEND_DATA)->SetFocus();
	m_strSendData = "";
	UpdateData(FALSE);
}

// 키보드 제어를 미리 가로채서 처리해주는 메소드(재정의함)
BOOL CKatalkPCLinkerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	
	// 키가 눌렸을때
	if(pMsg->message == WM_KEYDOWN)
	{
		// 엔터키가 눌렸으면
		if(pMsg->wParam == VK_RETURN)
		{
			// 포커스가 메세지 입력창에 있으면
			
		}
		// ESC가 눌리면
		else if(pMsg->wParam == VK_ESCAPE)
		{
		
		}

		// 방향키가 눌리면 각각의 방향 명령 전송
		switch(pMsg->wParam){
		case VK_RETURN:
			if(GetFocus()->GetDlgCtrlID() == IDC_EDIT_SEND_DATA)
			{
				// 메세지 전송 수행
				OnBnClickedButtonSend();
				// Enter키 입력시 원래 창이 종료되므로 이 현상을 막기위해 return TRUE를 해줘야됨
				return TRUE;
			}
			else
			{
				// 그냥 엔터만 눌리면 엔터 명령 전송
				OnBnClickedEnter();
				return TRUE;
			}
		case VK_ESCAPE:
			// 뒤로가기 명령 전송
			OnBnClickedBack();
			// ESC키는 원래 종료를 하기때문에 이 현상을 막기위해 retrun TRUE를 해줘야됨
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

// 왼쪽 에디트 컨트롤에 메시지를 출력해주는 메소드
void CKatalkPCLinkerDlg::AddMessage(CString strMsg)
{

	UpdateData();
	
	m_strListData += strMsg + "\r\n";

	UpdateData(FALSE);

	m_edListData.LineScroll(m_edListData.GetLineCount());
}

// 서버로부터 온 메세지를 받아서 에디드 컨트롤에 출력
LRESULT CKatalkPCLinkerDlg::OnReceiveData(WPARAM wParam, LPARAM lParam)
{
	char Rcvdata[MAXLINE];

	CDataSocket* pDataSocket = (CDataSocket*)wParam;

	pDataSocket->Receive(Rcvdata, sizeof(Rcvdata));

	CString strMsg = Rcvdata;

	AddMessage(strMsg);

	return 0;
}

// 서버와의 연결 종료 처리
LRESULT CKatalkPCLinkerDlg::OnCloseSocket(WPARAM wParam, LPARAM lParam)
{
	dataSocket.Close();

	AddMessage("서버가 연결을 종료 했습니다.");
	// 접속끊기 버튼 비활성화, 서버접속 버튼 활성화, 보내기 버튼 비활성화
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_CONNECT)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(FALSE);

	return 0;
}

// Ansi를 UTF8 형식으로 변환하여 CString으로 리턴
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
		AddMessage("메세지 전송 실패");
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
// ↑ 눌렀을때 처리
void CKatalkPCLinkerDlg::OnBnClickedUp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	inputDirectioinKey(INPUT_UP);
}

// ↓ 눌렀을때 처리
void CKatalkPCLinkerDlg::OnBnClickedDown()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	inputDirectioinKey(INPUT_DOWN); 
}

// ← 눌렀을때 처리
void CKatalkPCLinkerDlg::OnBnClickedLeft()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	inputDirectioinKey(INPUT_LEFT);
}

// → 눌렀을때 처리
void CKatalkPCLinkerDlg::OnBnClickedRight()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	inputDirectioinKey(INPUT_RIGHT);
}

// 엔터키 명령을 전송할때 처리
void CKatalkPCLinkerDlg::OnBnClickedEnter()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	inputDirectioinKey(INPUT_ENTER);
}

// ESC 명령을 전송할때 처리
void CKatalkPCLinkerDlg::OnBnClickedBack()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	inputDirectioinKey(INPUT_BACK);
}

// 응용프로그램을 선택할때 처리하는 부분
void CKatalkPCLinkerDlg::OnCbnSelchangeOpenpack()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nIndex = selOpenPack.GetCurSel();
	if(nIndex == -1){
		return;
	}

	CString str, cmd;
	selOpenPack.GetLBText(nIndex, str);

	// 카톡을 선택하면
	if(str == "카톡")
	{
		// 카톡을 실행하라는 명령을 전송
		cmd.Format("%s", "\\\\OPENPACK_com.kakao.talk");

		AddMessage("카카오톡 실행");	
	}
	// 라인을 선택하면
	else if(str == "라인")
	{
		// 라인을 실행하라는 명령을 전송
		cmd.Format("%s", "\\\\OPENPACK_jp.naver.line.android");

		AddMessage("라인 실행");	
	}
	

	GetDlgItem(IDC_IPADDRESS)->SetFocus();
	GetDlgItem(IDC_IPADDRESS)->SendMessage(WM_KILLFOCUS, NULL); 
	sendMsg(cmd);
	UpdateData(FALSE);
}
