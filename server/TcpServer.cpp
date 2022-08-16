/*
 * TcpClient.cpp - client客户端类成员函数实现
 * 作者：徐轶哲 
 * 版本 1.0
 * 2021/12/17
 */
#include"TcpServer.h"
CTcpServer::CTcpServer()
{
    // 构造函数初始化socket
    m_listenfd = m_clientfd = 0;
}

CTcpServer::~CTcpServer()
{
    if (m_listenfd != 0) close(m_listenfd);  // 析构函数关闭socket
    if (m_clientfd != 0) close(m_clientfd);  // 析构函数关闭socket
}

//功能：初始化服务端的socket
//输入：
//  通信端口port
//输出：
//  返回是否初始化成功
bool CTcpServer::InitServer(int port)
{
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);  // 创建服务端的socket

    // 把服务端用于通信的地址和端口绑定到socket上
    struct sockaddr_in servaddr;    // 服务端地址信息的数据结构
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;  // 协议族，在socket编程中只能是AF_INET
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // 本主机的任意ip地址
    servaddr.sin_port = htons(port);  // 绑定通信端口
    if (bind(m_listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        close(m_listenfd); m_listenfd = 0; return false;
    }

    // 把socket设置为监听模式
    if (listen(m_listenfd, 5) != 0) { close(m_listenfd); m_listenfd = 0; return false; }

    return true;
}
//功能：接收客户端连接
//输入：
//  无
//输出：
//  返回是否接收成功
bool CTcpServer::Accept()
{
    if ((m_clientfd = accept(m_listenfd, 0, 0)) <= 0) return false;

    return true;
}
//功能：发送字符串数据报
//输入：
//  待发送字符串指针buf, 字符串长度
//输出：
//  返回是否发送成功，<=0为发送失败
int CTcpServer::Send(const void* buf, const int buflen)
{
    return send(m_clientfd, buf, buflen, 0);
}

//功能：接收字符串数据报
//输入：
//  待接受字符串指针buf, 字符串长度
//输出：
//  返回是否接收成功，<=0为发送失败
int CTcpServer::Recv(void* buf, const int buflen)
{
    return recv(m_clientfd, buf, buflen, 0);
}