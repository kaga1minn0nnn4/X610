#include "main.hpp"
#include "board.hpp"
#include "current_controller.hpp"
#include "angle_controller.hpp"
#include "debug.hpp"

void x610_hardware::it_timer_task() {
    angle_controller.update();

    static uint32_t count = 0;
    if (count++ > 1000) {
        count = 0;
        user_led.toggle();
        // x610_hardware::serial << controller.getControlCount() << "\n";
    }

    static uint32_t sensor_count = 0;
    if (sensor_count++ > 100) {
        sensor_count = 0;
    }
}

void main_process(void) {
    x610_hardware::config();
    angle_controller.config();

    while (true) {
        auto key = x610_hardware::serial.read().value_or(0);
        if (key == '\n' || key == '\r') {
            debug.setup();
            break;
        }
    }

    while (true) {
        debug.run();
    }
}
