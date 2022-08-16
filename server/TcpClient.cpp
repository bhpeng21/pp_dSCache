/*
 * TcpClient.cpp - client客户端类成员函数实现
 * 作者：徐轶哲 
 * 版本 1.0
 * 2021/12/17
 */
#include "TcpClient.h"

CTcpClient::CTcpClient()
{
    m_sockfd = 0;  // 构造函数初始化m_sockfd
}

CTcpClient::~CTcpClient()
{
    if (m_sockfd != 0) close(m_sockfd);  // 析构函数关闭m_sockfd
}

//功能：向服务器发起连接
//输入：
//  serverip-服务端ip指针，port通信端口
//输出：
//  返回是否连接成功

bool CTcpClient::ConnectToServer(const char* serverip, const int port)
{
    m_sockfd = socket(AF_INET, SOCK_STREAM, 0); // 创建客户端的socket

    struct hostent* h; // ip地址信息的数据结构
    if ((h = gethostbyname(serverip)) == 0)
    {
        close(m_sockfd); m_sockfd = 0; return false;
    }

    // 把服务器的地址和端口转换为数据结构
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);

    // 向服务器发起连接请求
    if (connect(m_sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        close(m_sockfd); m_sockfd = 0; return false;
    }

    return true;
}
//功能：向服务器发送报文
//输入：
//  待发送字符串报文指针buf, 报文长度 buflen
//输出：
//  返回是否发送成功 若返回值<=0则说明发送失败
int CTcpClient::Send(const void* buf, const int buflen)
{
    return send(m_sockfd, buf, buflen, 0);
}

//功能：接受服务器报文
//输入：
//  待接受字符串报文指针buf, 报文长度 buflen
//输出：
//  返回是否接受成功 若返回值<=0则说明接受失败
int CTcpClient::Recv(void* buf, const int buflen)
{
    return recv(m_sockfd, buf, buflen, 0);
}