#include <../lib/Scheduler/Scheduler.h>
#include <Arduino.h>
#include <TeensyThreads.h>
Scheduler scheduler;
Threads::Mutex Serial_lock;

// Placeholders for indexing demonstration
int blinkTaskIndex = -1;
int mathTaskIndex = -1;
int debugTaskIndex = -1;
int fastTaskIndex = -1;

unsigned long lastControlToggle = 0;
bool mathTaskEnabled = true;

// Task: LED Blink Task (runs in separate thread)
void blinkTask()
{
    static bool ledState = false;
    digitalWrite(LED_BUILTIN, ledState);
    ledState = !ledState;

    Threads::Scope scope(Serial_lock);
    Serial.println("[LED] Toggled.");
}

// Task: Math task every 100 ms
void mathTask()
{
    float result = 0.0f; // No need for volatile if using proper synchronization

    for (int i = 1; i < 500; ++i) {
        result += sqrtf(i);
        if (i % 25 == 0) { // Yield more frequently
            Threads::yield();
        }
    }

    // Minimize time spent holding the mutex
    {
        Threads::Scope scope(Serial_lock);
        Serial.println("[MATH] Calculation done.");
    }
    Threads::yield(); // Yield after completing calculation
}

// Task: Fast math task (runs every loop)
void fastTask()
{

    // Not using volatile here since this task runs every loop, and dummy is not used anywhere else.
    // Okay for compiler to optimize it.
    static float dummy = 0.0;
    dummy += 1.1;
    // Not printing every loop to avoid spamming
}

// Task: Serial debug print every 300 ms
void debugTask()
{
    Threads::Scope scope(Serial_lock);
    Serial.print("[DEBUG] Total tasks: ");
    Serial.print(scheduler.getTaskCount());
    Serial.print(" | Math task is ");
    Serial.println(mathTaskEnabled ? "ENABLED" : "DISABLED");
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    delay(1000);

    Serial.println(">>> Scheduler Demo Starting <<<");

    // Reorder tasks and be selective about threading
    scheduler.addTask(blinkTask, 500); // LED can be safely threaded
    scheduler.addTask(mathTask, 100, true); // Keep math in main thread initially
    scheduler.addTask(fastTask, 0); // Keep fast task in main
    scheduler.addTask(debugTask, 300, true); // Debug can be threaded

    // Update indices to match new order
    blinkTaskIndex = 0;
    mathTaskIndex = 1;
    fastTaskIndex = 2;
    debugTaskIndex = 3;

    Serial.println("[INIT] Tasks added.");
    Serial.print("[INIT] Total tasks registered: ");
    Serial.println(scheduler.getTaskCount());

    Serial.println("[INFO] Math task will toggle on/off every 5 seconds.");
}

void loop()
{
    static unsigned long lastDebug = 0;
    auto now = millis();

    scheduler.update();

    // Debug print every second to verify loop is running
    if (now - lastDebug >= 1000) {
        lastDebug = now;
        Threads::Scope scope(Serial_lock);
        Serial.println("Main loop running...");
    }

    // Your existing toggle logic
    if (now - lastControlToggle >= 5000) {
        lastControlToggle = now;
        Threads::Scope scope(Serial_lock); // Add mutex protection

        if (mathTaskEnabled) {
            scheduler.disableTask(mathTaskIndex);
            Serial.println("[CONTROL] Math task DISABLED.");
        } else {
            scheduler.enableTask(mathTaskIndex);
            Serial.println("[CONTROL] Math task ENABLED.");
        }

        mathTaskEnabled = !mathTaskEnabled;
    }
}
