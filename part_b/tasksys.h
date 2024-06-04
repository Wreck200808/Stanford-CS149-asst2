#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <queue>
#include <unordered_set>
#include <algorithm>

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial : public ITaskSystem
{
public:
    TaskSystemSerial(int num_threads);
    ~TaskSystemSerial();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                            const std::vector<TaskID> &deps);
    void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn : public ITaskSystem
{
public:
    TaskSystemParallelSpawn(int num_threads);
    ~TaskSystemParallelSpawn();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                            const std::vector<TaskID> &deps);
    void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning : public ITaskSystem
{
public:
    TaskSystemParallelThreadPoolSpinning(int num_threads);
    ~TaskSystemParallelThreadPoolSpinning();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                            const std::vector<TaskID> &deps);
    void sync();
};

struct Task
{
public:
    Task(IRunnable *runnable, int num_total_tasks, int task_id, std::vector<TaskID> deps) : runnable(runnable), num_total_tasks(num_total_tasks), task_id(task_id), deps(deps), task_num(0) {}
    ~Task(){};
    IRunnable *runnable;
    int num_total_tasks;
    int task_id;
    std::vector<TaskID> deps;
    int task_num;
    bool operator<(const Task &other) const
    {
        // compare the largest dep task_id of the two tasks
        return *std::max_element(deps.begin(), deps.end()) < *std::max_element(other.deps.begin(), other.deps.end());
    }
};

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping : public ITaskSystem
{
public:
    TaskSystemParallelThreadPoolSleeping(int num_threads);
    ~TaskSystemParallelThreadPoolSleeping();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                            const std::vector<TaskID> &deps);
    void sync();
    void threadFunc();

private:
    int num_threads;
    TaskID nextTaskID;
    bool shutdown;
    // create a mutex for shutdown adn nextTaskID
    std::mutex *mutex;
    std::vector<std::thread> threads;
    std::unordered_map<TaskID, Task *> waitingTasks;                     // Tasks that are waiting for dependencies
    std::unordered_map<TaskID, int> unresolvedDependenciesCount;         // Count of unresolved dependencies per task
    std::unordered_map<TaskID, std::vector<TaskID>> reverseDependencies; // Tasks that depend on a given task
    std::queue<Task *> readyQueue;
    std::condition_variable *newtask_cv;
    std::condition_variable *finishtask_cv;
    std::unordered_set<TaskID> completedTasks;
};
#endif
