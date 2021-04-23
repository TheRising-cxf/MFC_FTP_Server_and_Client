
// ftpServerDlg.h: 头文件
//

#pragma once
#include "stdlib.h"
#include "CServer.h"

// CftpServerDlg 对话框
class CftpServerDlg : public CDialogEx
{
// 构造
public:
	CftpServerDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FTPSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CIPAddressCtrl ServerIP;
	CString strPort;
	CListBox m_accountList;
	CListBox m_serverInfo;
	AccountArray m_AccountArray;
	CServer m_server;
	afx_msg void OnStartServer();
	afx_msg void OnStopServer();
	afx_msg void OnEditUsr();
	afx_msg void OnAddUsr();
	afx_msg void OnDeleteUsr();
};
