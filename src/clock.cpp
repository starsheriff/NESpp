#include <chrono>
#include<iostream>
#include<chrono>
#include<vector>
#include<condition_variable>
#include<atomic>
#include<thread>
#include<list>

#include "clock.h"

Clock::Clock(Frequency freq) {
    period_ns = 1.0/freq.val * 1e9;
    std::cout << "clock period: " << period_ns << " ns\n";
}

Clock::~Clock() {
    // TODO: check if t was actually launched
    t.join();
}

void Clock::run() {
    t = std::thread(&Clock::count, this);
}

void Clock::count() {
    while(true) {
        auto t0 = std::chrono::high_resolution_clock::now();
        auto t1 = std::chrono::high_resolution_clock::now();
        while(true) {
            t1 = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0);
            if(elapsed.count() > period_ns) {
                std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count()
                     << " nanoseconds passed\n";
                break;
            } else {
                int sleep_for = (period_ns - elapsed.count())*0.5;
                // only sleep longer than 10ms
                if(sleep_for > 10000) {
                    std::this_thread::sleep_for(std::chrono::nanoseconds{sleep_for});
                }
            }
        }


        std::unique_lock<std::mutex> lck {sig_out->m};
        sig_out->clk_sig.notify_all();
    }
}

// Some ideas for a more functional style clock
//class Clock;
//struct Wire;

//void launch_clock();
//void launch_clock(Clock);

//void launch_clock(double freq, struct Wire);

