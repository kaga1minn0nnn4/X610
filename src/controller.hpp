#pragma once

#include "RUR_CANBoard_Base/board/X610.hpp"
#include "board.hpp"
#include "common.hpp"

namespace board::x610::receiver {


class BLDCMotorController {
    static constexpr uint16_t kDutyMax = 90;

public:
    void config();

    void calculateSpeedResponse(float current, float time);

    void setVoltage(float voltage);
    void setTargetCurrent(float current);
    void setTargetVelocity(float velocity);
    void setTargetPosition(float position);

    void artificialCommutationControl();

    void positionInitialize(float velocity);

    void setMotorBehavior(MotorBehavior behavior);

    float getCurrent() const { return current_; }
    float getVelocity() const { return velocity_; }
    float getPosition() const { return position_; }

    void printSensorValue() {
        for (size_t i = 0 ; i < enc_logs_.size() ; i++) {
            // x610_hardware::serial << uvw_logs_[i].u << "," << uvw_logs_[i].v << "," << uvw_logs_[i].w << ",";
            x610_hardware::serial << dq_logs_[i].d << "," << dq_logs_[i].q << "\n";
            // x610_hardware::serial << enc_logs_[i].cos << "," << enc_logs_[i].sin << "\n";
            delay_ms(1);
        }
    }

    void enableprint() {
        x610_hardware::serial << "Begin...\n";
        enable_print_ = true;
    }


    void updateSensorValue();
    void resetcount() {ctl_count_ = 0;}
private:
    void controlTask();

    void trgoHandler() {
        updateSensorValue();
        controlTask();
    }

    void enableDriver();
    void disableDriver();

private:
    bool enable_print_ = false;

    float current_ = 0.f;
    float velocity_ = 0.f;
    float position_ = 0.f;

    std::array<float, 3> raw_current_uvw_offset_;

    std::array<x610_common::UVW, 100> uvw_logs_;
    std::array<x610_common::M2006EncoderValue, 100> enc_logs_;
    std::array<x610_common::DQ, 100> dq_logs_;
    uint16_t logs_count_ = 0;


    x610_common::UVW current_uvw_;
    x610_common::AB current_ab_;
    x610_common::DQ current_dq_;

    x610_common::M2006EncoderValue m2006_enc_;

    float raw_cos_;
    float raw_sin_;

    float target_voltage_;

    uint32_t ctl_count_;
};

}

extern board::x610::receiver::BLDCMotorController controller;
