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
            dq_vol_ += 0.05;
            controller.setVoltage(dq_vol_);
            controller.setTargetVelocity(dq_vol_);
            serial << "DQ Vol: " << dq_vol_ << "\n";
        }
        if (key == '-') {
            dq_vol_ -= 0.05;
            controller.setVoltage(dq_vol_);
            controller.setTargetVelocity(dq_vol_);
            serial << "DQ Vol: " << dq_vol_ << "\n";
        }
        if (key == '0') {
            dq_vol_ = 0.f;
            controller.setVoltage(dq_vol_);
            controller.setTargetVelocity(dq_vol_);
            serial << "DQ Vol Reset\n";
        }
        if (key == 'c') {
            controller.calibration();
        }

        if (key == 'p') {
            enable_print_ = !enable_print_;
        }


        if (count++ > 50000 && enable_print_) {
            count = 0;
            // x610_hardware::serial << controller.getCurrentD() << ", " << controller.getCurrentQ() << "\n";
            x610_hardware::serial << controller.getVelocity() << "\n";
        }
    }

private:
    float dq_vol_;
    bool sw_status_last_;
    bool driver_status_;

    bool enable_print_;
};

}

x610_hardware::Debug debug;
