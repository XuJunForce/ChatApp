#define _CRT_SECURE_NO_WARNINGS
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<Windows.h>
#include<stdio.h>
#include<string>

#define BUF_SIZE 1024

#pragma comment(lib,"Ws2_32.lib")

char szMsg[BUF_SIZE];


unsigned SendMsg(void* arg) {
    SOCKET sock = *(SOCKET*)arg;
    while (1) {
        scanf("%s", szMsg);
        if (!(strcmp(szMsg, "Quit") || strcmp(szMsg, "quit")))
        {
            closesocket(sock);
            exit(0);
        }
        send(sock, szMsg, strlen(szMsg), 0);
    }
    return 0;
}


unsigned RecvMsg(void* arg) {
    SOCKET sock = *(SOCKET*)arg;

    char msg[BUF_SIZE];
    while (1) {
        int len = recv(sock, msg, sizeof(msg) - 1, 0);
        if (len == -1) {
            return -1;
        }
        msg[len] = '\0';
        printf("%s\n", msg);
    }
    return 0;
}





int main() {
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        return -1;
    }

    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        WSACleanup();
        return -1;
    }

    /* The WinSock DLL is acceptable. Proceed. */

    SOCKET hSock;
    hSock = socket(AF_INET, SOCK_STREAM, 0);

    //绑定端口
    SOCKADDR_IN servAdr;
    memset(&servAdr, 0, sizeof(servAdr));
    servAdr.sin_family = AF_INET;
    servAdr.sin_port = htons(9999);
    inet_pton(AF_INET, "47.107.126.242", &servAdr.sin_addr);

    //连接服务器
    if (connect(hSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) {
        printf("connet error\n");
        return -1;
    }
    else {
        printf("请输入你的名字:");
    }

    //循环发消息
    HANDLE hSendHand = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SendMsg, (void*)&hSock, 0, NULL);

    //循环收消息
    HANDLE hRecvHand = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecvMsg, (void*)&hSock, 0, NULL);

    //等待两个线程结束

    WaitForSingleObject(hSendHand, INFINITE);
    WaitForSingleObject(hRecvHand, INFINITE);


    closesocket(hSock);


    WSACleanup();

    return 0;



}