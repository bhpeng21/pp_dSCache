/*
 * main.cpp - client客户端主函数 
 * 作者：徐轶哲 
 * 版本 1.0
 * 2021/12/17
 */
#include "TcpClient.h"

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    printf("usage:./tcpclient ip port\n"); return -1; // 运行时需要输入 ip 和 端口号
  }

  CTcpClient TcpClient;   // 用于跟master连接
  CTcpClient TcpClient_2; // 用于跟cache server连接
  // 向服务器发起连接请求
  if (TcpClient.ConnectToServer(argv[1], atoi(argv[2])) == false)
  {
    printf("connect(%s:%s) failed.\n",argv[1],argv[2]); close(TcpClient.m_sockfd);  return -1;
  }

  printf("connect ok.\n");
  char buf[1024];

  for (int ii=0;ii<10000;ii++)
  {
    // 从命令行输入内容。
    memset(buf,0,sizeof(buf));
    printf("please input:"); 
    // scanf("%s",buf);
    cin.getline(buf, sizeof(buf)); // 获取客户端输入  注意：此时用的是cin.getline()来获取输入，因为其它输入方式比如scanf、cin等输入不能带有空格
    string str(buf);
    if(str=="exit")
    {
      close(TcpClient.m_sockfd); break;
    }
    if (write(TcpClient.m_sockfd,buf,strlen(buf)) <=0) // 将客户端输入发送到远程服务器中
    { 
      printf("write() failed.\n");  close(TcpClient.m_sockfd);   continue;// 发送失败则关闭客户端
    }
		
    memset(buf,0,sizeof(buf));
    if (read(TcpClient.m_sockfd,buf,sizeof(buf)) <=0)  // 读取远程服务器响应
    { 
      printf("read() failed.\n");  close(TcpClient.m_sockfd);  continue;// 读取远程服务器响应失败
    }
    string recvStr(buf);
    string sendStr;
    // 与 cache server节点进行通信
    if(recvStr.size()>30) // master返回的字符串数据报满足要求
    {
      string command = recvStr.substr(23, 3); // 提取出client与cache通信命令 set / get
      if(command=="set"||command=="get")
      {
        memset(buf,0,sizeof(buf));
        char ip[16];
        strcpy(ip, recvStr.substr(0, 13).c_str()); // 存储读取的ip地址
        if (TcpClient_2.ConnectToServer(ip, atoi(recvStr.substr(14, 4).c_str())) == false) // 目前设定ip地址长度为13 如 10.134.52.232 端口长度为4 如 5006
        {
          printf("connect(%s:%s) failed.\n",argv[1],argv[2]); close(TcpClient_2.m_sockfd); 
        }
        else
        {
          if(command=="set")
          {
            sendStr = recvStr.substr(23, 13);
            strcpy(buf, sendStr.c_str());
            write(TcpClient_2.m_sockfd,buf,strlen(buf));
          }
          else if(command=="get")
          {
            sendStr = recvStr.substr(23, 8);
            strcpy(buf, sendStr.c_str());
            write(TcpClient_2.m_sockfd,buf,strlen(buf));
          }
           memset(buf,0,sizeof(buf));
          if (read(TcpClient_2.m_sockfd,buf,sizeof(buf)) <=0)  // 读取远程服务器响应
          { 
            printf("read() failed.\n");  close(TcpClient_2.m_sockfd);  // 读取远程服务器响应失败
          }
          printf("recv:%s\n",buf);
          close(TcpClient_2.m_sockfd);
        }
       
      }
    }
    else printf("recv:%s\n",buf);

    // close(TcpClient.m_sockfd); break;
  }
} 
