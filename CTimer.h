#pragma once
#include "main.h"
#include <vector>
#include <string>
#include <sqrat.h>

class CTimer
{
public:
    CTimer()
        : ticksElapsed(0.0f)
        , intervalInTicks(0.0f)
        , maxNumberOfPulses(0)
        , pulseCount(0)
        , isPaused(false)
        , committingSeppuku(false)
    {}

    bool Pulse(float elapsedTime);

    // --- Script-exposed API ---
    void Delete();
    void Start();
    void Stop();
    float GetElapsedTicks();

    Sqrat::Object funcObj;

    std::string functionName;

    // Called by Squirrel itself once the instance's refcount drops to zero
    static SQInteger ReleaseHook(SQUserPointer p, SQInteger size);

public:
    float ticksElapsed;
    float intervalInTicks;

    unsigned int maxNumberOfPulses; // 0 = infinite
    unsigned int pulseCount;

    bool isPaused;
    bool committingSeppuku;

    std::vector<Sqrat::Object> params;
};

void RegisterTimer();