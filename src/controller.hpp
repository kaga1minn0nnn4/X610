#pragma once

#include "RUR_CANBoard_Base/board/X610.hpp"
#include "board.hpp"
#include "common.hpp"
#include "RUR_STM_CommonLib/PIDLib/PID.hpp"

namespace board::x610::receiver {

constexpr BackCalculationPI_D::Parameter d_param = {
    .kp = 0.03,
    .ki = 20.0f,
    .kd = 0.0f,
    .control_frequency = 20000.f,
    .manipulated_value_limit = 1.0f,
    .feed_forward = 0.0f
};

constexpr BackCalculationPI_D::Parameter q_param = {
    .kp = 0.03,
    .ki = 20.0f,
    .kd = 0.0f,
    .control_frequency = 20000.f,
    .manipulated_value_limit = 1.0f,
    .feed_forward = 0.0f
};

class BLDCMotorController {
    static constexpr uint16_t kDutyMax = 90;

public:
    BLDCMotorController() : d_pid_(d_param), q_pid_(q_param) {}

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

    void calibration() {
        x610_hardware::serial << "Calibration..." << "\n";

        m2006_enc_.reset_offset();
        is_calibration_ = true;
        enableDriver();

        delay_ms(500);

        m2006_enc_.angle_offset = m2006_enc_.angle;

        disableDriver();

        x610_hardware::serial << "Encoder offset: " << m2006_enc_.angle_offset << "\n";

        is_calibration_ = false;
    }

    float getPosition() const { return position_; }

    void printSensorValue() {
        for (size_t i = 0 ; i < enc_logs_.size() ; i++) {
            // x610_hardware::serial << uvw_logs_[i].u << "," << uvw_logs_[i].v << "," << uvw_logs_[i].w << ",";
            x610_hardware::serial << dq_logs_[i].d << "," << dq_logs_[i].q << "\n";
            // x610_hardware::serial << enc_logs_[i].angle << "\n";
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

    bool is_calibration_ = false;

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

    BackCalculationPI_D d_pid_;
    BackCalculationPI_D q_pid_;
};

}

extern board::x610::receiver::BLDCMotorController controller;
