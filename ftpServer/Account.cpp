// Account.cpp: 实现文件
//

#include "pch.h"
#include "ftpServer.h"
#include "Account.h"
#include "afxdialogex.h"


// Account 对话框

IMPLEMENT_DYNAMIC(Account, CDialogEx)

Account::Account(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, userName(_T(""))
	, password(_T(""))
	, dirtory(_T(""))
{

}

Account::~Account()
{
}

void Account::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, userName);
	DDX_Text(pDX, IDC_EDIT3, password);
	DDX_Text(pDX, IDC_EDIT2, dirtory);
}


BEGIN_MESSAGE_MAP(Account, CDialogEx)
END_MESSAGE_MAP()


// Account 消息处理程序
