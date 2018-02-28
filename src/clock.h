#pragma once
#include<mutex>
#include<condition_variable>
#include<thread>

struct Frequency { double val; };

struct Wire {
    std::mutex m;
    std::condition_variable clk_sig{};

    // default constructor
    Wire(): m{}, clk_sig{} {};
};

class Clock {
public:
    Clock(Frequency freq);
    ~Clock();

    void run(); // starts the clock
    Wire* sig_out;

private:
    long int period_ns;
    void count();
    std::thread t;
};

