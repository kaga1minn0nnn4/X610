#pragma once

#include "RUR_STM_CommonLib/PIDLib/PID.hpp"
#include "board.hpp"
#include "common.hpp"
#include "encoder.hpp"

namespace x610_controller {

constexpr BackCalculationPI_D::Parameter d_param = {
    .kp = 0.03,
    .ki = 20.0f,
    .kd = 0.0f,
    .control_frequency = x610_hardware::kCurrentControlFreq,
    .manipulated_value_limit = 1.0f,
    .feed_forward = 0.0f
};

constexpr BackCalculationPI_D::Parameter q_param = {
    .kp = 0.03,
    .ki = 20.0f,
    .kd = 0.0f,
    .control_frequency = x610_hardware::kCurrentControlFreq,
    .manipulated_value_limit = 1.0f,
    .feed_forward = 0.0f
};

class BLDCMotorCurrentController {

    enum class Mode {
        voltage,
        current,
        calibration
    };

    static constexpr uint16_t kDutyMax = 90;
    static constexpr float kLPFAlpha = 0.3;

public:
    BLDCMotorCurrentController() : d_pid_(d_param), q_pid_(q_param) {}

    void config();

    void enable();
    void disable();

    void calibration();

    void setTargetCurrent(float current) {
        target_current_ = current;
        mode_ = Mode::current;
    }

    void setVoltage(float d, float q) {
        target_voltage_d_ = d;
        target_voltage_q_ = q;
        mode_ = Mode::voltage;
    }

    float getCurrentD() const { return current_dq_.d; }
    float getCurrentQ() const { return current_dq_.q; }
    float getVelocity() const { return velocity_; }
    float getPosition() const { return position_; }
    uint32_t getControlCount() {return ctl_count_;}

private:
    void controlTask();
    void updateSensorValue();

    void trgoHandler();

    void setDuty(const x610_common::UVW& uvw);

private:
    Mode mode_;

    bool is_configuration_ = true;

    float current_ = 0.f;
    float velocity_ = 0.f;
    float position_ = 0.f;

    float target_voltage_d_;
    float target_voltage_q_;
    float target_current_;

    std::array<float, 3> raw_current_uvw_offset_;

    x610_common::UVW current_uvw_;
    x610_common::AB current_ab_;
    x610_common::DQ current_dq_;

    BackCalculationPI_D d_pid_;
    BackCalculationPI_D q_pid_;

    uint32_t ctl_count_;

    x610_common::RotaryEncoder encoder{7};
};

}
