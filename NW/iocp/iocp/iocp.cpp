﻿#include <iostream>
using namespace std;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h> 
#include <stdlib.h> 
#include <stdio.h>

#define SERVERPORT 9000
#define BUFSIZE	512

struct SOCKETINFO
{
	OVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	WSABUF wsabuft;
};


DWORD WINAPI WorkerThread(LPVOID arg);

void err_quit(const char *msg);
void err_display(const char *msg);

int main(int argc, char *argv[])
{
	int retval;


	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), & wsa) != 0) return 1;

	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL) return 1;

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	HANDLE hThread;
	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; i++) {
		hThread = CreateThread(NULL, 0, WorkerThread, hcp, 0, NULL);
		if (hThread == NULL) return 1;
		CloseHandle(hThread);
	}

	SOCKET listen_sock = socket(AF_INET,SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("soclket()");

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)err_quit("listen()");

	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen;
	DWORD recvbytes, flags;

	while (1) {
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));

		CreateIoCompletionPort((HANDLE)client_sock, hcp, client_sock, 0);
		SOCKETINFO* ptr = new SOCKETINFO;
		if (ptr == NULL) break;
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = client_sock;
		ptr->recvbytes = ptr->sendbytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuft.len = BUFSIZE;

		flags = 0;
		retval = WSARecv(client_sock, &ptr->wsabuf, 1, &recvbytes,
			&flags, &ptr->overlapped, NULL);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING) {
				err_display("WSARecv()");
			}
			continue;
		}
	}

	WSACleanup();
	return 0;

	DWORD WINAPI WorkerThread(LPVOID arg) {

		int retval;
		HANDLE hcp = (HANDLE)arg;


		while (1) {
			DWORD cbTransferred;
			SOCKET client_sock;
			SOCKETINFO* ptr;
			retval = GetQueuedCompletionStatus(hcp, &cbTransferred,
				(LPDWORD)&client_sock, (LPOVERLAPPED*)&ptr, INFINITE);

			SOCKADDR_IN clientaddr;
			int addrlen = sizeof(clientaddr);
			getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

			if (retval == 0 || cbTransferred == 0)1{
				if (retval == 0) {
					DWORD temp1, temp2);
					WSAGetOverlappedResult(ptr->sock, &ptr->overlapped,
						&temp1, FALSE, &temp2);
					err_display("WSAGetOverlappedResult()");
				}
				closesocket(ptr->soclk);
				printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
					inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
				delete ptr;
				continue;
			}

			if (ptr->recvbytes == 0) {
				ptr->recvhbytes = cbTransferred;
				ptr->sendbytes = 0;
				ptr->buf[ptr->recvbytes] = '\0';
				printf("[TCP\%s:%d]%s\n", inet_ntoa(clientaddr.sin_addr),
					ntohs(clientaddr.sin_port), ptr->buf);
			}

			else {
				ptr->sendbytes += cbTransferred;
			}


			if (ptr->recvbytes > ptr->sendbytes) {
				ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
				ptr->wsabuf.buf = ptr->buf + ptr->sendbytes;
				ptr->wsabuf.len = ptr->recvbytes - ptr->sendbytes;

				DWORD sendbytes;
				retval = WSASend(ptr->sock, &ptr->wsabut, 1,
					&sendbytes, 0, &ptr->overlapped, NULL);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING) {
						err_display("WSASend()");
					}
					continue;
				}
			}

			else {

				ptr->recvhbytes = 0;

				ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
				ptr->wsabuf.buf = ptr->butf;
				ptr->wsabuf.len = BUFSIZE;

				DWORD recvhbytes;
				DWORD flags = 0;
				retval = WSARecv(ptr->sock, &ptr->wsabut, 1,
					&recvbytes, &flags, &ptr->overlapped, NULL);
				if (retval == SOCKET_ERROR) {
					if (WSAGetLastErrorltl) != WSA IO PENDING){
						err_display("WSARecv()");
						}
						continue;
				}
			}
		}



		return 0;
	}

	void err_quit(const char* msg)
	{
		LPVOID lpMsgButf;
		FormatMessagel(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, WSAGetLastError(),
			MAKELANGID(LANG _NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgButf, 0, NULL);
		MessageBox(NULL, (LPCTSTR)IpMsgBuf, msg, MB_ICONERROR);
		LocalFree(lpMsgBuf);
		exit(1);
	}

	void err_display(const char* msg)
	{
		LPVOID lpMsgButf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, WSAGetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgButf, 0, NULL);
		printf("[%s] %s", msg, (char*)lpMsgBuf);
		LocalFree(IpMsgBuf);
	}