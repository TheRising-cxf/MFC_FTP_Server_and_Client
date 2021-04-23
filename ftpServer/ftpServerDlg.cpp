
// ftpServerDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "ftpServer.h"
#include "ftpServerDlg.h"
#include "afxdialogex.h"
#include "Account.h"
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CftpServerDlg 对话框



CftpServerDlg::CftpServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FTPSERVER_DIALOG, pParent)
	, strPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CftpServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, ServerIP);
	DDX_Text(pDX, IDC_EDIT1, strPort);
	DDX_Control(pDX, IDC_LIST2, m_accountList);
	DDX_Control(pDX, IDC_LIST1, m_serverInfo);
}

BEGIN_MESSAGE_MAP(CftpServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CftpServerDlg::OnStartServer)
	ON_BN_CLICKED(IDC_BUTTON2, &CftpServerDlg::OnStopServer)
	ON_BN_CLICKED(IDC_BUTTON4, &CftpServerDlg::OnEditUsr)
	ON_BN_CLICKED(IDC_BUTTON5, &CftpServerDlg::OnAddUsr)
	ON_BN_CLICKED(IDC_BUTTON3, &CftpServerDlg::OnDeleteUsr)
END_MESSAGE_MAP()


// CftpServerDlg 消息处理程序

BOOL CftpServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CAccount fixAccount;
	fixAccount.userName = "admin";
	fixAccount.password = "admin";
	fixAccount.directory = "c:\\";
	m_AccountArray.Add(fixAccount);
	m_accountList.AddString(fixAccount.userName);

	fixAccount.userName = "usr";
	fixAccount.password = "usr";
	fixAccount.directory = "c:\\";
	m_AccountArray.Add(fixAccount);
	m_accountList.AddString(fixAccount.userName);

	fixAccount.userName = "anonymous";
	fixAccount.password = "";
	fixAccount.directory = "C:\\";
	m_AccountArray.Add(fixAccount);
	m_accountList.AddString(fixAccount.userName);
	CStatic* m_pStatic1 = (CStatic*)GetDlgItem(IDC_EDIT1);
	m_pStatic1->SetWindowText("21");
	char szHostName[MAX_PATH + 1];
	gethostname(szHostName, MAX_PATH); //得到计算机名
	hostent *p = gethostbyname(szHostName); //从计算机名得到主机信息
	char *pIP1 = inet_ntoa(*(in_addr *)p->h_addr_list[0]);//将32位IP转化为字符串IP
	DWORD dwip = inet_addr(pIP1);
	unsigned char *pIP = (unsigned char*)&dwip;
	ServerIP.SetAddress(*pIP, *(pIP + 1), *(pIP + 2), *(pIP + 3));//控件
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CftpServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CftpServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CftpServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CftpServerDlg::OnStartServer()
{
	// TODO: 在此添加控件通知处理程序代码
	BYTE nFild[4];
	CString sIP;
	this->UpdateData();
	if (ServerIP.IsBlank()) {
		AfxMessageBox("请配置IP地址");
		return;
	}
	if (strPort.IsEmpty()) {
		AfxMessageBox("请配置服务器端口");
		return;
	}
	ServerIP.GetAddress(nFild[0], nFild[1], nFild[2], nFild[3]);
	sIP.Format("%d.%d.%d.%d", nFild[0], nFild[1], nFild[2], nFild[3]);
	UINT i = atoi(strPort);
	m_server.ServerConfigInfo(&m_AccountArray, nFild, i);
	AfxBeginThread(ServerThread, LPVOID(&m_server));
	m_serverInfo.AddString("FTP服务器启动成功");
	m_serverInfo.AddString("IP" + sIP + "端口" + strPort);
	m_serverInfo.AddString("服务运行中");
}


void CftpServerDlg::OnStopServer()
{
	// TODO: 在此添加控件通知处理程序代码
	m_server.m_bStopServer = true;
	closesocket(m_server.sDialog);
	closesocket(m_server.sListen);
	m_serverInfo.AddString("服务已停止");
}


void CftpServerDlg::OnEditUsr()
{
	// TODO: 在此添加控件通知处理程序代码
	int selectedItemIndex = m_accountList.GetCurSel();
	int accountToBeEditedIndex = 0;
	if (selectedItemIndex == -1) {
		if (m_accountList.GetCount() != 0) {
			AfxMessageBox("请选择要编辑的用户");

		}
		else {
			AfxMessageBox("用户列表为空");
		}
	}
	else {
		CString account;
		m_accountList.GetText(selectedItemIndex, account);
		int length = (int)m_AccountArray.GetSize();
		for (int i = 0; i < length; i++) {
			if (m_AccountArray[i].userName == account) {
				accountToBeEditedIndex = i;
				break;
			}
		}
		Account dlg;
		dlg.userName = m_AccountArray[accountToBeEditedIndex].userName;
		dlg.password = m_AccountArray[accountToBeEditedIndex].password;
		dlg.dirtory = m_AccountArray[accountToBeEditedIndex].directory;
		while (dlg.DoModal() == IDOK) {
			if (dlg.userName == "" || (dlg.password == ""&& dlg.userName != "anonymous") || dlg.dirtory == "") {
				AfxMessageBox("用户名、密码或FTP根目录不能为空");
				continue;
			}
			else {
				int length = (int)m_AccountArray.GetSize();
				BOOL isAccountExit = false;
				for (int i = 0; i < length; i++) {
					if (m_AccountArray[i].userName == account && i!=selectedItemIndex) {
						AfxMessageBox("该用户已存在");
						isAccountExit = true;
						break;
					}
				}
				if (isAccountExit) {
					continue;
				}
				else if(AfxMessageBox("你确定要修改吗?",4+48) == 6){
					m_AccountArray[accountToBeEditedIndex].userName = dlg.userName;
					m_AccountArray[accountToBeEditedIndex].password = dlg.password;
					m_AccountArray[accountToBeEditedIndex].directory = dlg.dirtory;
					m_accountList.DeleteString(selectedItemIndex);
					m_accountList.AddString(m_AccountArray[accountToBeEditedIndex].userName);
					break;
				}
			}
		}
		account.Empty();
		account.FreeExtra();
		delete dlg;
	}
}


void CftpServerDlg::OnAddUsr()
{
	// TODO: 在此添加控件通知处理程序代码
	Account dlg;
	dlg.userName = "";
	dlg.password = "";
	dlg.dirtory = "";
	while (dlg.DoModal() == IDOK) {
		if (dlg.userName == "" || (dlg.password == ""&& dlg.userName != "anonymous") || dlg.dirtory == "") {
			AfxMessageBox("用户名、密码或FTP根目录不能为空");
			continue;
		}
		else {
			int length = (int)m_AccountArray.GetSize();
			BOOL isAccountExit = false;
			for (int i = 0; i < length; i++) {
				if (m_AccountArray[i].userName == dlg.userName) {
					AfxMessageBox("该用户已存在");
					isAccountExit = true;
					break;
				}
			}
			if (isAccountExit) {
				continue;
			}
			else if (AfxMessageBox("你确定要修改吗?", 4 + 48) == 6) {
				CAccount newAccoumt;

				newAccoumt.userName = dlg.userName;
				newAccoumt.password = dlg.password;
				newAccoumt.directory = dlg.dirtory;
				m_AccountArray.Add(newAccoumt);
				m_accountList.AddString(newAccoumt.userName);
				break;
			}
		}
	}
	delete dlg;
}


void CftpServerDlg::OnDeleteUsr()
{
	// TODO: 在此添加控件通知处理程序代码
	int selectedItemIndex = m_accountList.GetCurSel();
	int accountToBeEditedIndex = 0;
	if (selectedItemIndex == -1) {
		if (m_accountList.GetCount() != 0) {
			AfxMessageBox("请选择要删除的用户");

		}
		else {
			AfxMessageBox("用户列表为空");
		}
	}
	else {
		CString account;
		m_accountList.GetText(selectedItemIndex, account);
		int length = (int)m_AccountArray.GetSize();
		for (int i = 0; i < length; i++) {
			if (m_AccountArray[i].userName == account) {
				m_AccountArray.RemoveAt(i);
				length = (int)m_AccountArray.GetSize();
				break;
			}
		}
		m_accountList.DeleteString(selectedItemIndex);
		account.Empty();
		account.FreeExtra();
	}
}
