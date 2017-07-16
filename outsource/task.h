#include <functional>
#include <thread>
#include <list>
#include <mutex>
#include <atomic>
class Processer
{
public:
    virtual bool Process() = 0;
};

class Task
{
public:
    void Start(int timeus);
    void Stop();
    void Run();
    void AddProcesser( Processer * );
    void RemoveProcesser( Processer* );
private:
    std::thread _thread;
    std::list<Processer*> _processers;
    std::mutex _lock;
    std::atomic<bool> _run_flag{ false };
    int _timeus;
};

