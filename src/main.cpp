#include<iostream>

class Cpu {
public:
    Cpu();
    void run();
    void hard_reset();
    void cold_reset();

private:
    // special purpose registers
    // todo: type? 8bit or 16bit? -> char or short?
    int stack_pointer;
    int program_counter;
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
    std::cout << "cpu running\n";
}

int main(int argc, char** argv) {
    // instantiate the cpu
    Cpu cpu {};

    // start the cpu
    cpu.run();
}
