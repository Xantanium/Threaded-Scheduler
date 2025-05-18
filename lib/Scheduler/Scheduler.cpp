#include "Scheduler.h"
#include <csignal>

//
void Scheduler::threadRunner(void *arg) {

    // The void pointer is casted explicitly to `Task*` to retrieve the callback.
    const auto task = static_cast<Task*>(arg);

    if (!task) return;

    // run thread if it is enabled
    while (true) {
        unsigned long now = millis();
        if ((now - task->lastRun >= task->interval) && task->isEnabled){
            task->lastRun = now;
            task->callback();
        }
        threads.yield();
    }
}
//

//
void Scheduler::addTask(const Task::TaskCallback &callback, const unsigned long interval, const bool useThread) {
    tasks.emplace_back(callback, interval);
    Task &newTask = tasks.back();

    if (useThread) {
        newTask.threadId = threads.addThread(threadRunner, &newTask);
    }
}
//

//
void Scheduler::update() {
    unsigned long now = millis();

    // Iterate over the tasks vector to execute enabled and non-threaded tasks based on their intervals.
    for (auto& task : tasks) {
        if (!task.isEnabled || task.threadId != -1) continue;

        if (task.interval == 0) {
            task.callback();
            continue;
        }

        if (now - task.lastRun >= task.interval) {
            task.callback();
            task.lastRun = now;
        }
    }
}
//