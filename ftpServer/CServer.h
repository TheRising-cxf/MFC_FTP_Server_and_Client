#pragma once
#include "afxinet.h"
#include "string.h"
#include "stdlib.h"

#define WSA_RECV       0
#define WSA_SEND       1
#define DATA_BUFSIZE      8192
#define MAX_NAME_LEN     128
#define MAX_PWD_LEN     128
#define MAX_RESP_LEN     1024
#define MAX_REQ_LEN   256
#define MAX_ADDR_LEN   80
#define RTP_PORT      21
#define DATA_FTP_PORT     20
#define OPENING_AMODE   150
#define CMD_OK   200
#define OS_TYPE 215
#define FTP_QUIT 221
#define TRANS_COMPLETE  226
#define PASSIVE_MODE    227
#define LOGGED_IN 230
#define DIR_CHANGED   250
#define CURR_DIR     257

#define USER_OK     331
#define FILE_EXIST     350
#define ACCESS_DENY    450
#define NOT_IMPLEMENT   502
#define REPLY_MARKER    504
#define LOGIN_FAILED    530
#define CANNOT_FIND    550
#define MAX_FILE_NUM    1024
#define MODE_PORT    0
#define MODE_PASV    1
#define PORT_BIND    1821
typedef struct {
	CHAR bufferRecv[DATA_BUFSIZE];
	CHAR buffsrSend[DATA_BUFSIZE];
	WSABUF    wasBuf;
	SOCKET    socket;
	WSAOVERLAPPED  wsaOverLapped;
	DWORD     dwBytesSend;
	DWORD     dwBytesRecv;
	int SocketStatus;
	BOOL userLoggedIn;
	CHAR   userCrrentDir[MAX_PATH];
	CHAR    userRootDir[MAX_PATH];
}SOCKET_INF,*LPSOCKET_INF;
typedef struct {
	CHAR szFileName[MAX_PATH];
	DWORD   dwFileAttributes;
	FILETIME ftCreationTime;
	FILETIME ftLastAccessTime;
	FILETIME ftLastWriteTime;
	DWORD    nFileSizeHigh;
	DWORD    nFileSizeLow;
}FILE_INF,*LPFILE_INF;

struct CAccount {
	CString userName;
	CString password;
	CString directory;
};
typedef CArray<CAccount, CAccount& > AccountArray;
  
extern UINT ServerThread(LPVOID lpParameter);
class CServer
{
public:
	CServer(void);
	~CServer(void);
public:
	AccountArray m_RegisteredAccount;
	BYTE    IpFild[4];
	UINT m_serverPort;
	BOOL m_bStopServer;
	SOCKET sListen;
	SOCKET  sDialog;
public: 
	void DivideRequest(char* request, char* command, char* cmdtail);
	void ServerConfigInfo(AccountArray *accountArray, BYTE nFild[], UINT port);
	BOOL SendWelcomeMsg(SOCKET s);
	int Login(LPSOCKET_INF pSocketInfo);
	int DealWithCommand(LPSOCKET_INF socketInfo);
	int SendACK(LPSOCKET_INF socketInfo);
	int RecvRequest(LPSOCKET_INF socketInfo);
	int ConvertCommaAddrToDotAddr(char*commAddr, LPDWORD pdwIpAddr, LPWORD pwPort);
	int ConvertDotAddrToCommAddr(char *dotAddr, WORD wPort, char* commAddr);
	int DataConnect(SOCKET& s, DWORD dwIp, WORD wPort, int nMode);
	char* GetLocalAddress();
	SOCKET AcceptConnectRequest(SOCKET &s);
	int FileListToString(char* buffer, UINT nBufferSize, BOOL isListCommand);
	int GetFileList(LPFILE_INF fileInfo, int arraySize, const char* searchPath);
	int DataSend(SOCKET s, char* buff, int buffSize);
	int CombindFileNameSize(const char* fileName, char * fileNameSize);
	int ReadFileToBUffer(const char*szFileName, char* buff, int fileSize);
	int DataRecv(SOCKET s, const char * fileName);
	void GetRalativeDirectory(char *currDir, char* rootDir, char* ralaDir);
	void GetAbsoluteDirectory(char * dir, char*currentDir, char *userCurrDir);
	void HostToNet(char* path);
	void NetToHost(char* path);
	int TryDeleteFile(char* deletedPath);
	BOOL IsPathExist(char* path);

};  

