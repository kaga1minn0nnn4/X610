#include "main.hpp"
#include "board.hpp"
#include "controller.hpp"

uint32_t adc_count = 0;

void x610_hardware::it_timer_task() {
    static uint32_t count = 0;
    if (count++ > 1000) {
        count = 0;
        user_led.toggle();
    }

    static uint32_t sensor_count = 0;
    if (sensor_count++ > 100) {
        sensor_count = 0;
    }
}

void x610_hardware::adc_task() {
    controller.updateSensorValue();
}

bool sw_status_last = false;
bool driver_status = false;

void main_process(void) {
    x610_hardware::config();
    controller.config();
    // controller.setMotorBehavior(board::x610::MotorBehavior::enable);

    while (true) {
        // controller.printSensorValue();

        bool sw_status = x610_hardware::user_sw.read();
        if (sw_status && (!sw_status_last)) {
            driver_status = !driver_status;
            if (driver_status) {
                // x610_hardware::serial << "enable\n";
                controller.setMotorBehavior(board::x610::MotorBehavior::enable);
            } else {
                // x610_hardware::serial << "disable\n";
                controller.setMotorBehavior(board::x610::MotorBehavior::disable);
            }
        }
        sw_status_last = sw_status;
    }
}
