#include "Scheduler.h"
#include <csignal>

//
[[noreturn]] void Scheduler::threadRunner(void* arg)
{
    // The void pointer is casted explicitly to `Task*` to retrieve the callback.
    // *update: switched to reinterpret cast to validate types.
    const auto task = reinterpret_cast<Task*>(arg);

    if (!task)
        return;

    // run thread if it is enabled
    while (true) {
#if defined(POLLING)
        auto now = millis();
        if ((now - task->lastRun >= task->interval) && task->isEnabled) {
            task->lastRun = now;
            task->callback();
        }
        threads.yield();
#else
        // TODO: Compare these two logics

        if (task->isEnabled) {
            task->callback();
            // if the thread is an interval based, delay for the set time else yield and exit the thread.
            task->interval > 0 ? threads.delay(task->interval) : Threads::yield();
        } else {
            Threads::yield();
        }
#endif
    }
}
//

//
void Scheduler::addTask(const Task::TaskCallback& callback, const unsigned long interval, const bool useThread)
{
    // using emplace_back to avoid the move-copy gimmick of push_back.
    tasks.emplace_back(callback, interval);
    Task& newTask = tasks.back();

    if (useThread) {
        newTask.threadId = threads.addThread(threadRunner, &newTask);
    }
}
//

//
void Scheduler::update()
{
    unsigned long now = millis();

    // Iterate over the tasks vector to execute enabled and non-threaded tasks based on their intervals.
    for (auto& task : tasks) {
        if (!task.isEnabled || task.threadId != -1)
            continue;

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
