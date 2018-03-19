#ifndef TRACERATOPS_TIMER_H
#define TRACERATOPS_TIMER_H

#include <chrono>

struct timer
{
public:

    timer() = default;
    ~timer() = default;

    void start()
    {
        NumIterations = 0;
        Start = std::chrono::high_resolution_clock::now();
    }

    void new_iteration()
    {
        NumIterations += 1;
    }

    void end()
    {
        End = std::chrono::high_resolution_clock::now();
    }

    double get_seconds_elapsed()
    {
        int64_t TotalNanoseconds = get_nanoseconds_elapsed();
        return double(TotalNanoseconds) / 1000000000.0;
    }

    int64_t get_milliseconds_elapsed()
    {
        int64_t TotalMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(End - Start).count();
        return TotalMilliseconds;
    }

    int64_t get_nanoseconds_elapsed()
    {
        int64_t TotalNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(End - Start).count();
        return TotalNanoseconds;
    }

    int64_t get_nanoseconds_elapsed_per_iteration()
    {
        assert(NumIterations > 0);
        return get_nanoseconds_elapsed() / NumIterations;;
    }

private:

    std::chrono::high_resolution_clock::time_point Start;
    std::chrono::high_resolution_clock::time_point End;

    uint64_t NumIterations = 0;

};

#endif // TRACERATOPS_TIMER_H
