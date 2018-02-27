#include<iostream>
#include<chrono>
#include<vector>
#include<condition_variable>
#include<atomic>
#include<thread>
#include<list>

#include "clock.h"

enum class AddressingMode {
    One,
    Two,
    Three,
};

struct MemoryAddress { unsigned int addr; };

class Memory{
public:
    Memory(int bytes);
    void write(AddressingMode m, MemoryAddress addr);
    char read(AddressingMode m, MemoryAddress addr);
private:
    std::vector<char> mem;
};

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
enum class ProcessorStatus: char {
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
    const char clock_divider{12};
    char clock_counter{0};

    // special purpose registers
    unsigned int program_counter;
    char stack_pointer;
    ProcessorStatus reg_p;

    // general purpose registers
    char accumulator;     // 8-bit register
    char reg_x; // 8-bit register
    char reg_y; // 8-bit register

    // memory access
    void mem_write(MemoryAddress addr);
    char mem_read(MemoryAddress addr);
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

    // instantiate and wire up memory
    Memory mem{64*1024};
    // or this?
    std::vector<char> mem2(64*1024);

    // powerup
    cpu.powerup();

    // start the clock and cpu
    clk.run();
    cpu.run();

}
