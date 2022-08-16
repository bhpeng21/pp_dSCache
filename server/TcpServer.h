/*
 * TcpClient.cpp - client客户端类成员函数实现
 * 作者：徐轶哲 
 * 版本 1.0
 * 2021/12/17
 */
#pragma once

#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

class CTcpServer
{
public:
    int m_listenfd;   // 服务端用于监听的socket
    int m_clientfd;   // 客户端连上来的socket

    CTcpServer();

    bool InitServer(int port);  // 初始化服务端

    bool Accept();  // 等待客户端的连接

    // 向对端发送报文
    int  Send(const void* buf, const int buflen);
    // 接收对端的报文
    int  Recv(void* buf, const int buflen);

    ~CTcpServer();
};

#endif