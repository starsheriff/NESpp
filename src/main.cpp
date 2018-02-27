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

struct Frequency { double val; };

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

// 7 6 5 4 3 2 1 0
// | | | | | | | |
// N V   B D I Z C
//
// C - CarryFlag
// Z - ZeroFlag
// I - InterruptDisable
// D - Decimal Mode
// B - BreakCommand
// V - Overflow Flag
// N - Negative Flag
//
// This enum can be used as bit flags with bitwise operations.
enum ProcessorStatus: char {
    CarryFlag        = 0,
    ZeroFlag         = 1,
    InterruptDisable = 2,
    DecimalMode      = 4,
    BreakCommand     = 8,
    OverflowFlag_0   = 16,
    OverflowFlag_1   = 32,
    NegativeFlag     = 64,
};

constexpr ProcessorStatus set_bit(ProcessorStatus a, ProcessorStatus b) {
    auto aa = static_cast<int>(a);
    auto bb = static_cast<int>(b);

    return static_cast<ProcessorStatus>(aa | bb);
}

ProcessorStatus init_bits(std::initializer_list<ProcessorStatus> list) {
    ProcessorStatus reg_p = static_cast<ProcessorStatus>(0);
    for (auto p : list) {
        reg_p = set_bit(reg_p, p);
    }
    return reg_p;
}


class Cpu {
public:
    Cpu();
    void run();
    void powerup();
    void reset();
    int* ticks; // the input from the clock TODO: proper type
    void set_clk(std::condition_variable* c, std::mutex* m);
    std::condition_variable* cond;

    // clk input
    Wire* clk_cpu;

private:
    // special purpose registers
    unsigned int program_counter;
    char stack_pointer;
    ProcessorStatus reg_p;

    // general purpose registers
    char accumulator;     // 8-bit register
    char reg_x; // 8-bit register
    char reg_y; // 8-bit register
};

Cpu::Cpu() {
};

void Cpu::powerup() {
    reg_p = init_bits({
                ProcessorStatus::InterruptDisable,
                ProcessorStatus::BreakCommand,
                ProcessorStatus::OverflowFlag_0,
            });

    reg_x = 0;
    reg_y = 0;

    stack_pointer = 0xFD;

    // TODO: memory
}

void Cpu::reset() {
    // do nothing to registers A, X and Y
    stack_pointer -= 3;

    reg_p = set_bit(reg_p, ProcessorStatus::InterruptDisable);

    // TODO: memory
}

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
    Clock clk {Frequency{1}}; // 1Hz
    //Clock clk {Frequency{1.79e6}}; // NES frequency: 1.79 MHz

    // instantiate the cpu
    Cpu cpu {};

    // instantiate the wire between cpu and clock
    Wire clk_out{};
    // connect the wire
    clk.sig_out = &clk_out;
    cpu.clk_cpu = &clk_out;

    // powerup
    cpu.powerup();

    // start the clock and cpu
    clk.run();
    cpu.run();

}
