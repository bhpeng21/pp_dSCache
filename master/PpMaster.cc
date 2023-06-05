#include "PpMaster.h"

using namespace ppMaster;
using namespace mymuduo;
using namespace mymuduo::net;

std::string PpMaster::handle_client_request(const std::string& mes)
{
    // std::string cmd = mes.substr(0, 3);
    // std::string key = mes.substr(3, 8);
    //mes:slot+cmd+key+value
    std::string slotStr = mes.substr(0, 5);
    int slot = std::stoi(slotStr);
    RWmutex_.lock_shared();
    int id = slotsToId_[slot];
    std::string IpPort = idToAddr_[id].toIpPort();
    RWmutex_.unlock_shared();
    return IpPort;
}

//检查slot值是否合法，不过可以直接在客户端完成
// bool PpMaster::checkSlot(const int& slot)
// {
//     if (slot < SLOTS_NUM) return true;
//     return false;
// }

//范围性的置位
void PpMaster::rangeSetSlots(const int& serverId, const int& start, const int& end)
{
    std::bitset<SLOTS_NUM> mask = ((1 << (end - start)) - 1) << start;
    idToSlots_[serverId] |= mask;
}

//复位操作，第二个参数为需要复位的slot的个数，返回值为被复位的slots位置的数组
std::bitset<SLOTS_NUM> PpMaster::resetSlots(int serverId, int n) 
{
    std::bitset<16384> ret;  // 新的 bitset 对象，初始值为 0

    const int block_size = 64;  // 块的大小
    const int num_blocks = 16384 / block_size;  // 块的数量

    uint64_t blocks[num_blocks];  // 保存每个块的值
    uint64_t masks[num_blocks];  // 保存每个块的掩码
    int positions[num_blocks][block_size];  // 保存每个块已置位的位置
    int counts[num_blocks];  // 保存每个块已置位的数量

    int count = 0;  // 记录已复位的位置数量

    // 将 bitset 对象分成若干个块，并分别处理每个块
    for (int i = 0; i < num_blocks && count < n; ++i) {
        blocks[i] = idToSlots_[serverId].to_ullong() >> (i * block_size);
        masks[i] = (1ULL << block_size) - 1;

        int count_i = 0;
        while (blocks[i] != 0 && count_i < block_size) {
            int pos = __builtin_ctzll(blocks[i]);
            positions[i][count_i++] = i * block_size + pos;
            blocks[i] &= (blocks[i] - 1);
            if (++count == n) break;  // 已复位 n 个位置，停止对后面的块的遍历
        }

        counts[i] = count_i;

        for (int j = 0; j < count_i; ++j) {
            int pos = positions[i][j];
            ret.set(pos);
            masks[i] &= ~(1ULL << pos);
        }

        idToSlots_[serverId] &= std::bitset<16384>(~(masks[i] << (i * block_size)));
    }

    return ret;
}

void set_vector_bits(std::bitset<16384>& bits, const std::bitset<16384>& reset_bits, std::vector<int>& vec) {
    const int block_size = 64;  // 块的大小
    const int num_blocks = 16384 / block_size;  // 块的数量

    uint64_t blocks[num_blocks];  // 保存每个块的值
    int positions[num_blocks][block_size];  // 保存每个块已置位的位置
    int counts[num_blocks];  // 保存每个块已置位的数量

    // 将 bitset 对象分成若干个块，并分别处理每个块
    for (int i = 0; i < num_blocks; ++i) {
        blocks[i] = reset_bits.to_ullong() >> (i * block_size);

        int count_i = 0;
        while (blocks[i] != 0 && count_i < block_size) {
            int pos = i * block_size + __builtin_ctzll(blocks[i]);
            positions[i][count_i++] = pos;
            blocks[i] &= (blocks[i] - 1);
        }

        counts[i] = count_i;

        for (int j = 0; j < count_i; ++j) {
            int pos = positions[i][j];
            vec[pos] = 2;
        }
    }
}

void set_vector_bits(std::bitset<16384>& bits, const std::bitset<16384>& reset_bits, std::vector<int>& vec) {
    const int block_size = 64;  // 块的大小
    const int num_blocks = 16384 / block_size;  // 块的数量

    // 将 bitset 对象分成若干个块，并分别处理每个块
    for (int i = 0; i < num_blocks; ++i) {
        unsigned long long block = reset_bits.to_ulong() >> (i * block_size);
        std::for_each(
            std::begin(vec) + i * block_size,
            std::begin(vec) + (i + 1) * block_size,
            [&block](int& value) {
                if (block & 1ull) {
                    value = 2;
                }
                block >>= 1;
            }
        );
    }

    // 更新 bitset 对象
    bits = std::bitset<16384>(vec.data());
}


