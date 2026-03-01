#pragma once

#include "controller.hpp"

namespace x610_hardware {

class Debug {

public:
    Debug() : dq_vol_{0.f} {}

    void setup() {
        serial << "Debug Mode Setup...\n";
    }

    void run() {
        static uint32_t count = 0;

        auto key = x610_hardware::serial.read().value_or(0);
        if (key == 'e') {
            serial << "Enable\n";
            controller.setMotorBehavior(board::x610::MotorBehavior::enable);
        }
        if (key == 'd') {
            serial << "Disable\n";
            controller.setMotorBehavior(board::x610::MotorBehavior::disable);
        }

        if (key == '+') {
            dq_vol_ += 0.005;
            controller.setVoltage(dq_vol_);
            serial << "DQ Vol: " << dq_vol_ << "\n";
        }
        if (key == '-') {
            dq_vol_ -= 0.005;
            controller.setVoltage(dq_vol_);
            serial << "DQ Vol: " << dq_vol_ << "\n";
        }
        if (key == '0') {
            dq_vol_ = 0.f;
            controller.setVoltage(dq_vol_);
            serial << "DQ Vol Reset\n";
        }
        if (key == 'b') {
            controller.enableprint();
        }
        if (key == 'v') {
            controller.disableprint();
        }
        if (key == 'p') {
            controller.printSensorValue();
        }
        if (key == 'c') {
            controller.calibration();
        }

        if (count++ > 500) {
            count = 0;
        }
    }

private:
    float dq_vol_;
    bool sw_status_last_;
    bool driver_status_;
};

}

x610_hardware::Debug debug;
