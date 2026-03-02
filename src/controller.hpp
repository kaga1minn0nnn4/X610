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
    .control_frequency = x610_hardware::kPWMTimerFreq/2,
    .manipulated_value_limit = 1.0f,
    .feed_forward = 0.0f
};

constexpr BackCalculationPI_D::Parameter q_param = {
    .kp = 0.03,
    .ki = 20.0f,
    .kd = 0.0f,
    .control_frequency = x610_hardware::kPWMTimerFreq/2,
    .manipulated_value_limit = 1.0f,
    .feed_forward = 0.0f
};

class BLDCMotorController {
    static constexpr uint16_t kDutyMax = 90;

public:
    BLDCMotorController() : d_pid_(d_param), q_pid_(q_param) {}

    void config();

    void calculateSpeedResponse(float current, float time);

    void setVoltage(float voltage) {
        target_voltage_ = voltage;
    }
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

    std::array<float, 3> raw_current_uvw_offset_;

    x610_common::UVW current_uvw_;
    x610_common::AB current_ab_;
    x610_common::DQ current_dq_;

    x610_common::M2006EncoderValue m2006_enc_;

    BackCalculationPI_D d_pid_;
    BackCalculationPI_D q_pid_;

    uint32_t ctl_count_;
};

}

extern board::x610::receiver::BLDCMotorController controller;
