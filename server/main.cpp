/*
 * TcpClient.cpp - client客户端类成员函数实现
 * 作者：徐轶哲 
 * 版本 1.0
 * 2021/12/17
 */
#include "LRU.h"
#include "TcpServer.h"
#include "utils.h"


LRUCache myCache(5); // 创建全局变量 cache节点

int main(int argc,char *argv[])
{

  if (argc != 2)
  {
    printf("usage: ./tcpselect port\n"); return -1;
  }

  CTcpServer TcpServer;
  CTcpClient TcpClient_1; // 创建客户端1 用于跟第二个cache Server节点连接
  CTcpClient TcpClient_2; // 创建客户端2 用于跟第三个cache Server节点连接


  // 初始化服务端用于监听的socket。
  if (TcpServer.InitServer(atoi(argv[1])) == false)
  {
    printf("TcpServer.InitServer failed,exit...\n"); return -1;
  }


  fd_set readfdset;  // 读事件的集合，包括监听socket和客户端连接上来的socket。
  int maxfd;  // readfdset中socket的最大值。

  // 初始化结构体，把TcpServer.m_listenfd添加到集合中。
  FD_ZERO(&readfdset);

  FD_SET(TcpServer.m_listenfd,&readfdset);
  maxfd = TcpServer.m_listenfd;

  while (1)
  {
    // 调用select函数时，会改变socket集合的内容，所以要把socket集合保存下来，传一个临时的给select。
    fd_set tmpfdset = readfdset;

    int infds = select(maxfd+1,&tmpfdset,NULL,NULL,NULL);
    // printf("select infds=%d\n",infds);

    // 返回失败。
    if (infds < 0)
    {
    printf("select() failed.\n"); perror("select()"); break;
    }

    // 超时，在本程序中，select函数最后一个参数为空，不存在超时的情况，但以下代码还是留着。
    if (infds == 0)
    {
      printf("select() timeout.\n"); continue;
    }

    // 检查有事情发生的socket，包括监听和客户端连接的socket。
    // 这里是客户端的socket事件，每次都要遍历整个集合，因为可能有多个socket有事件。
    for (int eventfd=0; eventfd <= maxfd; eventfd++)
    {
      if (FD_ISSET(eventfd,&tmpfdset)<=0) continue;

      if (eventfd==TcpServer.m_listenfd)
      { 
        // 如果发生事件的是TcpServer.m_listenfd，表示有新的客户端连上来。
        
        if (TcpServer.Accept() == false)
        { 
          printf("TcpServer.Accept() failed,exit...\n"); continue; 
        }


        printf ("client(socket=%d) connected ok.\n",TcpServer.m_clientfd);

        // 把新的客户端socket加入集合。
        FD_SET(TcpServer.m_clientfd,&readfdset);

        if (maxfd < TcpServer.m_clientfd) maxfd = TcpServer.m_clientfd;

        continue;
      }
      else
      {
        // 客户端有数据过来或客户端的socket连接被断开。
        char buffer[1024];
        memset(buffer,0,sizeof(buffer));

        // 读取客户端的数据。
        ssize_t isize=read(eventfd,buffer,sizeof(buffer));

        // 发生了错误或socket被对方关闭。
        if (isize <=0)
        {
          printf("client(eventfd=%d) disconnected.\n",eventfd);

          close(eventfd);  // 关闭客户端的socket。

          FD_CLR(eventfd,&readfdset);  // 从集合中移去客户端的socket。

          // 重新计算maxfd的值，注意，只有当eventfd==maxfd时才需要计算。
          if (eventfd == maxfd)
          {
            for (int ii=maxfd;ii>0;ii--)
            {
              if (FD_ISSET(ii,&readfdset))
              {
                maxfd = ii; break;
              }
            }

            printf("maxfd=%d\n",maxfd);
          }

          continue;
        }

        printf("recv(eventfd=%d,size=%ld):%s\n",eventfd,isize,buffer);

        // 对获取报文进行解析，判别是来自于client、master或者其它cache 节点
        char sendbuffer[1024];
        string sendStr;
        string recvStr(buffer);
        
        if(recvStr.size()<7) // 输入命令不合规范，直接返回要求重新输入
        {
          sendStr = "输入数据报命令有误，请重新输入！";
          strcpy(sendbuffer, sendStr.c_str());
          write(eventfd,sendbuffer,strlen(sendbuffer));
          continue;
        }
        else if(recvStr.substr(0, 3)=="get" || recvStr.substr(0, 3)=="set") // 数据包来自于 client
        {
          sendStr = PreClient(recvStr, myCache);
          strcpy(sendbuffer, sendStr.c_str());
          write(eventfd,sendbuffer,strlen(sendbuffer));
        }
        else if(recvStr.substr(0, 6)=="expand" || recvStr.substr(0, 6)=="narrow") // 数据包来自于 master 为扩容缩容命令
        {
          // 输入格式错误
          if(recvStr.size()<22)
          {
            sendStr = "输入数据报命令有误，请重新输入！";
            strcpy(sendbuffer, sendStr.c_str());
            write(eventfd,sendbuffer,strlen(sendbuffer));
            continue;
          }
          // 将此cache节点与扩容缩容其它cache节点进行连接
          char ip[16];
          strcpy(ip, recvStr.substr(7, 13).c_str()); // 存储读取的ip地址
          if (TcpClient_1.ConnectToServer(ip, atoi(recvStr.substr(21, 4).c_str())) == false) // 目前设定ip地址长度为13 如 10.134.52.232 端口长度为4 如 5006
          {
            printf("connect(%s:%s) failed.\n",argv[1],argv[2]); close(TcpClient_1.m_sockfd); 
            sendStr = "扩容/缩容中连接其它cache节点失败！";
            
          }
          else sendStr = PreMaster(recvStr, myCache, TcpClient_1);
          strcpy(sendbuffer, sendStr.c_str());
          write(eventfd,sendbuffer,strlen(sendbuffer));
        }
        else if(recvStr.substr(0, 7)=="isAlive") // 数据报来自于master 心跳机制 检测cache server节点是否正常运行
        {
          sendStr = "ALIVE";
          strcpy(sendbuffer, sendStr.c_str());
          write(eventfd,sendbuffer,strlen(sendbuffer));
        }
        else if(recvStr.substr(0, 11)=="updateCache") // 数据包来自于 cache server节点
        {
          sendStr = PreCacheServer(recvStr, myCache);
          strcpy(sendbuffer, sendStr.c_str());
          write(eventfd,sendbuffer,strlen(sendbuffer));
        }
        else
        {
          sendStr = "输入命令有误，请重新输入！";
          strcpy(sendbuffer, sendStr.c_str());
          write(eventfd,sendbuffer,strlen(sendbuffer));
        }
        // else if(st.substr(0, 3) == "")

        
        // write(eventfd,sendbuffer,strlen(sendbuffer));
      }
    }
  }

  return 0;
}

