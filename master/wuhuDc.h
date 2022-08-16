#ifndef _WUHUDC_H_
#define _WUHUDC_H_

#include <unordered_map>
#include <vector>
#include <ctime>
#include <string>
#include "md5.h"
//test-----wuhudc头文件和源文件中的string写成string
// #include <iostream>

using namespace std;

#define numslots 16384

class wuhuDc
{
	friend class wuhuServer;
//friend void wuhuServer::narrow(const string &ipandport);
//friend void wuhuServer::expand(const string &ipandport);
//friend void wuhuServer::heartbeatMonitor();
//friend void wuhuServer::adjustAndReply();
//void wuhuServer::adjustAndReply(const string &cmdStr);
private:
    string ip;     //ip地址
    int port;       //端口
    // int sockfd = 0;     //socket
    int rehashidx = 0;      //缩扩容
    unordered_map<string, int> indexTable;     //索引表
    long long expireTime;   //心跳过期时间
    int heartbeatLen = 30;
    int survival = 1;       //心跳
public:
    wuhuDc();     //默认初始函数
    wuhuDc(const string &ip, long long timeval);      //构造函数，默认心跳最大时间
    wuhuDc(string ip, int heartbeatLen);        //构造函数，自定义心跳时间
    ~wuhuDc();
    void indexAdd(string key);
    // void indexDelete(string key);
    void moveIndex(pair<string, int>);
    void deleteIndex();
    int indexExist(string key);
    void setExpire();
	int isAlive();
	string getip();
};

#endif