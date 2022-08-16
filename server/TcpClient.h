/*
 * TcpClient.h - client客户端类定义
 * 作者：徐轶哲 
 * 版本 1.0
 * 2021/12/17
 */
#pragma once

#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;
// TCP客户端类
class CTcpClient
{
public:
    int m_sockfd;

    CTcpClient();

    // 向服务器发起连接，serverip-服务端ip，port通信端口
    bool ConnectToServer(const char* serverip, const int port);
    // 向对端发送报文
    int  Send(const void* buf, const int buflen);
    // 接收对端的报文
    int  Recv(void* buf, const int buflen);

    ~CTcpClient();
};

#endif
