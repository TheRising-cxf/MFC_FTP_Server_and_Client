#pragma once


// Account 对话框

class Account : public CDialogEx
{
	DECLARE_DYNAMIC(Account)

public:
	Account(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Account();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString userName;
	CString password;
	CString dirtory;
};
