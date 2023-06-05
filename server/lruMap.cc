#include "lruMap.h"

using namespace PpServer;

//注意，假如lruMap会被多个线程访问，则Del()和Set()需要对lruMap加锁后执行

//查询map_是否包含key
bool lruMap::Contain(const std::string& key)
{
    auto mapItor = map_.find(key);
    if (mapItor == map_.end())
    {
        return false;
    }
    return true;
}

//查找键值对
bool lruMap::Get(const std::string& key, std::string& value)
{
    auto mapItor = map_.find(key);
    if (mapItor == map_.end())
        return false;
    
    value = mapItor->second.second;
    lruList_.erase(mapItor->second.first);
    lruList_.push_front(mapItor->first);
    mapItor->second.first = lruList_.begin();
    return true;
}

//删除键值对
bool lruMap::Del(const std::string& key)
{
    auto mapItor = map_.find(key);
    if (mapItor == map_.end())
        return false;

    //这里需要先erase map_中的对应元素，否则map_对应元素中存放的itor的行为会变得不可知
    //因此先将其赋值给listItor，便于我们在删除map_对应元素后还能在lruList_中找到该元素
    auto listItor = mapItor->second.first;
    map_.erase(mapItor);
    lruList_.erase(listItor);

    return true;
}

//修改或添加键值对
bool lruMap::Set(const std::string& key, const std::string& value)
{
    //查询是否包含：若包含，则为修改操作；否则为添加操作
    
    auto mapItor = map_.find(key);
    //包含，修改操作，不会发生数据淘汰
    if (mapItor == map_.end())
    {
        mapItor->second.second = value;
        lruList_.erase(mapItor->second.first);
        lruList_.push_front(mapItor->first);
        mapItor->second.first = lruList_.begin();
    }
    //不包含，添加操作，可能发生数据淘汰
    else{
        //判断缓存容量是否超过最大值
        if (isOverSize())
        {
            map_.erase(lruList_.back());
            lruList_.pop_back();
        }
        //添加操作
        lruList_.push_front(mapItor->first);
        map_[key] = make_pair(lruList_.begin(), value);
    }
    
    return true;
}