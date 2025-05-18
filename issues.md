# Known Issues with the library:

## 1. If all tasks are offloaded to threads, loop is never executed.

Tried adding all tasks to threads, the program did not enter loop.

## 2. If threaded task is added first to the scheduler, program locks.

This happens because `threadRunner` contains a while(1) that starts to execute immediately,
when initialization of the task is done. This prevents further tasks from getting inited.

## 3. Large tasks block threads

Making `mathTask()` threaded is causing a lot of locks. Possible reasons:
1. Serial mutex clashes, would have to be managed very carefully.
2. Time spent in the task maybe high, needing frequent yield() s inside the task.

// if that is the case, then the wrapper being for teensythreads completely loses its purpose.