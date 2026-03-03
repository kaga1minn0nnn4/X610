#pragma once

#include "RUR_CANBoard_Base/board/X610.hpp"
#include "board.hpp"
#include "common.hpp"
#include "RUR_STM_CommonLib/PIDLib/PID.hpp"
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

constexpr BackCalculationPI_D::Parameter velocity_param = {
    .kp = 0.05f,
    .ki = 5.f / 5.f,
    .kd = 0.0f,
    .control_frequency = x610_hardware::kTimerInterruptFreq,
    .manipulated_value_limit = 3.0f,
    .feed_forward = 0.0f
};

constexpr BackCalculationPI_D::Parameter position_param = {
    .kp = 10.f,
    .ki = 0.3f,
    .kd = 0.3f,
    .control_frequency = x610_hardware::kTimerInterruptFreq,
    .manipulated_value_limit = 100.0f,
    .feed_forward = 0.0f
};

enum class ControlMode {
    dq_voltage,
    current,
    velocity,
    position,
    calculate_speed_response
};

class BLDCMotorCurrentController {
    static constexpr uint16_t kDutyMax = 90;
    static constexpr float kLPFAlpha = 0.3;

public:
    BLDCMotorCurrentController() : d_pid_(d_param), q_pid_(q_param), velocity_pid_{velocity_param}, position_pid_{position_param} {}

    void config();

    void calculateSpeedResponse(float current, float time);

    void setVoltage(float voltage) {
        mode = ControlMode::dq_voltage;
        target_voltage_ = voltage;
    }

    void setTargetCurrent(float current) {
        mode = ControlMode::current;
        target_current_ = current;
    }

    void setTargetVelocity(float velocity) {
        mode = ControlMode::velocity;
        target_velocity_ = velocity;
    }

    void setTargetPosition(float position) {
        mode = ControlMode::position;
        target_position_ = position;
    }

    void update();

    void positionInitialize(float velocity);

    void setMotorBehavior(board::x610::MotorBehavior behavior);

    float getCurrentD() const { return current_dq_.d; }
    float getCurrentQ() const { return current_dq_.q; }
    float getVelocity() const { return velocity_; }
    float getPosition() const { return position_; }

    void calibration();

    uint32_t getControlCount() {return ctl_count_;}

private:
    void controlTask();
    void updateSensorValue();

    void trgoHandler() {
        uint32_t start = delay_getCount();

        updateSensorValue();
        if(!is_configuration_) {
            controlTask();
        }

        ctl_count_ = delay_getCount() - start;
    }

    void enableDriver();
    void disableDriver();

private:
    bool is_configuration_ = true;
    bool is_calibration_ = false;

    float current_ = 0.f;
    float velocity_ = 0.f;
    float position_ = 0.f;

    float target_voltage_;
    float target_current_;
    float target_velocity_;
    float target_position_;

    std::array<float, 3> raw_current_uvw_offset_;

    x610_common::UVW current_uvw_;
    x610_common::AB current_ab_;
    x610_common::DQ current_dq_;

    BackCalculationPI_D d_pid_;
    BackCalculationPI_D q_pid_;

    uint32_t ctl_count_;

    ControlMode mode = ControlMode::dq_voltage;

    x610_common::RotaryEncoder encoder{7};

    BackCalculationPI_D velocity_pid_;
    BackCalculationPI_D position_pid_;

    uint32_t stop_time_ = 0;
};

}

extern x610_controller::BLDCMotorCurrentController current_controller;
