#pragma once

#include <bitset>
#include <string>
#include <map>
#include <string>
#include <vector>
#include <mutex>

#include "Db.h"

#define SLOTS_NUM 16384

using std::bitset;
using std::map;
using std::vector;

namespace PpServer
{

class DbServer
{
public:
    
    // 返回值为1，则为扩容，需要减少slots，需要用到sendKV；否则为缩容，只需增加slots
    int updateSlots(const std::string& newslots, std::string& sendKV);
    
    // 处理指令，参数怎么设置？有的一个又的两个又的三个这样子的
    // 处理client指令
    std::string handle_client_request(const std::string& mes);
    // 查询Id是否正确
    bool isRightId(const int& id);
    // 检查slot是否正确
    bool isRightSlot(const int& id, const int& slot);
    // 检查是否时本serverid
    bool isThisServer(const int& id);



private:
    int serverId_;
    // 需要一个标志，确定是否在扩容吧；缩容好像无所谓？
    // 是否需要一个标志确定DbServer正在运行？后期再设置？

    // vector<Db> Dbs_;
    // 同时也保存了从服务器id以及是谁的从服务器
    // ids_[0]是本服务器，ids_[1]是slave1，本服务器是其slave2；ids_[2]是slave2，本服务器是其slave1
    vector<int> ids_;
    map<int, Db> idToDb_;
    std::mutex dbmtx_;
};

}