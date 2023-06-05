#pragma once

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <map>
#include <atomic>
// #include "lruMap.h"


#define SLOTS_NUM 16384

using std::bitset;
using std::multimap;
using std::list;
using std::unordered_map;
using std::pair;
using std::atomic_bool;

namespace PpServer
{

class Db
{
public:

    // using pairValueSlot = pair<std::string, int>;
    using itorPair = pair<list<std::string>::iterator, multimap<int, std::string>::iterator>;
    // using mapValue = pair<list<std::string>::iterator, pairValueSlot>;
    using mapValue = pair<itorPair, std::string>;

    // 对lrumap的操作
    // 是否超过maxSize
    bool isOverSize() {return lruList_.size() >= maxSize_;}
    // 查询key是否在lrumap中
    bool Contain(const std::string& key);
    // 取KV
    bool getKV(const std::string& key, std::string& value);
    // 设置KV（修改或添加）
    bool setKV(const std::string& key, const std::string& value, const int& slot);
    // 删除KV
    bool delKV(const std::string& key);



    // 对槽的操作
    // 计算置位槽节点的数量
    int getSlotsNum();
    // 更新slots
    void addSlots(const bitset<SLOTS_NUM>& newSlots);
    //扩容，slot减少，与newslots进行异或操作，得到要移走的slot，将对应的数据加入sendKV中
    void delSlots(bitset<SLOTS_NUM>& newSlots);
    // 检查slot是否置位
    bool testSlot(const int& slot);


private:

    void setSlotNum();


    int slots_num_;
    //ture表明在扩容，false表明未在扩容
    atomic_bool scaleUp_;
    bitset<SLOTS_NUM> slots_;
    int maxSize_;
    list<std::string> lruList_;
    std::unordered_map<std::string, mapValue> map_;
    //这个是为了在数据迁移时快速找到slot对应的key
    multimap<int, std::string> slotToKey_;
};

}