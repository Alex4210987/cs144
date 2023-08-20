#include"timer.hh"

void Timer::start()
{
    running_ = true;
    ms_since_last_tick_ = 0;
}

void Timer::stop()
{
    running_ = false;
}

bool Timer::is_running()
{
    return running_;
}

bool Timer::is_expired()
{
    return ms_since_last_tick_ >= RTO_ms_&&running_;
}

void Timer::reset()
{
    RTO_ms_ = initial_RTO_ms_;
}

void Timer::tick(size_t ms_since_last_tick)
{
    if (running_)
    {
        ms_since_last_tick_ += ms_since_last_tick;
    }
}

void Timer::double_RTO() 
{
    RTO_ms_ *= 2;
}