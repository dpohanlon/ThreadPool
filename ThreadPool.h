#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <set>
#include <list>
#include <queue>
#include <thread>
#include <cassert>
#include <mutex>

#include <boost/function.hpp>
#include <boost/bind.hpp>

class ThreadPool
{

public:

    typedef boost::function<void(void)> func;

    ThreadPool(int n);
    ~ThreadPool(void);

    int addJob(func f);
    void wait(int jid);

    int getNThreads(void){ return nThreads; }

    template<typename T>
    void waitAll(T container)
    {
        for (auto & e : container) { wait(e); }
    }

    template<typename F, typename T>
    void map(F f, T container) 
    {
        std::set<int> jids;

        for (auto & e : container) {
            jids.insert(addJob(boost::bind(f, e)));
        }

        waitAll(jids);
    }

private:

    int nThreads;
    int jidCount;

    static bool stop;

    // Maybe have a wrapper around the queue to lock and unlock
    // the mutex on access?

    typedef std::pair<func, int> funcPair_t;
    typedef std::queue<funcPair_t> funcQ_t;
    typedef std::set<int> runningJids_t;
    typedef std::vector<std::thread *> threads_t;

    void join(void);

    static funcQ_t funcQ;
    static threads_t threads;
    static runningJids_t runningJids;

    static void idleLoop(void);

    static std::mutex qMutex;
    static std::mutex jidMutex;

};

#endif
