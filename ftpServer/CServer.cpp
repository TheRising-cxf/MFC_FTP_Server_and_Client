#include "pch.h"
#include "CServer.h"
#include "ftpServer.h"
#pragma warning(disable:4996)
DWORD g_dwTotalEventAmount = 0;
DWORD g_index;
WSAEVENT g_EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INF g_SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION g_CriticalSection;
UINT ProccessThreadIO(LPVOID LpParam);

CServer::CServer(void)
{
	m_bStopServer = false;
}

CServer::~CServer(void)
{
}

void CServer::DivideRequest(char * request, char * command, char* cmdtail)
{
	int i = 0;
	int len = (int)strlen(request);
	command[0] = '\0';
	for (i = 0; i < len&&request[i] != ' '&&request[i] != '\r'&&request[i] != '\n'; i++) {
		command[i] = request[i];

	}
	command[i] = '\0';
	_strupr(command);
	int index = 0;
	for (i++; i < len && request[i] != '\r'&&request[i] != '\n'; i++) {
		cmdtail[index] = request[i];
		index++;
		if (index>1 && request[i] == 's'&& request[i-1] == '%') {
			index-=2;
		}
	}
	cmdtail[index] = '\0';
}

void CServer::ServerConfigInfo(AccountArray * accountArray, BYTE nFild[], UINT port)
{
	int size = (int)(*accountArray).GetCount();
	for (int i = 0; i < size; i++) {
		m_RegisteredAccount.Add((*accountArray)[i]);
	}
	for (int i = 0; i < 4; i++) {
		IpFild[i] = nFild[i];
	}
	m_serverPort = port;
	m_bStopServer = FALSE;
}
BOOL CServer::SendWelcomeMsg(SOCKET s)
{
	char* welcomeInfo = "220 Server ready...\r\n";
	if (send(s, welcomeInfo, (int)strlen(welcomeInfo), 0) == SOCKET_ERROR) {
		AfxMessageBox(_T("Failed in sending welcome msg.POS:CServer::SendWelcomeMsg"));
		return FALSE;
	}
	return TRUE;
}
int CServer::Login(LPSOCKET_INF pSocketInfo)
{
	static char username[MAX_NAME_LEN];
	static char password[MAX_PWD_LEN];
	int loginResult = 0;
	if (strstr(strupr(pSocketInfo->bufferRecv), "USER") || strstr(strlwr(pSocketInfo->bufferRecv), "user")) {
		sprintf(username, "%s", pSocketInfo->bufferRecv + strlen("user") + 1);
		strtok(username, "\r\n");
		sprintf(pSocketInfo->buffsrSend, "%s", "331 User name ok,need password.\r\n");
		if (SendACK(pSocketInfo) == -1) {
			AfxMessageBox(_T("Failed in sending ACK。POS:CServer::Login()"));
			return -1;
		}
		return USER_OK;
	}
	if (strstr(strupr(pSocketInfo->bufferRecv), "PASS") || strstr(strlwr(pSocketInfo->bufferRecv), "pass")) {
		sprintf(password, "%s", pSocketInfo->bufferRecv + strlen("pass") + 1);
		strtok(password, "\r\n");
		int size = (int)m_RegisteredAccount.GetCount();
		BOOL isAccountExisted = FALSE;
		CString user = (CString)username;
		CString pwd = (CString)password;
		for (int i = 0; i < size; i++) {
			if ((m_RegisteredAccount[i].userName.CompareNoCase(user) == 0 && m_RegisteredAccount[i].password.CompareNoCase(pwd) == 0)
				|| (m_RegisteredAccount[i].userName.CompareNoCase(user) == 0 && user == "ANONYMOUS")) {
				isAccountExisted = TRUE;
				strcpy(pSocketInfo->userCrrentDir, m_RegisteredAccount[i].directory);
				strcpy(pSocketInfo->userRootDir, m_RegisteredAccount[i].directory);
				break;
			}
		}
		if (isAccountExisted) {
			sprintf(pSocketInfo->buffsrSend, "230 User:%s loged in.proceed.\r\n", username);
			loginResult = LOGGED_IN;
		}
		else {
			sprintf(pSocketInfo->buffsrSend, "530 User:%s connot logged in,Wrong username or passwoed.Try again.\r\n", username);
			loginResult = LOGIN_FAILED;
		}
		if (SendACK(pSocketInfo) == -1) {
			AfxMessageBox(_T("Failed in sending ACK。POS:CServer::Login()"));
			return -1;
		}
	}

	return loginResult;
}
int CServer::DealWithCommand(LPSOCKET_INF socketInfo)
{
	static SOCKET sDialog = INVALID_SOCKET;
	static SOCKET sAccept = INVALID_SOCKET;
	static BOOL isPasvMode = FALSE;
	int operatinResult = 0;
	char command[20];
	char cmdtail[MAX_REQ_LEN];
	char currentDir[MAX_PATH];
	char relativeDir[128];
	const char* szOpeningAMode = "150 OPening ASCII mode data connection for";
	static DWORD dwIpAddr = 0;
	static WORD wPort = 0;
	DivideRequest(socketInfo->bufferRecv, command, cmdtail);
	if (command[0] == '\0') {
		return -1;
	}
	if (strstr(command, "PORT")) {
		if (ConvertCommaAddrToDotAddr(socketInfo->bufferRecv + strlen("PORT") + 1, &dwIpAddr, &wPort) == -1) {
			AfxMessageBox(_T("Failed in ConvertCommaAddrToDotAddr.COD PORT"));
			return -1;
		}
		sprintf(socketInfo->buffsrSend, "%s","200 PORT connect success.\r\n");
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed.CMD:PORT"), MB_OK | MB_ICONERROR);
			return -1;
		}
		isPasvMode = FALSE;
		return CMD_OK;
	}
	if (strstr(command, "PASV")) {
		if (DataConnect(sAccept, htonl(INADDR_ANY), PORT_BIND, MODE_PASV) == -1) {
			AfxMessageBox(_T("DataConnect() failed.CMD:PASV"), MB_OK | MB_ICONERROR);
			return -1;
		}
		char szCommaAddress[40];
		if (ConvertDotAddrToCommAddr(GetLocalAddress(), PORT_BIND, szCommaAddress) == -1) {
			AfxMessageBox(_T("ConvertDotAddrToCommAddr() failed.CMD:PASV"), MB_OK | MB_ICONERROR);
			return -1;
		}
		sprintf(socketInfo->buffsrSend, "%s ", "227 Entering Pasive Mode success.\r\n");
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed.CMD:PASV"), MB_OK | MB_ICONERROR);
			return -1;
		}
		isPasvMode = TRUE;
		return CMD_OK;
	}
	if (strstr(command, "NLST")||strstr(command,"LIST")) {
		if (isPasvMode) {
			sDialog = AcceptConnectRequest(sAccept);
		}
		if (!isPasvMode) {
			sprintf(socketInfo->buffsrSend, "150 Opening ASCII mode data connection for directory list.\r\n");
		}
		else {
			sprintf(socketInfo->buffsrSend, "125 Data connect already open Transfer starting.\r\n");
		}
		BOOL isListCommand = strstr(command, "LIST") ? TRUE : FALSE;
		char buffer[DATA_BUFSIZE];
		UINT nStrLen = FileListToString(buffer, sizeof(buffer), isListCommand);
		if (!isPasvMode) {
			if (DataConnect(sAccept, dwIpAddr, wPort, MODE_PORT) == -1) {
				AfxMessageBox(_T("DataConnect() failed.CMD:NLST/LIST"), MB_OK | MB_ICONERROR);
				return -1;
			}
			if (SendACK(socketInfo) == -1) {
				AfxMessageBox(_T("SendACK() failed.CMD:NLST/LIST"), MB_OK | MB_ICONERROR);
				return -1;
			}
			if (DataSend(sAccept, buffer, nStrLen) == -1) {
				AfxMessageBox(_T("DataSend() failed.CMD:NLST/LIST"), MB_OK | MB_ICONERROR);
				return -1;
			}
			closesocket(sAccept);
		}
		else {
			DataSend(sDialog, buffer, nStrLen);
			closesocket(sDialog);
		}
		sprintf(socketInfo->buffsrSend, "%s", "226 Transfer complete.\r\n");
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed.CMD:NLST/LIST"), MB_OK | MB_ICONERROR);
			return -1;
		}
		return TRANS_COMPLETE;
	}
	if (strstr(command, "RETR")) {
		if (isPasvMode) {
			sDialog = AcceptConnectRequest(sAccept);
		}
		char fileNameSize[MAX_PATH];
		int nFileSize = CombindFileNameSize(cmdtail, fileNameSize);
		sprintf(socketInfo->buffsrSend, "%s%s.\r\n", szOpeningAMode, fileNameSize);
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed.CMD:RETR"), MB_OK | MB_ICONERROR);
			return -1;
		}
		char* buffer = new char[nFileSize];
		if (buffer == NULL) {
			AfxMessageBox(_T("分配缓存失败!\n"));
			return -1;
		}
		if (ReadFileToBUffer(cmdtail, buffer, nFileSize) == nFileSize) {
			Sleep(10);
			if (isPasvMode) {
				DataSend(sDialog, buffer, nFileSize);
				closesocket(sDialog);
			}
			else {
				if (DataConnect(sAccept, dwIpAddr, wPort, MODE_PORT) == -1) {
					return -1;
				}
				DataSend(sAccept, buffer, nFileSize);
				closesocket(sAccept);
			}
		}
		if (buffer != NULL) {
			delete[] buffer;
		}
		sprintf(socketInfo->buffsrSend, "%s", "226 Transfer complete.\r\n");
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed.CMD:RETE"), MB_OK | MB_ICONERROR);
			return -1;
		}
		return TRANS_COMPLETE;
	}
	if (strstr(command, "STOR")) {
		if (isPasvMode) {
			sDialog = AcceptConnectRequest(sAccept);
		}
		if (cmdtail[0] == '\0') {
			return -1;
		}
		sprintf(socketInfo->buffsrSend, "%s%s.\r\n", szOpeningAMode, cmdtail);
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed.CMD:STOR"), MB_OK | MB_ICONERROR);
			return -1;
		}
		if (isPasvMode) {
			DataRecv(sDialog, cmdtail);
		}
		else {
			if (DataConnect(sAccept, dwIpAddr, wPort, MODE_PORT) == -1) {
				return -1;
			}
			DataRecv(sAccept, cmdtail);
		}
		
		sprintf(socketInfo->buffsrSend, "%s", "226 Transfer complete.\r\n");
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed.CMD:STOR"), MB_OK | MB_ICONERROR);
			return -1;
		}
		return TRANS_COMPLETE;

	}
	if (strstr(command, "QUIT")) {
		sprintf(socketInfo->buffsrSend, "%s", "221 Goodbye.\r\n");
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed.CMD:QUIT"), MB_OK | MB_ICONERROR);
			return -1;
		}
		return FTP_QUIT;
	}
	if (strstr(command, "XPWD") || strstr(command, "PWD")) {
		GetCurrentDirectory(MAX_PATH, currentDir);
		char tmpStr[128];
		GetRalativeDirectory(currentDir, socketInfo->userRootDir, tmpStr);
		HostToNet(tmpStr);
		sprintf(socketInfo->buffsrSend, "%s", "257 \""+ (CString)tmpStr +"\" is currrent dirsctor.\r\n");
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed.CMD:PWD"), MB_OK | MB_ICONERROR);
			return -1;
		}
		return CURR_DIR;
	}
	if (strstr(command, "CDUP")) {
		if (cmdtail == '\0') {
			strcpy(cmdtail, "\\");
		}
		char szDir[MAX_PATH];
		strcpy(szDir, "..");
		if (SetCurrentDirectory(szDir) == 0) {
			sprintf(socketInfo->buffsrSend, "%s", "550 'CWD command failed,'%s' no such path.\r\n", cmdtail);
			operatinResult = CANNOT_FIND;
		}
		else {
			GetCurrentDirectory(MAX_PATH, currentDir);
			relativeDir[0] = '\0';
			GetRalativeDirectory(currentDir, socketInfo->userRootDir, relativeDir);
			HostToNet(relativeDir);
			sprintf(socketInfo->buffsrSend, "%s", "250 'CWD command successful,'%s' is current dir.\r\n", relativeDir);
			operatinResult = DIR_CHANGED;
		}
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed CMD CDUP"));
			return -1;
		}
		return operatinResult;
	}
	if (strstr(command, "CWD")) {
		if (cmdtail == '\0') {
			strcpy(cmdtail, "\\");
		}
		GetAbsoluteDirectory(cmdtail, socketInfo->userRootDir, currentDir);
		if (strlen(currentDir) < strlen(socketInfo->userRootDir)) {
			sprintf(socketInfo->buffsrSend, "%s", "555 'CWD command failed,\"" + (CString)cmdtail + "\" User no power.\r\n");
		}
		else if (SetCurrentDirectory(currentDir)==0) {
			sprintf(socketInfo->buffsrSend, "%s", "555 'CWD command failed,'%s' no such path.\r\n", cmdtail);
			operatinResult = CANNOT_FIND;
		}
		else {
			strcpy(socketInfo->userCrrentDir, currentDir);
			relativeDir[0] = '\0';
			GetRalativeDirectory(currentDir, socketInfo->userRootDir, relativeDir);
			HostToNet(relativeDir);
			sprintf(socketInfo->buffsrSend, "%s", "250 'CWD command successful,\"" + (CString)relativeDir + "\" is current dir.\r\n", relativeDir);
			operatinResult = DIR_CHANGED;
		}
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed CMD CWD"));
			return -1;
		}
		return operatinResult;
	}
	if (strstr(command, "DELE")) {
		GetAbsoluteDirectory(cmdtail, socketInfo->userCrrentDir, currentDir);
		GetRalativeDirectory(currentDir, socketInfo->userRootDir, relativeDir);
		HostToNet(relativeDir);
		BOOL isDELE = TRUE;
		operatinResult = TryDeleteFile(currentDir);
		if (operatinResult == DIR_CHANGED) {
			sprintf(socketInfo->buffsrSend, "250 File '/%s' has been deleted\r\n", relativeDir);
		}
		else if(operatinResult == ACCESS_DENY) {
			sprintf(socketInfo->buffsrSend, "450 File '/%s' can't been deleted\r\n", relativeDir);
		}
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed CMD DELETE"));
			return -1;
		}
		return operatinResult;
	}
	if (strstr(command, "TYPE")) {
		if (cmdtail == '\0') {
			strcpy(cmdtail, "A");
		}
		sprintf(socketInfo->buffsrSend, "200 TYPE set to %s.\r\n", cmdtail);
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed CMD TYPE"));
			return -1;
		}
		return CMD_OK;
	}
	if (strstr(command, "REST")) {
		sprintf(socketInfo->buffsrSend, "504 Reply marker must be 0.\r\n");
		if (SendACK(socketInfo) == -1) {
			AfxMessageBox(_T("SendACK() failed CMD REST"));
			return -1;
		}
		return REPLY_MARKER;
	}
	sprintf(socketInfo->buffsrSend, "500 '%s' Unknown command.\r\n", command);
	if (SendACK(socketInfo) == -1) {
		AfxMessageBox(_T("SendACK() failed.CMD:otherwish"), MB_OK | MB_ICONERROR);
			return -1;
	}
	return operatinResult;
}
int CServer::SendACK(LPSOCKET_INF socketInfo)
{
	static DWORD dwSendBytes = 0;
	char errorMsg[128];
	socketInfo->SocketStatus = WSA_SEND;
	memset(&(socketInfo->wsaOverLapped), 0, sizeof(WSAOVERLAPPED));
	socketInfo->wsaOverLapped.hEvent = g_EventArray[g_index - WSA_WAIT_EVENT_0];
	socketInfo->wasBuf.buf = socketInfo->buffsrSend + socketInfo->dwBytesSend;
	socketInfo->wasBuf.len = (int)strlen(socketInfo->buffsrSend) - socketInfo->dwBytesSend;
	if (WSASend(socketInfo->socket, &(socketInfo->wasBuf), 1, &dwSendBytes, 0, &(socketInfo->wsaOverLapped), NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			sprintf(errorMsg, "WSASend() faile with error: %d. POS:CServer::SendACk()\n", WSAGetLastError());
			AfxMessageBox(errorMsg);
			return -1;
		}
	}
	return 0;
}
UINT ServerThread(LPVOID lpParameter){
	CServer* server = (CServer*)lpParameter;
	SOCKADDR_IN addrInfo;
	DWORD dwFlags;
	DWORD dwRecvBytes;
	char errorMsg[128];
	SOCKADDR_IN ClientAddr;
	int ClientAddrLength;
	LPCTSTR ClientIP;
	UINT  ClientPort;
	InitializeCriticalSection(&g_CriticalSection);
	if ((server->sListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		sprintf(errorMsg, "ERROR: Failed to get a socket %d.POS: ServerThread()\n", WSAGetLastError());
		AfxMessageBox(errorMsg, MB_OK, 0);
		WSACleanup();
		return 0;
	}
	CString sIP;
	sIP.Format("%d.%d.%d.%d", server->IpFild[0], server->IpFild[1], server->IpFild[2], server->IpFild[3]);
	addrInfo.sin_family = AF_INET;
	addrInfo.sin_addr.S_un.S_addr = inet_addr(sIP);
	addrInfo.sin_port = htons(server->m_serverPort);
	if (bind(server->sListen, (PSOCKADDR)&addrInfo, sizeof(addrInfo)) == SOCKET_ERROR) {
		sprintf(errorMsg, "ERROR: ********* %d.POS: ServerThread()\n", WSAGetLastError());
		AfxMessageBox(errorMsg, MB_OK, 0);
		return 0;
	}
	if (listen(server->sListen, SOMAXCONN)) {
		sprintf(errorMsg, "ERROR: ********* %d.POS: ServerThread()\n", WSAGetLastError());
		AfxMessageBox(errorMsg, MB_OK, 0);
		return 0;
	}
	if ((server->sDialog = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		sprintf(errorMsg, "ERROR: ********* %d.POS: ServerThread()\n", WSAGetLastError());
		AfxMessageBox(errorMsg, MB_OK, 0);
		return 0;
	}
	if ((g_EventArray[0] = WSACreateEvent()) == WSA_INVALID_EVENT) {
		sprintf(errorMsg, "ERROR: ********* %d.POS: ServerThread()\n", WSAGetLastError());
		AfxMessageBox(errorMsg, MB_OK, 0);
		return 0;
	}
	g_dwTotalEventAmount = 1;
	AfxBeginThread(ProccessThreadIO, (LPVOID)server);
	while (!server->m_bStopServer) {
		ClientAddrLength = sizeof(ClientAddr);
		if ((server->sDialog = accept(server->sListen, (sockaddr*)&ClientAddr, &ClientAddrLength)) == INVALID_SOCKET) {
			return 0;
		}
		ClientIP = inet_ntoa(ClientAddr.sin_addr);
		ClientPort = ClientAddr.sin_port;
		if (!server->SendWelcomeMsg(server->sDialog)){
			break;
		}
		EnterCriticalSection(&g_CriticalSection);
		if ((g_SocketArray[g_dwTotalEventAmount] = (LPSOCKET_INF)GlobalAlloc(GPTR, sizeof(SOCKET_INF))) == NULL) {
			sprintf(errorMsg, "ERROR: ********* %d.POS: ServerThread()\n", WSAGetLastError());
			AfxMessageBox(errorMsg, MB_OK, 0);
			return 0;
		}
		char buffer[DATA_BUFSIZE];
		ZeroMemory(buffer, DATA_BUFSIZE);
		g_SocketArray[g_dwTotalEventAmount]->wasBuf.len = DATA_BUFSIZE;
		g_SocketArray[g_dwTotalEventAmount]->wasBuf.buf = buffer;
		g_SocketArray[g_dwTotalEventAmount]->socket = server->sDialog;
		memset(&(g_SocketArray[g_dwTotalEventAmount]->wsaOverLapped), 0, sizeof(OVERLAPPED));
		memset(&(g_SocketArray[g_dwTotalEventAmount]->buffsrSend), 0, sizeof(DATA_BUFSIZE));
		memset(&(g_SocketArray[g_dwTotalEventAmount]->buffsrSend), 0, sizeof(DATA_BUFSIZE));
		g_SocketArray[g_dwTotalEventAmount]->dwBytesRecv = 0;
		g_SocketArray[g_dwTotalEventAmount]->dwBytesSend = 0;
		g_SocketArray[g_dwTotalEventAmount]->userLoggedIn = FALSE;
		g_SocketArray[g_dwTotalEventAmount]->SocketStatus = WSA_RECV;
		if ((g_SocketArray[g_dwTotalEventAmount]->wsaOverLapped.hEvent = g_EventArray[g_dwTotalEventAmount] = WSACreateEvent()) == WSA_INVALID_EVENT){
			sprintf(errorMsg, "ERROR: ********* %d.POS: ServerThread()\n", WSAGetLastError());
			AfxMessageBox(errorMsg, MB_OK, 0);
			return 0;
         }
		 dwFlags = 0;
		 if (WSARecv(g_SocketArray[g_dwTotalEventAmount]->socket, &(g_SocketArray[g_dwTotalEventAmount]->wasBuf), 1,
			 &dwRecvBytes, &dwFlags, &(g_SocketArray[g_dwTotalEventAmount]->wsaOverLapped), NULL) == SOCKET_ERROR) {
			 if (WSAGetLastError() != WSA_IO_PENDING) {
				 closesocket(g_SocketArray[g_dwTotalEventAmount]->socket);
				 WSACloseEvent(g_EventArray[g_dwTotalEventAmount]);
				 sprintf(errorMsg, "ERROR: ********* %d.POS: ServerThread()\n", WSAGetLastError());
				 AfxMessageBox(errorMsg, MB_OK, 0);
				 return 0;
			 }
		 }
		 g_dwTotalEventAmount++;
		 LeaveCriticalSection(&g_CriticalSection);
		 if (WSASetEvent(g_EventArray[0]) == FALSE) {
			 sprintf(errorMsg, "ERROR: ********* %d.POS: ServerThread()\n", WSAGetLastError());
			 AfxMessageBox(errorMsg, MB_OK, 0);
			 return 0;
		 }
    }
	server->m_bStopServer = FALSE;
	return 0;
}

UINT ProccessThreadIO(LPVOID LpParam)
{
	DWORD dwFlags;
	LPSOCKET_INF pSocketInfo;
	DWORD dwBytesTransferred;
	DWORD i;
	CServer* server = (CServer*)LpParam;
	char errorMsg[128];

	while (TRUE) {
		if ((g_index = WSAWaitForMultipleEvents(g_dwTotalEventAmount, g_EventArray, FALSE, WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED) {
			sprintf(errorMsg, "ERROR WSAWaitForMultipleEvents failed with error:%d\n", WSAGetLastError());
			AfxMessageBox(errorMsg, MB_OK | MB_ICONSTOP);
			return 0;
		}
		if ((g_index - WSA_WAIT_EVENT_0) == 0) {
			WSAResetEvent(g_EventArray[0]);
			continue;
		}
		pSocketInfo = g_SocketArray[g_index - WSA_WAIT_EVENT_0];
		WSAResetEvent(g_EventArray[g_index - WSA_WAIT_EVENT_0]);
		if (WSAGetOverlappedResult(pSocketInfo->socket, &(pSocketInfo->wsaOverLapped), &dwBytesTransferred, FALSE, &dwFlags) == FALSE || dwBytesTransferred == 0) {
			closesocket(pSocketInfo->socket);
			GlobalFree(pSocketInfo);
			WSACloseEvent(g_EventArray[g_index - WSA_WAIT_EVENT_0]);
			EnterCriticalSection(&g_CriticalSection);
			if ((g_index - WSA_WAIT_EVENT_0) + 1 != g_dwTotalEventAmount) {
				for (i = (g_index - WSA_WAIT_EVENT_0); i < g_dwTotalEventAmount; i++) {
					g_EventArray[i] = g_EventArray[i + 1];
					g_SocketArray[i] = g_SocketArray[i + 1];
				}
			}
			g_dwTotalEventAmount--;
			LeaveCriticalSection(&g_CriticalSection);
			continue;
		}
		if (pSocketInfo->SocketStatus == WSA_RECV) {
			memcpy(&pSocketInfo->bufferRecv[pSocketInfo->dwBytesRecv], pSocketInfo->wasBuf.buf, dwBytesTransferred);
			pSocketInfo->dwBytesRecv = dwBytesTransferred;
			if (pSocketInfo->dwBytesRecv > 2 && pSocketInfo->bufferRecv[pSocketInfo->dwBytesRecv - 2] == '\r'&&pSocketInfo->bufferRecv[pSocketInfo->dwBytesRecv - 1] == '\n') {
				if (!pSocketInfo->userLoggedIn) {
					if (server->Login(pSocketInfo) == LOGGED_IN) {
						pSocketInfo->userLoggedIn = TRUE;
						if (SetCurrentDirectory(pSocketInfo->userRootDir ) == 0) {
							sprintf(errorMsg, "ERROR:POS: ServerThread()\n failed with error: %d.", GetLastError());
							AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);

						}
					}
				}
				else {
					if (server->DealWithCommand(pSocketInfo) == FTP_QUIT) {
						break;
					}
				}
				memset(pSocketInfo->bufferRecv, 0, sizeof(pSocketInfo->bufferRecv));
				pSocketInfo->dwBytesRecv = 0;
			}
		}
		else {
			pSocketInfo->dwBytesSend += dwBytesTransferred;
		}
		if (server->RecvRequest(pSocketInfo) == -1) {
			return -1;
		}
	}
	return 0;
}

int CServer::RecvRequest(LPSOCKET_INF socketInfo)
{
	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;
	char errorMsg[128];
	socketInfo->SocketStatus = WSA_RECV;
	memset(&(socketInfo->wsaOverLapped), 0, sizeof(WSAOVERLAPPED));
	socketInfo->wsaOverLapped.hEvent = g_EventArray[g_index - WSA_WAIT_EVENT_0];
	socketInfo->wasBuf.len = DATA_BUFSIZE;
	if (WSARecv(socketInfo->socket, &(socketInfo->wasBuf), 1, &dwRecvBytes, &dwFlags, &(socketInfo->wsaOverLapped), NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			sprintf(errorMsg, "WSARecv() falied with error : %d.POS: CServer::RecvRequest()\n", WSAGetLastError());
			AfxMessageBox(errorMsg, MB_OK, MB_ICONERROR);
			return -1;
		}
	}
	return 0;
}

int CServer::ConvertCommaAddrToDotAddr(char * commAddr, LPDWORD pdwIpAddr, LPWORD pwPort)
{
	int index = 0;
	int i = 0;
	int commaAmout = 0;
	char dotAddr[MAX_ADDR_LEN];
	char szPort[MAX_ADDR_LEN];
	memset(dotAddr, 0, sizeof(dotAddr));
	memset(szPort, 0, sizeof(szPort));
	*pdwIpAddr = 0;
	*pwPort = 0;
	while (commAddr[index]) {
		if (commAddr[index] == ',') {
			commaAmout++;
			commAddr[index] = '.';
		}
		if (commaAmout < 4) {
			dotAddr[index] = commAddr[index];
		}
		else {
			szPort[i++] = commAddr[index];
		}
		index++;
	}
	if (commaAmout != 5) {
		return -1;
	}
	(*pdwIpAddr) = inet_addr(dotAddr);
	if ((*pdwIpAddr) == INADDR_NONE) {
		return -1;
	}
	char *temp = strtok(szPort + 1, ".");
	if (temp == NULL) {
		return -01;
	}
	(*pwPort) = (WORD)(atoi(temp) * 256);
	temp = strtok(NULL, ".");
	if (temp == NULL) {
		return -1;
	}
	(*pwPort) += (WORD)atoi(temp);
	return 0;
}

int CServer::ConvertDotAddrToCommAddr(char * dotAddr, WORD wPort, char * commAddr)
{
	char szPort[10];
	sprintf(szPort, "%d,%d,", wPort / 256, wPort % 256);
	sprintf(commAddr, "%s,", dotAddr);
	int index = 0;
	while (commAddr[index]) {
		if (commAddr[index] == '.') {
			commAddr[index] == ',';
		}
		index++;
	}
	sprintf(commAddr, "%s%s", commAddr, szPort);
	return 0;
}

int CServer::DataConnect(SOCKET & s, DWORD dwIp, WORD wPort, int nMode)
{
	char errorMsg[128];
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		sprintf(errorMsg, "DataConnect() error %d", WSAGetLastError());
		AfxMessageBox(errorMsg);
		return -1;
	}
	struct sockaddr_in addrInfo;
	addrInfo.sin_family = AF_INET;
	if (nMode == MODE_PASV) {
		addrInfo.sin_port = htons(wPort);
		addrInfo.sin_addr.s_addr = dwIp;
	}
	else {
		addrInfo.sin_port = htons(DATA_FTP_PORT);
		addrInfo.sin_addr.s_addr = inet_addr(GetLocalAddress());
	}
	BOOL bReuseAddr = TRUE;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&bReuseAddr, sizeof(BOOL)) == SOCKET_ERROR) {
		closesocket(s);
		sprintf(errorMsg, "setsockopt() error %d", WSAGetLastError());
		AfxMessageBox(errorMsg);
		return -1;
	}
	if (bind(s, (struct sockaddr*)&addrInfo, sizeof(addrInfo)) == SOCKET_ERROR) {
		closesocket(s);
			sprintf(errorMsg, "bind() error %d", WSAGetLastError());
			AfxMessageBox(errorMsg);
			return -1;
	}
	if (nMode == MODE_PASV) {
		if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
			closesocket(s);
			sprintf(errorMsg, "listen() error %d", WSAGetLastError());
			AfxMessageBox(errorMsg);
			return -1;
		}
	}
	else if (nMode == MODE_PORT) {
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(wPort);
		addr.sin_addr.s_addr = dwIp;
		if (connect(s, (const sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
			closesocket(s);
			sprintf(errorMsg, "connect() error %d", WSAGetLastError());
			AfxMessageBox(errorMsg);
			return -1;
		}
	}
	return 0;
}

int CServer::TryDeleteFile(char * deletedPath)
{
	CFileFind fileFider;
	if (!fileFider.FindFile(deletedPath)) {
		return CANNOT_FIND;
	}
	else if (DeleteFile(deletedPath)) {
		return DIR_CHANGED;
	}
	else {
		return ACCESS_DENY;
	}
	return 0;
}

BOOL CServer::IsPathExist(char * path)
{
	if (GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES || GetLastError() != ERROR_FILE_NOT_FOUND) {
		return TRUE;
	}
	return FALSE;
}

int CServer::GetFileList(LPFILE_INF fileInfo, int arraySize, const char * searchPath)
{
	int index = 0;
	WIN32_FIND_DATA wfd;
	ZeroMemory(&wfd, sizeof(WIN32_FIND_DATA));
	char fileName[MAX_PATH];
	ZeroMemory(fileName, MAX_PATH);
	GetCurrentDirectory(MAX_PATH, fileName);
	if (fileName[strlen(fileName) - 1] != '\\') {
		strcat(fileName, "\\");
	}
	strcat(fileName, searchPath);
	HANDLE fileHandle = FindFirstFile((LPCTSTR)fileName, &wfd);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		//lstrcpy(fileInfo[index].szFileName, wfd.cFileName);
		//fileInfo[index].dwFileAttributes = wfd.dwFileAttributes;
		//fileInfo[index].ftCreationTime = wfd.ftCreationTime;
		//fileInfo[index].ftLastAccessTime = wfd.ftLastAccessTime;
		//fileInfo[index].ftLastWriteTime = wfd.ftLastWriteTime;
		//fileInfo[index].nFileSizeHigh = wfd.dwFileAttributes;
		//fileInfo[index].nFileSizeLow = wfd.nFileSizeLow;
		//index++;
		while (FindNextFile(fileHandle, &wfd) != 0 && index < arraySize) {
			if (wfd.cFileName[0] == '.')continue;
			lstrcpy(fileInfo[index].szFileName, wfd.cFileName);
			fileInfo[index].dwFileAttributes = wfd.dwFileAttributes;
			fileInfo[index].ftCreationTime = wfd.ftCreationTime;
			fileInfo[index].ftLastAccessTime = wfd.ftLastAccessTime;
			fileInfo[index].ftLastWriteTime = wfd.ftLastWriteTime;
			fileInfo[index].nFileSizeHigh = wfd.dwFileAttributes;
			fileInfo[index].nFileSizeLow = wfd.nFileSizeLow;
			index++;
		}
		FindClose(fileHandle);
	}

	return index;
}

int CServer::DataSend(SOCKET s, char * buff, int buffSize)
{
	int nBytesLeft = buffSize;
	int index = 0;
	int sendBytes = 0;
	char errorMsg[128];
	while (nBytesLeft > 0) {
		sendBytes = send(s, &buff[index], nBytesLeft, 0);
		if (sendBytes == SOCKET_ERROR) {
			closesocket(s);
			sprintf(errorMsg, "send() failed to send with error:%d.POS:CServer::DataSend()\n", WSAGetLastError);
			AfxMessageBox(errorMsg, MB_OK | MB_ICONERROR);
			return  -1;
		}
		nBytesLeft -= sendBytes;
		index += sendBytes;
	}
	return index;
}

int CServer::CombindFileNameSize(const char * fileName, char * fileNameSize)
{
	int fileSize = -1;
	FILE_INF fileInfo[1];
	int fileAmount = GetFileList(fileInfo, 1, fileName);
	if (fileAmount == -1) {
		return -1;
	}

	sprintf(fileNameSize, "%s<%dbytes>", fileName, fileInfo[0].nFileSizeLow);
	fileSize = fileInfo[0].nFileSizeLow;

	return fileSize;
}

int CServer::ReadFileToBUffer(const char * szFileName, char * buff, int fileSize)
{
	int index = 0;
	int bytesLeft = fileSize;
	int bytesRead = 0;
	char fileAbsolutePath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, fileAbsolutePath);
	if (fileAbsolutePath[strlen(fileAbsolutePath) - 1] != '\\') {
		strcat(fileAbsolutePath,"\\");
	}
	strcat(fileAbsolutePath, szFileName);
	HANDLE hFile = CreateFile(fileAbsolutePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		AfxMessageBox(_T("Filed in CreateFile(),POS:CServer::ReadFileToBuffer()"));
	}
	else {
		while (bytesLeft > 0) {
			if (!ReadFile(hFile, &buff[index], bytesLeft, (LPDWORD)&bytesRead, NULL)) {
				CloseHandle(hFile);
				AfxMessageBox(_T("File in ReadFile(),POS:CServer::ReadFileToBuffer()"));
				return 0;
			}
			index += bytesRead;
			bytesLeft -= bytesRead;
		}
		CloseHandle(hFile);
	}
	return index;
}

int CServer::DataRecv(SOCKET s, const char * fileName)
{
	DWORD index = 0;
	DWORD bytesWritten = 0;
	DWORD buffBytes = 0;
	char buff[DATA_BUFSIZE];
	int totalRecvBytes = 0;
	char fileAbsolutePath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, fileAbsolutePath);
	strcat(fileAbsolutePath, "\\");
	strcat(fileAbsolutePath, fileName);
	HANDLE hFile = CreateFile(fileAbsolutePath, GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		AfxMessageBox(_T("CreateFile() failed.Pos:CServer::DataRecv()"));
		return -1;
	}
	while (TRUE) {
		int nBytesRecv = 0;
		index = 0;
		buffBytes = DATA_BUFSIZE;
		while (buffBytes > 0) {
			nBytesRecv = recv(s, &buff[index], buffBytes, 0);
if (nBytesRecv == SOCKET_ERROR) {
	AfxMessageBox(_T("Failed in recv().Pos:CServer::DataRecv()"));
	return -1;
}
if (nBytesRecv == 0) {
	break;
}
index += nBytesRecv;
buffBytes -= nBytesRecv;
		}
		totalRecvBytes += index;
		buffBytes = index;
		index = 0;
		while (buffBytes > 0) {
			if (!SetEndOfFile(hFile)) {
				return 0;
			}
			if (!SetEndOfFile(hFile)) {
				return 0;
			}
			if (!WriteFile(hFile, &buff[index], buffBytes, &bytesWritten, NULL)) {
				CloseHandle(hFile);
				AfxMessageBox(_T("写入文件出错"));
				return 0;
			}
			index += bytesWritten;
			buffBytes -= bytesWritten;
		}
		if (nBytesRecv == 0) {
			break;
		}
	}
	CloseHandle(hFile);
	return totalRecvBytes;
}

void CServer::GetRalativeDirectory(char * currDir, char * rootDir, char * ralaDir)
{
	int nStrlen = (int)strlen(rootDir);
	if (_strnicmp(currDir, rootDir, nStrlen) == 0) {
		strcpy(ralaDir, currDir + nStrlen);
	}
	if (ralaDir != NULL && ralaDir[0] == '\0') {
		strcpy(ralaDir, "/");
	}
}

void CServer::GetAbsoluteDirectory(char * dir, char * currentDir, char * userCurrDir)
{
	char szTemp[MAX_PATH];
	int len = 0;
	strcpy(userCurrDir, currentDir);
	if (strcmp(dir, "..") == 0) {
		int flag = 0;
		len = (int)strlen(userCurrDir);
		for (int i = 0; i < len; i++) {
			if (userCurrDir[i] == '\\') {
				flag = i;
			}
		}
		if (flag > 2) {
			userCurrDir[flag] == '\0';
		}
		else if (flag == 2) {
			userCurrDir[flag + 1] == '\0';
		}
		return;
	}
	len = (int)strlen(userCurrDir);
	if (userCurrDir[len - 1] == '\\') {
		if (dir[0] == '/' || dir[0] == '\\') {
			strcpy(szTemp, dir + 1);
		}
		else {
			strcpy(szTemp, dir);
		}
	}
	else {
		if (dir[0] != '/'&&dir[0] != '\\') {
			strcpy(szTemp, "\\");
			strcpy(szTemp, dir);
		}
		else {
			strcpy(szTemp, dir);
		}
	}
	strcat(userCurrDir, szTemp);
	NetToHost(userCurrDir);
}

void CServer::HostToNet(char * path)
{
	int index = 0;
	while (path[index]) {
		if (path[index] == '\\') {
			path[index] = '/';
		}
		index++;
	}
}

void CServer::NetToHost(char * path)
{
	int index = 0;
	while (path[index]) {
		if (path[index] == '/') {
			path[index] = '\\';
		}
		index++;
	}
}

char * CServer::GetLocalAddress()
{
	char hostName[128], errorMsg[120];
	memset(hostName, 0, sizeof(hostName));
	if (gethostname(hostName, sizeof(hostName)) == SOCKET_ERROR) {
		sprintf(errorMsg, "gethostName() filled with error:%d", WSAGetLastError());
		AfxMessageBox(errorMsg);
		return NULL;
	}
	LPHOSTENT lpHostEnt;
	lpHostEnt = gethostbyname(hostName);
	if (lpHostEnt == NULL) {
		sprintf(errorMsg, "gethostbyName() filled with error:%d", WSAGetLastError());
		AfxMessageBox(errorMsg);
		return NULL;
	}
	struct in_addr* pinAddr;
	pinAddr = ((LPIN_ADDR)lpHostEnt->h_addr);
	int length = (int)strlen(inet_ntoa(*pinAddr));
	if ((DWORD)length > sizeof(hostName)) {
		WSASetLastError(WSAEINVAL);
		return NULL;
	}
	return inet_ntoa(*pinAddr);
}

SOCKET CServer::AcceptConnectRequest(SOCKET & s)
{
	SOCKET sDialog = accept(s, NULL, NULL);
	if (sDialog == INVALID_SOCKET) {
		AfxMessageBox(_T("accept() failed. POS:CServer::AcceptConnectReauest()"), MB_OK | MB_ICONERROR);
		return NULL;
	}
	return sDialog;
}

int CServer::FileListToString(char * buffer, UINT nBufferSize, BOOL isListCommand)
{
	FILE_INF fileInfo[MAX_FILE_NUM];
	int fileAmmount = GetFileList(fileInfo, MAX_FILE_NUM, "*.*");
	sprintf(buffer, "%s", "");
	if (isListCommand) {
		SYSTEMTIME systemTime;
		char tempTimeFormat[128] = "";
		char timeBlock[20];
		for (int i = 0; i < fileAmmount; i++) {
			if (strlen(buffer) > nBufferSize - 128) {
				break;
			}
			if (strcmp(fileInfo[i].szFileName, ".") == 0 || strcmp(fileInfo[i].szFileName, "..") == 0) {
				continue;
			}
			if (FileTimeToSystemTime(&(fileInfo[i].ftLastWriteTime), &systemTime) == 0) {
				char errorMsg[128];
				sprintf(errorMsg, "POS:CServer::FileListToString()\nFailed eo convert filetime to systemTime with error:%d.", GetLastError());
				AfxMessageBox(errorMsg);
			}
			if (systemTime.wHour <= 12) {
				strcpy(timeBlock, "AM");
			}
			else {
				systemTime.wHour - 12;
				strcpy(timeBlock, "PM");
			}
			if (systemTime.wYear >= 2000) {
				systemTime.wYear -= 2000;
			}
			else {
				systemTime.wYear -= 1900;
			}
			ZeroMemory(tempTimeFormat, sizeof(tempTimeFormat));
			sprintf(tempTimeFormat, "%02u-%02u-%02u  %02u:%02u%s  ", systemTime.wMonth, systemTime.wDay, systemTime.wYear, systemTime.wHour, systemTime.wMinute, timeBlock);
			strcat(buffer, tempTimeFormat);
			if (fileInfo[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				strcat(buffer, "<DIR>");
				strcat(buffer, "     ");
			}
			else {
				strcat(buffer, "   ");
				//sprintf(tempTimeFormat, "%9d  Bytes", fileInfo[i].nFileSizeLow);
				//strcat(buffer, tempTimeFormat);
			}
			//strcat(buffer, curBuffer);
			//strcat(buffer, "\\");
			strcat(buffer, fileInfo[i].szFileName);
			strcat(buffer, "\r\n");
			}
		}
	else {
		for (int i = 0; i < fileAmmount; i++) {
			if (strlen(buffer) + strlen(fileInfo[i].szFileName) + 2 < nBufferSize) {
				strcat(buffer, fileInfo[i].szFileName);
				strcat(buffer, "\r\n");
			}
			else {
				break;
			}
		}
	}
	return (int)strlen(buffer);
}
