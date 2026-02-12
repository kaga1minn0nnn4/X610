#include "main.hpp"
#include "board.hpp"
#include "controller.hpp"

void x610::it_timer_task() {
    static uint32_t count = 0;
    if (count++ > x610::kTimerInterruptFreq) {
        count = 0;
        x610::user_led.toggle();
    }
}

void x610::ctl_timer_task() {
}

void main_process(void) {
    x610::config();
    x610::start();

    while (true) {

    }
}
