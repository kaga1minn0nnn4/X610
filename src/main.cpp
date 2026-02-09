#include "main.hpp"
#include "board.hpp"

void x610::it_timer_task() {
    static uint32_t count = 0;
    if (count++ > x610::kTimerInterruptFreq) {
        count = 0;
    }
}

void main_process(void) {
    x610::config();

    while (true) {

    }
}
