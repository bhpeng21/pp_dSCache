#pragma once

#include <unordered_map>
#include <list>
#include <string>

using std::list;
using std::unordered_map;
using std::pair;

namespace PpServer
{

class lruMap
{
public:
    // using pairValueSlot = pair<std::string, int>;
    // using mapValue = pair<list<std::string>::iterator, pairValueSlot>;
    using mapValue = pair<list<std::string>::iterator, std::string>;
    
    lruMap() = default;
    lruMap(int maxSize) : maxSize_(maxSize){}
    //这个析构函数可以默认生产吗？
    ~lruMap() = default;

    //是否超过maxSize
    bool isOverSize() {return lruList_.size() >= maxSize_;}

    //查询key是否在lruMap中
    bool Contain(const std::string& key);

    //查询key对应的value
    bool Get(const std::string& key, std::string& value);
	
    //删除key对应的value
    bool Del(const std::string& key);

    //添加或修改kv对
    bool Set(const std::string& key, const std::string& value);


private:
    int maxSize_;
    list<std::string> lruList_;
    std::unordered_map<std::string, mapValue> map_;
};

}