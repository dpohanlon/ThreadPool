#include "ThreadPool.h"

bool ThreadPool::stop;

ThreadPool::funcQ_t       ThreadPool::funcQ;
ThreadPool::threads_t     ThreadPool::threads;
ThreadPool::runningJids_t ThreadPool::runningJids;
std::mutex                ThreadPool::qMutex;
std::mutex                ThreadPool::jidMutex;

ThreadPool::ThreadPool(int n) : nThreads(n), jidCount(0)
{
    this->stop = false;

    for (int i = 0; i < nThreads; ++i){
        threads.push_back(new std::thread(idleLoop));
    }
}

ThreadPool::~ThreadPool(void)
{
    this->join();

    std::vector<std::thread *>::iterator itr;
    for(itr = threads.begin(); itr!= threads.end(); ++itr){
        delete *itr;
        *itr = nullptr;
    }
}

int ThreadPool::addJob(func f)
{
    qMutex.lock();
    funcQ.push(std::make_pair(f, ++jidCount));
    qMutex.unlock();
    
    return jidCount;
}

void ThreadPool::join(void)
{
    // Stop the infinite idle loop
    stop = true;

    for (auto t : threads) { t->join(); }
}

void ThreadPool::wait(int jid)
{
    // Block until job with job id jid finishes

    while (1){

        bool running = runningJids.find(jid) != runningJids.end();

        if (!running){

            // jids monotonically increase in Q
            bool inQ = !funcQ.empty() && !funcQ.front().second > jid;

            if (!inQ || funcQ.empty()){
                return;
            }
        }

    }
}

void ThreadPool::idleLoop(void)
{
    while (1){

        qMutex.lock();

        if (!funcQ.empty()){

            // Get a local copy of the function closure and the jid

            func localFunc = (funcQ.front()).first;
            int localJid = (funcQ.front()).second;

            funcQ.pop();
            qMutex.unlock();

            // Push the jid to the list of running jobs

            jidMutex.lock();
            runningJids.insert(localJid);
            jidMutex.unlock();

            assert(localFunc);
            localFunc();

            // Job done, remove the job id from the list of running jobs

            jidMutex.lock();
            runningJids.erase(localJid);
            jidMutex.unlock();
        }
        else {
            qMutex.unlock();
        }

        if (funcQ.empty() && stop){ return; }

    }
}
