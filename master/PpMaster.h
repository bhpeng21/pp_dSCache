#include <map>
#include <string>
#include <bitset>
#include <vector>
#include <string>
#include <atomic>
#include <queue>
#include <algorithm>

#include "RWLock.h"
#include "net/InetAddress.h"

#define SLOTS_NUM 16384

using std::map;
using std::bitset;
using std::vector;
using std::atomic_bool;
using std::queue;
using namespace ppMaster;
using namespace mymuduo;
using namespace mymuduo::net;

namespace ppMaster
{

class PpMaster
{
public:
    PpMaster();
    std::string handle_client_request(const std::string& mes);
    // 这个返回值需要确定一下的
    void handle_server_request(const std::string&mes);

    // 对哈希槽进行范围置位
    // 对start~end-1范围的槽进行范围置位
    void rangeSetSlots(const int& serverId, const int& start, const int& end);
    // 对start~end-1范围的槽进行范围复位,并返回被复位元素位置bitset
    std::bitset<SLOTS_NUM> resetSlots(int serverId, int n);
    // 将bitset中的置位元素在vector对应位置设置指定id


private:

    // bool checkSlot(const int& slot);
    // 待准备就绪后，running调整为
    atomic_bool running_;
    int dbNum_;
    int nowDbNum_;

    //存随时可以上线的server，假如可以的话，建议用队列。但是需要满足可以删除人意itor后，itor不失效
    //

    //dbserver对应的id，由服务器io线程设置，客户端io线程可读
    //这个InetAddress从哪里来（muduo中的哪个可以提供）
    map<int, InetAddress> idToAddr_;
    queue<InetAddress> readyServer;
    map<int, bitset<SLOTS_NUM>> idToSlots_;
    vector<int> slotsToId_;
    RWLock RWmutex_;
};

}