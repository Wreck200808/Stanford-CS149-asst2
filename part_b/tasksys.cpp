#include "tasksys.h"
#include <iostream>


IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->num_threads = num_threads;
    this->mutex = new std::mutex();
    this->finishtask_cv = new std::condition_variable();
    this->newtask_cv = new std::condition_variable();
    this->nextTaskID = 0;
    this->shutdown = false;
    for (int i = 0; i < num_threads; i++) {
        threads.push_back(std::thread(&TaskSystemParallelThreadPoolSleeping::threadFunc, this));
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    sync();
    shutdown = true;
    newtask_cv->notify_all();
    for (int i = 0; i < num_threads; i++) {
        threads[i].join();
    }
    delete mutex;
    delete finishtask_cv;
    delete newtask_cv;
}

void TaskSystemParallelThreadPoolSleeping::threadFunc(){
    while (true) {
        std::unique_lock<std::mutex> lock(*mutex);
        while (readyQueue.empty() && !shutdown) {
            newtask_cv->wait(lock);
        }
        if (shutdown) {
            return;
        }
        if (readyQueue.empty()) {
            continue;
        }
        Task* task = readyQueue.top();
        int task_num = task->task_num++;
        bool last_task = task->task_num == task->num_total_tasks;
        if (last_task){
            readyQueue.pop();
        }
        lock.unlock();
        task->runnable->runTask(task_num, task->num_total_tasks);
        lock.lock();
        task->task_finished++;
        if (task->task_finished == task->num_total_tasks) {
            completedTasks.insert(task->task_id);
            for (TaskID dep : task->reverseDeps) {
                unresolvedDependenciesCount[dep]--;
                if (unresolvedDependenciesCount[dep] == 0) {
                    readyQueue.push(waitingTasks[dep]);
                    waitingTasks.erase(dep);
                    newtask_cv->notify_all();
                }
            }
            if (completedTasks.size() == nextTaskID) {
                finishtask_cv->notify_all();
            }
            delete task;
        }
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    runAsyncWithDeps(runnable, num_total_tasks, std::vector<TaskID>());
    sync();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {

    //
    // TODO: CS149 students will implement this method in Part B.
    //
    std::unique_lock<std::mutex> lock(*mutex);
    Task* task = new Task(runnable, num_total_tasks, nextTaskID, deps);
    int unresolvedDependencies = 0;
    for (TaskID dep : deps) {
        if (completedTasks.find(dep) == completedTasks.end()) {
            unresolvedDependencies++;
            task->reverseDeps.push_back(dep);
        }
    }
    if (unresolvedDependencies == 0) {
        readyQueue.push(task);
        newtask_cv->notify_all();
    } else {
        unresolvedDependenciesCount[nextTaskID] = unresolvedDependencies;
        waitingTasks[nextTaskID] = task;
    }
    return nextTaskID++;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    std::unique_lock<std::mutex> lock(*mutex);
    while (completedTasks.size() < nextTaskID) {
        finishtask_cv->wait(lock);
    }
    return;
}
