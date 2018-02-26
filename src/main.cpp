#include<iostream>
#include<chrono>
#include<vector>
#include<condition_variable>
#include<atomic>
#include<thread>
#include<list>

struct Wire {
    std::mutex m;
    std::condition_variable clk_sig{};

    // default constructor
    Wire(): m{}, clk_sig{} {};
};


class Clock {
public:
    Clock();
    ~Clock();

    void run(); // starts the clock
    Wire* sig_out;

private:
    void count();
    std::thread t;
};


Clock::Clock() {
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
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(t1-t0);
            if(elapsed.count() > 60000) {
                std::cout << "break, elapsed: " << elapsed.count() << '\n';
                break;
            } else {
                std::this_thread::sleep_for(std::chrono::microseconds{60});
            }
        }

        std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count()
             << " nanoseconds passed\n";

        std::unique_lock<std::mutex> lck {sig_out->m};
        sig_out->clk_sig.notify_all();
    }
}


class Cpu {
public:
    Cpu();
    void run();
    void hard_reset();
    void cold_reset();
    int* ticks; // the input from the clock TODO: proper type
    void set_clk(std::condition_variable* c, std::mutex* m);
    std::condition_variable* cond;

    // clk input
    Wire* clk_cpu;

private:
    // special purpose registers
    // todo: type? 8bit or 16bit? -> char or short?
    int program_counter;
    int stack_pointer;
    int status_register;

    // general purpose registers
    char accumulator;     // 8-bit register
    char index_register_x; // 8-bit register
    char index_register_y; // 8-bit register

    // processor status
    // number of single bit flags --> model as enum?
    // todo: bitfields -> p. 12 in NESDoc
    char processor_status;

};

Cpu::Cpu() {
};

// todo:
void Cpu::run() {
    while(true) {
        std::unique_lock<std::mutex> lck{clk_cpu->m};
        clk_cpu->clk_sig.wait(lck);
        std::cout << "cpu cycle...\n";
    }
}

int main(int argc, char** argv) {
    // instantiate a new clock
    Clock clk {};

    // instantiate the cpu
    Cpu cpu {};

    // instantiate the wire between cpu and clock
    Wire clk_out{};
    // connect the wire
    clk.sig_out = &clk_out;
    cpu.clk_cpu = &clk_out;

    // start the clock and cpu
    clk.run();
    cpu.run();

}
