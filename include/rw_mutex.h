#ifndef _QUICPP_RD_MUTEX_
#define _QUICPP_RD_MUTEX_

#include <mutex>
#include <condition_variable>
#include <cstdint>
#include <atomic>

namespace quicpp {

const int rw_mutex_max_readers = 1 << 30;

class rw_mutex {
private:
    std::mutex _mutex;
    std::condition_variable sem_writer;
    std::condition_variable sem_reader;
    std::atomic<int32_t> reader_count;
public:
    rw_mutex()
        : reader_count(0) {}

    void read_lock() {
        std::unique_lock<std::mutex> locker(this->_mutex);

        this->sem_reader.wait(locker,
                              [this] () -> bool {
                                  return this->reader_count >= 0;
                              });
        this->reader_count++;
    }

    void read_unlock() {
        if ((this->reader_count -= 1) == 0) {
            this->sem_writer.notify_one();
        }
    }

    void lock() {
        std::unique_lock<std::mutex> locker(this->_mutex);

        this->sem_writer.wait(locker,
                              [this] () -> bool {
                                  return this->reader_count == 0;
                              });
        this->reader_count -= rw_mutex_max_readers;
    }

    void unlock() {
        this->reader_count += rw_mutex_max_readers;
        this->sem_reader.notify_one();
        this->sem_writer.notify_one();
    }

    std::mutex &mutex() {
        return this->_mutex;
    }
};

class reader_lock_guard {
private:
    quicpp::rw_mutex &rd_mutex;
public:
    reader_lock_guard(quicpp::rw_mutex &rd_mutex)
        : rd_mutex(rd_mutex) {
        this->rd_mutex.read_lock();
    }

    ~reader_lock_guard() {
        this->rd_mutex.read_unlock();
    }
};

class writer_lock_guard {
private:
    quicpp::rw_mutex &rd_mutex;
public:
    writer_lock_guard(quicpp::rw_mutex &rd_mutex)
        : rd_mutex(rd_mutex) {
        this->rd_mutex.lock();
    }

    ~writer_lock_guard() {
        this->rd_mutex.unlock();
    }
};

}

#endif
