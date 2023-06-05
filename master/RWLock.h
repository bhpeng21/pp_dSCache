#include <shared_mutex>
#include <mutex>
#include <condition_variable>


namespace ppMaster
{

class RWLock : public std::shared_mutex {
public:
    void lock() {
        std::unique_lock<std::mutex> lock(m);
        has_writer = true;
        cv_writers.notify_all();
        std::shared_mutex::lock();
    }

    void unlock() {
        std::unique_lock<std::mutex> lock(m);
        has_writer = false;
        cv_readers.notify_all();
        std::shared_mutex::unlock();
    }

    void lock_shared() {
        std::unique_lock<std::mutex> lock(m);
        cv_readers.wait(lock, [&] { return !has_writer; });
        std::shared_mutex::lock_shared();
    }

    void unlock_shared() {
        std::shared_mutex::unlock_shared();
    }

private:
    std::mutex m;
    std::condition_variable cv_readers;
    std::condition_variable cv_writers;
    bool has_writer = false;
};

}