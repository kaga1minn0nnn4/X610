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
            current_controller.setMotorBehavior(board::x610::MotorBehavior::enable);
        }
        if (key == 'd') {
            serial << "Disable\n";
            current_controller.setMotorBehavior(board::x610::MotorBehavior::disable);
        }

        if (key == '+') {
            dq_vol_ += 1.0;
            current_controller.setTargetPosition(dq_vol_ * 36);
            serial << "DQ Vol: " << dq_vol_ << "\n";
        }
        if (key == '-') {
            dq_vol_ -= 1.0;
            current_controller.setTargetPosition(dq_vol_ * 36);
            serial << "DQ Vol: " << dq_vol_ << "\n";
        }
        if (key == '0') {
            dq_vol_ = 0.f;
            current_controller.setTargetVelocity(dq_vol_);
            serial << "DQ Vol Reset\n";
        }
        if (key == 'c') {
            current_controller.calibration();
        }

        if (key == 'p') {
            enable_print_ = !enable_print_;
        }

        if (key == 's') {
            current_controller.calculateSpeedResponse(1.0f, 1.0f);
        }


        if (count++ > 50000 && enable_print_) {
            count = 0;
            // x610_hardware::serial << current_controller.getCurrentD() << ", " << current_controller.getCurrentQ() << "\n";
            x610_hardware::serial << current_controller.getPosition() << "\n";
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
