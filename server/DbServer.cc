#include "DbServer.h"

using namespace PpServer;

std::string DbServer::handle_client_request(const std::string& mes)
{
    // cmd的格式：server编号、slot号、命令类型、key、value
    
    // 先查id是否在本服务器上
    std::string idStr = mes.substr(0, 2);
    int id = std::stoi(idStr);
    if (!isRightId(id))
    {
        return "WrongServerId";
    }

    // 查看slot槽值对否（扩容时可能会不对）
    std::string slotStr = mes.substr(2, 5);
    std::string cmd = mes.substr(7, 3);
    std::string key = mes.substr(10, 8);
    int slot = std::stoi(slotStr);
    {
        std::lock_guard<std::mutex> lock(dbmtx_);
        if (!isRightSlot(id, slot))
        {
            return "WrongSlot";
        }

        // 获取操作命令
        // std::string cmd = mes.substr(7, 3);
        // std::string key = mes.substr(10,8);
        if (cmd == "GET")
        {
            std::string value;
            if (idToDb_[id].getKV(key, value))
            {
                return value;
            }
            return "GETKeyNotFound";
        }
        if (cmd == "DEL")
        {
            if (idToDb_[id].delKV(key))
            {
                return "DELDown";
            }
            return "DELKeyNotFound";
        }
        if (cmd == "SET")
        {
            // set操作要检查是不是本机serverid
            if (!isThisServer(slot))
            {
                return "Moved";
            }
            // set是不是可以不用回复client哈哈哈 
            std::string value = mes.substr(18,8);
            idToDb_[serverId_].setKV(key, value, slot);  
            return "SETDONE" ;
        }
    }


}

bool DbServer::isRightId(const int& id)
{
    for (auto itor = ids_.begin(); itor != ids_.end(); itor++)
    {
        if (id == *itor) return true;
    }
    return false;
}

bool DbServer::isRightSlot(const int& id, const int& slot)
{
    return idToDb_[id].testSlot(slot);
}

bool DbServer::isThisServer(const int& id)
{
    return id == serverId_;
}



//还没判断是否在扩容之中呢。