#include "Db.h"

using namespace PpServer;

bool Db::Contain(const std::string& key)
{
    auto mapItor = map_.find(key);
    if (mapItor == map_.end())
    {
        return false;
    }
    return true;
}

bool Db::getKV(const std::string& key, std::string& value)
{
    auto mapItor = map_.find(key);
    if (mapItor == map_.end())
        return false;
    
    value = mapItor->second.second;
    lruList_.erase(mapItor->second.first.first);
    lruList_.push_front(mapItor->first);
    mapItor->second.first.first = lruList_.begin();
    return true;
}

bool Db::delKV(const std::string& key)
{
    auto mapItor = map_.find(key);
    if (mapItor == map_.end())
        return false;

    //这里需要先erase map_中的对应元素，否则map_对应元素中存放的itor的行为会变得不可知
    //因此先将其赋值给listItor，便于我们在删除map_对应元素后还能在lruList_中找到该元素
    auto listItor = mapItor->second.first.first;
    auto multimapItor = mapItor->second.first.second;
    map_.erase(mapItor);
    lruList_.erase(listItor);
    slotToKey_.erase(multimapItor);

    return true;
}

bool Db::setKV(const std::string& key, const std::string& value, const int& slot)
{
    //查询是否包含：若包含，则为修改操作；否则为添加操作
    
    auto mapItor = map_.find(key);
    //包含，修改操作，不会发生数据淘汰
    if (mapItor != map_.end())
    {
        mapItor->second.second = value;
        lruList_.erase(mapItor->second.first.first);
        lruList_.push_front(mapItor->first);
        mapItor->second.first.first = lruList_.begin();
    }
    //不包含，添加操作，可能发生数据淘汰
    else{
        //判断缓存容量是否超过最大值
        if (isOverSize())
        {
            auto multimapItor = map_[lruList_.back()].first.second;
            map_.erase(lruList_.back());
            slotToKey_.erase(multimapItor);
            lruList_.pop_back();
        }
        //添加操作
        lruList_.push_front(mapItor->first);
        auto Itor = slotToKey_.insert(make_pair(slot, key));
        map_[key] = make_pair(make_pair(lruList_.begin(), Itor), value);
    }
    
    return true;
}

int Db::getSlotsNum()
{
    return slots_num_;
}

void Db::setSlotNum()
{
    slots_num_ = slots_.size();
}

// 缩容或初始化（当slots_num_<newslots.size()时执行此操作）
void Db::addSlots(const bitset<SLOTS_NUM>& newSlots)
{
    slots_ = newSlots;
}


void Db::delSlots(bitset<SLOTS_NUM>& newSlots)
{
    //检查这个函数的操作是否正确
    std::swap(newSlots, slots_);
    newSlots ^= slots_;
}

bool Db::testSlot(const int& slot)
{
    return slots_.test(slot);
}