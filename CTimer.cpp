#include "CCore.h"
#include "CTimer.h"

bool CTimer::Pulse(float elapsedTime)
{
    if (this->committingSeppuku)
        return true;

    if (this->isPaused)
        return false;

    this->ticksElapsed += elapsedTime;

    if (this->ticksElapsed < this->intervalInTicks)
        return false;

    this->ticksElapsed = 0.0f;

    SQInteger top = sq_gettop(v);
    bool timerShouldDie = false;
    char buf[512];

    try
    {
        if (this->funcObj.IsNull())
        {
            snprintf(buf, sizeof(buf), "[Timer] Function object for '%s' is null -- stopping.", this->functionName.c_str());
            OutputError(buf);
            return (this->committingSeppuku = true);
        }

        sq_pushobject(v, this->funcObj.GetObject());

        sq_pushroottable(v);

        SQInteger nArgs = 1;
        for (Sqrat::Object& param : this->params)
        {
            sq_pushobject(v, param.GetObject());
            nArgs++;
        }

        if (SQ_FAILED(sq_call(v, nArgs, SQFalse, SQTrue)))
        {
            const SQChar* err = _SC("unknown error");
            sq_getlasterror(v);
            if (sq_gettype(v, -1) != OT_NULL)
            {
                sq_tostring(v, -1);
                sq_getstring(v, -1, &err);
            }

            snprintf(buf, sizeof(buf), "[Timer] Call to '%s' failed: %s", this->functionName.c_str(), err);
            OutputError(buf);
        }
    }
    catch (Sqrat::Exception& e)
    {
        snprintf(buf, sizeof(buf), "[Timer] Script exception in '%s': %s", this->functionName.c_str(), e.Message().c_str());
        OutputError(buf);
    }
    catch (const std::exception& e)
    {
        snprintf(buf, sizeof(buf), "[Timer] Native exception in '%s': %s", this->functionName.c_str(), e.what());
        OutputError(buf);
    }
    catch (...)
    {
        OutputError("[Timer] Critical native exception during execution.");
        timerShouldDie = true;
    }

    sq_settop(v, top);

    if (timerShouldDie)
        return (this->committingSeppuku = true);

    this->pulseCount++;
    if (this->maxNumberOfPulses > 0 && this->pulseCount >= this->maxNumberOfPulses)
        return (this->committingSeppuku = true);

    return false;
}

void  CTimer::Delete() { this->committingSeppuku = true; }
float CTimer::GetElapsedTicks() { return this->ticksElapsed; }
void  CTimer::Start() { this->isPaused = false; }
void  CTimer::Stop() { this->isPaused = true; }

SQInteger CTimer::ReleaseHook(SQUserPointer p, SQInteger size)
{
    (void)size;
    CTimer* timer = static_cast<CTimer*>(p);
    if (!timer) return 0;

    delete timer;
    return 1;
}

void RegisterTimer()
{
	Class<CTimer> c(v, "CTimer_INTERNAL");

	c.Prop(_SC("Elapsed"), &CTimer::GetElapsedTicks);

	c.Func(_SC("Delete"), &CTimer::Delete)
		.Func(_SC("Stop"), &CTimer::Stop)
		.Func(_SC("Start"), &CTimer::Start);

	c.Var(_SC("Interval"), &CTimer::intervalInTicks)
		.Var(_SC("Paused"), &CTimer::isPaused);

	RootTable(v).Bind(_SC("CTimer"), c);
}