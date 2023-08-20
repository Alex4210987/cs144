#include <cstdint>
#include <cstddef>

class Timer
{
private:
    uint64_t initial_RTO_ms_;
    uint64_t RTO_ms_;
    bool running_=false;
    size_t ms_since_last_tick_=0;
public:
    explicit Timer( uint64_t init_RTO ) : initial_RTO_ms_( init_RTO ), RTO_ms_( init_RTO ) {}
    void start();
    void stop();
    void reset();
    void tick(size_t ms_since_last_tick);
    bool is_running();
    bool is_expired();
    void double_RTO();
};

