//
// Created by xantanium on 18/5/25.
//

#pragma once

#include <Arduino.h>
#include <TeensyThreads.h>
#include <functional>

/**
 * @struct Task
 * @brief Represents a scheduled task that can be executed, managed, and updated within a scheduler system.
 *
 * This structure defines a task with its associated callback function, execution interval, and other metadata.
 * It also includes support for enabling/disabling tasks as well as associating the task with a thread, if necessary.
 */
struct Task {
    using TaskCallback = std::function<void()>;

    Task() = default;
    Task(const TaskCallback &callback, const unsigned long interval, const bool enabled = true) :
        callback(callback), interval(interval), lastRun(0), isEnabled(enabled), threadId(-1) { }

    TaskCallback callback;
    unsigned long interval;
    unsigned long lastRun;
    bool isEnabled : 1;
    int threadId;
};
