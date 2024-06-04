#include "tasksys.h"


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
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
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
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->num_threads = num_threads;
    this->threads = std::vector<std::thread>(num_threads);
    this->counter_ = 0;
    this->mutex_ = new std::mutex();
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {
    delete this->mutex_;
}

void TaskSystemParallelSpawn::Threadfunction(IRunnable* runnable) {
    int task_id = -1;
    while (task_id < this->num_total_tasks) {
        this->mutex_->lock();
        task_id = this->counter_;
        this->counter_++;
        this->mutex_->unlock();
        if (task_id >= this->num_total_tasks) {
            break;
        }
        runnable->runTask(task_id, this->num_total_tasks);
    }
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    this->num_total_tasks = num_total_tasks;
    this->counter_ = 0;
    for (int i = 0; i < this->num_threads; i++) {
        this->threads[i] = std::thread(&TaskSystemParallelSpawn::Threadfunction, this, runnable);
    }
    for (int i = 0; i < this->num_threads; i++) {
        this->threads[i].join();
    }
    threads.clear();
}



TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
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
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
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

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->shutdown = true;
    this->cv->notify_all();
    for(int i = 0; i < this->num_threads; i++) {
        this->threads[i].join();
    }
    delete this->mutex_;
    delete this->cv;
    delete this->main_cv;
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    this->num_threads = num_threads;
    this->threads = std::vector<std::thread>(num_threads);
    this->mutex_ = new std::mutex();
    this->cv = new std::condition_variable();
    this->num_total_tasks = 0;
    this->shutdown = false;
    for(int i = 0; i < num_threads; i++) {
        this->threads[i] = std::thread(&TaskSystemParallelThreadPoolSleeping::Threadfunction, this);
    }
    this->main_cv = new std::condition_variable();
}

void TaskSystemParallelThreadPoolSleeping::Threadfunction() {
    while (true) {
        std::unique_lock<std::mutex> lk(*this->mutex_);
        this->cv->wait(lk);
        if (this->shutdown) {
            break;
        }
        int task_id = begin_counter_;
        begin_counter_++;
        lk.unlock();
        while(task_id < this->num_total_tasks) {
            this->runnable->runTask(task_id, this->num_total_tasks);
            lk.lock();
            end_counter_++;
            if (end_counter_ == this->num_total_tasks) {
                main_cv->notify_all();
            }
            task_id = this->begin_counter_;
            this->begin_counter_++;
            lk.unlock();
        }
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    this->num_total_tasks = num_total_tasks;
    this->begin_counter_ = 0;
    this->end_counter_ = 0;
    this->runnable = runnable;
    this->cv->notify_all();
    std::unique_lock<std::mutex> lk(*this->mutex_);
    if (this->end_counter_ < this->num_total_tasks) {
        this->main_cv->wait(lk);
    }
    lk.unlock();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
