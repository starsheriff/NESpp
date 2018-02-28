#pragma once
#include<mutex>
#include<condition_variable>

struct Wire {
    std::mutex m;
    std::condition_variable clk_sig{};

    // default constructor
    Wire(): m{}, clk_sig{} {};
};

struct Frequency { double val; };
