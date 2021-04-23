
// SelfFtpUpDownLoaderDlg.h: 头文件
//

#pragma once
#include "afxinet.h"

// CSelfFtpUpDownLoaderDlg 对话框
class CSelfFtpUpDownLoaderDlg : public CDialogEx
{
// 构造
public:
	CSelfFtpUpDownLoaderDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SELFFTPUPDOWNLOADER_DIALOG };
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
	CString strUsr;
	CString strPwd;
	CButton m_noName;
	CEdit m_usr;
	CEdit m_pwd;
	afx_msg void OnNoName();
	CEdit m_port;
	CButton m_connect;
	CButton m_disConnect;
	CInternetSession* pInternetSession;
	CFtpConnection* pFtpConnect;
	bool bConnect;
	afx_msg void OnConnect();
	BOOL ConnectFtp();
	void UpDateDir();
	CListBox m_list;
	afx_msg void OnEnterDir();
	afx_msg void OnGoBack();
	afx_msg void OnDownLoad();
	afx_msg void OnDelete();
	afx_msg void OnDisConnect();
	afx_msg void OnUpLoad();
};
