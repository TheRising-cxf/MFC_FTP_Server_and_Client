
// SelfFtpUpDownLoaderDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SelfFtpUpDownLoader.h"
#include "SelfFtpUpDownLoaderDlg.h"
#include "afxdialogex.h"

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


// CSelfFtpUpDownLoaderDlg 对话框



CSelfFtpUpDownLoaderDlg::CSelfFtpUpDownLoaderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SELFFTPUPDOWNLOADER_DIALOG, pParent)
	, strPort(_T(""))
	, strUsr(_T(""))
	, strPwd(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSelfFtpUpDownLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS1, ServerIP);
	DDX_Text(pDX, IDC_EDIT1, strPort);
	DDX_Text(pDX, IDC_EDIT2, strUsr);
	DDX_Text(pDX, IDC_EDIT3, strPwd);
	DDX_Control(pDX, IDC_CHECK1, m_noName);
	DDX_Control(pDX, IDC_EDIT2, m_usr);
	DDX_Control(pDX, IDC_EDIT3, m_pwd);
	DDX_Control(pDX, IDC_EDIT1, m_port);
	DDX_Control(pDX, IDC_BUTTON1, m_connect);
	DDX_Control(pDX, IDC_BUTTON2, m_disConnect);
	DDX_Control(pDX, IDC_LIST1, m_list);
}

BEGIN_MESSAGE_MAP(CSelfFtpUpDownLoaderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHECK1, &CSelfFtpUpDownLoaderDlg::OnNoName)
	ON_BN_CLICKED(IDC_BUTTON1, &CSelfFtpUpDownLoaderDlg::OnConnect)
	ON_BN_CLICKED(IDC_BUTTON3, &CSelfFtpUpDownLoaderDlg::OnEnterDir)
	ON_BN_CLICKED(IDC_BUTTON4, &CSelfFtpUpDownLoaderDlg::OnGoBack)
	ON_BN_CLICKED(IDC_BUTTON7, &CSelfFtpUpDownLoaderDlg::OnDownLoad)
	ON_BN_CLICKED(IDC_BUTTON6, &CSelfFtpUpDownLoaderDlg::OnDelete)
	ON_BN_CLICKED(IDC_BUTTON2, &CSelfFtpUpDownLoaderDlg::OnDisConnect)
	ON_BN_CLICKED(IDC_BUTTON5, &CSelfFtpUpDownLoaderDlg::OnUpLoad)
END_MESSAGE_MAP()


// CSelfFtpUpDownLoaderDlg 消息处理程序

BOOL CSelfFtpUpDownLoaderDlg::OnInitDialog()
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

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSelfFtpUpDownLoaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSelfFtpUpDownLoaderDlg::OnPaint()
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
HCURSOR CSelfFtpUpDownLoaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSelfFtpUpDownLoaderDlg::OnNoName()
{
	// TODO: 在此添加控件通知处理程序代码
	int isCheck = m_noName.GetCheck();
	if (isCheck) {
		m_usr.EnableWindow(false);
		m_pwd.EnableWindow(false);
		m_usr.SetWindowTextA("anonymous");
		m_pwd.SetWindowTextA("");
		UpdateData();
		if (!ServerIP.IsBlank() && !strPort.IsEmpty()) {
			m_connect.EnableWindow(true);
		}
		else {
			m_usr.EnableWindow(true);
			m_pwd.EnableWindow(true);
			m_usr.SetWindowTextA("");
			m_pwd.SetWindowTextA("");
			m_connect.EnableWindow(false);
		}
	}
	else {
		m_usr.EnableWindow(true);
		m_pwd.EnableWindow(true);
		m_connect.EnableWindow(true);
	}
}


void CSelfFtpUpDownLoaderDlg::OnConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	this->ConnectFtp();
	if (bConnect ) {
		this->UpDateDir();
	}

}

BOOL CSelfFtpUpDownLoaderDlg::ConnectFtp()
{
	BYTE nFild[4];
	UpdateData();
	if (ServerIP.IsBlank()) {
		AfxMessageBox("请指定IP地址！");
		return false;
	}
	if (strPort.IsEmpty()) {
		AfxMessageBox("请指定连接端口！");
		return false;
	}
	if (strUsr.IsEmpty()) {
		AfxMessageBox("请填写用户名！");
		return false;
	}
	if (strPwd.IsEmpty() && strUsr != "anonymous") {
		AfxMessageBox("请输入密码！");
		return false;
	}
	ServerIP.GetAddress(nFild[0], nFild[1], nFild[2], nFild[3]);
	CString sip;
	sip.Format("%d.%d.%d.%d", nFild[0], nFild[1], nFild[2], nFild[3]);
		pInternetSession = new CInternetSession("MR", INTERNET_OPEN_TYPE_PRECONFIG);
		try {
			pFtpConnect = pInternetSession->GetFtpConnection(sip, strUsr, strPwd, atoi(strPort));
			bConnect = true;
		}
		catch (CInternetException *pEx) {
			TCHAR szErr[1024];
			pEx->GetErrorMessage(szErr, 1024);
			AfxMessageBox(szErr);
			pEx->Delete();
			bConnect = false;
		}
		return true;
}

void CSelfFtpUpDownLoaderDlg::UpDateDir()
{
	m_list.ResetContent();
	CFtpFileFind ftpFind(pFtpConnect);
	BOOL bfind = ftpFind.FindFile(NULL);
	while (bfind) {
		bfind = ftpFind.FindNextFile();
		CString strPath;
		if (!ftpFind.IsDirectory()) {
			strPath = ftpFind.GetFileName();
			m_list.AddString(strPath);
		}
		else {
			strPath = ftpFind.GetFilePath();
			m_list.AddString(strPath);
		}
	}
}


void CSelfFtpUpDownLoaderDlg::OnEnterDir()
{
	// TODO: 在此添加控件通知处理程序代码
	CString selfFile;

	m_list.GetText(m_list.GetCurSel(), selfFile);
	if (!selfFile.IsEmpty()) {
		pInternetSession->Close();
		this->ConnectFtp();
		CString strDir;
		pFtpConnect->GetCurrentDirectory(strDir);
		strDir += selfFile;
		pFtpConnect->SetCurrentDirectory(strDir);
		this->UpDateDir();
	}
}


void CSelfFtpUpDownLoaderDlg::OnGoBack()
{
	// TODO: 在此添加控件通知处理程序代码
	CString strDir;
	pFtpConnect->GetCurrentDirectory(strDir);
	int pos;
	pos = strDir.ReverseFind('/');
	strDir = strDir.Left(pos);
	pInternetSession->Close();
	this->ConnectFtp();
	pFtpConnect->SetCurrentDirectory(strDir);
	this->UpDateDir();
}


void CSelfFtpUpDownLoaderDlg::OnDownLoad()
{
	// TODO: 在此添加控件通知处理程序代码
	CString selfFile;
	m_list.GetText(m_list.GetCurSel(), selfFile);
	if (!selfFile.IsEmpty()) {
		CFileDialog file(false, NULL, selfFile, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "所有文件(*.*)|*.*|", this);
		if (file.DoModal() == IDOK) {
			CString strName;
			strName = file.GetFileName();
			CString strDir;
			pFtpConnect->GetCurrentDirectory(strDir);
			pFtpConnect->GetFile(selfFile, strName);
			pInternetSession->Close();
			this->ConnectFtp();
			pFtpConnect->SetCurrentDirectory(strDir);
			this->UpDateDir();
			AfxMessageBox("下载成功");
		}
	}
}


void CSelfFtpUpDownLoaderDlg::OnDelete()
{
	// TODO: 在此添加控件通知处理程序代码
	CString selfFile;
	m_list.GetText(m_list.GetCurSel(), selfFile);
	if (!selfFile.IsEmpty()) {
		if (AfxMessageBox("确定要删除吗？",4 + 48) == 6) {
			pFtpConnect->Remove(selfFile);
			CString strDir;
			pFtpConnect->GetCurrentDirectory(strDir);
			pInternetSession->Close();
			this->ConnectFtp();
			pFtpConnect->SetCurrentDirectory(strDir);
			this->UpDateDir();
		}
	}

}


void CSelfFtpUpDownLoaderDlg::OnDisConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	pInternetSession->Close();
	m_list.ResetContent();
	m_list.AddString("连接已断开");

}


void CSelfFtpUpDownLoaderDlg::OnUpLoad()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	CString strName;
	CFileDialog file(true, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, "所有文件(*.*)|*.*|", this);
	if (file.DoModal() == IDOK) {
		str = file.GetPathName();
		strName = file.GetFileName();
	}if (bConnect) {
		CString strDir;
		pFtpConnect->GetCurrentDirectory(strDir);
		BOOL bPut = pFtpConnect->PutFile(LPCTSTR(str),LPCTSTR(strName));
		if (bPut) {
			pInternetSession->Close();
			this->ConnectFtp();
			pFtpConnect->SetCurrentDirectory(strDir);
			this->UpDateDir();
			AfxMessageBox("上传成功");
		}
	}
}