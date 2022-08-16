#ifndef _WUHUSERVER_H_
#define _WUHUSERVER_H_

#include <map>
#include <vector>
#include <list>
#include <iostream>
#include <cstdlib>
#include <string>
#include <utility>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "TcpClient.h"
#include "wuhuDc.h"
#include "md5.h"

using namespace std;


class wuhuServer
{
    // friend class CTcpClient;
private:
    //配置文件绝对路径
    //serverCron()循环函数每秒调用次数
    //serverCron()运行次数计数器

    //cache server数量
    int nodeNum = 0;

    //cache server索引表
    list<wuhuDc> listDc;

    //客户端链表

    //时钟
    long long timeval;
    //tcp监听端口
    //地址
    //地址数量
    //一系列socket参数

    //槽分配
    vector<pair<string, pair<int, int>>> slots;
public:
    //CTcpClient TcpClient;
    wuhuServer();
    ~wuhuServer();
    //分配槽
    void allocateSlot();
    //增加槽
    void addSlot(const string &ipandport);
    //减少槽
    void cutSlot(const string &ipandport);
    //返回hash值所在槽代表的cache server的名称
    string compareSlot(const int hashVal);
    //增加cache server索引表，名称为ipandport
    void addListDcNode(const string &ipandport);
    //减少指定ipandport的cache server索引表
    void cutListDcNode(list<wuhuDc>::iterator iter);
    //getkey操作
    string getKey(const string &key);
    //setkey操作
    string setKey(const string &key);
    //根据ipandport查找索引表节点，返回指向节点的迭代器
    list<wuhuDc>::iterator findListDcNode(const string &ipandport);
    void adjustAndReply(const string &cmdStr);
    void expand(const string &ipandport, const string &cmdStr);
    void expand(const string &ipandport); // 重载
    void narrow(const string &ipandport, const string &cmdStr);
    void updateTimeval();
    void heartbeatMonitor();
    void updateHeartbeat(list<wuhuDc>::iterator iter);
    //test
	 //void show();
	 //int showhashval(const string &key);
	 //int shownum();
};

#endif