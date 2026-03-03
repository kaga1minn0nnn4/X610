#pragma once

#include "RUR_CANBoard_Base/board/X610.hpp"
#include "RUR_STM_CommonLib/PIDLib/PID.hpp"

#include "board.hpp"
#include "current_controller.hpp"

namespace x610_controller {

namespace {

enum class AngleControlMode {
    none,
    current,
    velocity,
    position,
    calculate_speed_response
};

}

class AngleController {


public:
    AngleController() : current_controller_{} {}

    void config();

    void update();

    void setVelocityPIDGain(float kp, float ki, float kd);
    void setPositionPIDGain(float kp, float ki, float kd);

    void setVelocityPIDFeedForward(float feed_forward);
    void setPositionPIDFeedForward(float feed_forward);

    void setTargetCurrent(float current);
    void setTargetVelocity(float velocity);
    void setTargetPosition(float position);

    void calculateSpeedResponse(float current, float time);

    void enable();
    void disable();

    float getCurrent() { current_controller_.getCurrentQ(); }
    float getVelocity() { current_controller_.getVelocity(); }
    float getPosition() { current_controller_.getPosition(); }

    void resetPIDStatus();

    void calibration();
private:
    AngleControlMode mode_ = AngleControlMode::none;

    float target_current_;
    float target_velocity_;
    float target_position_;

    BLDCMotorCurrentController current_controller_;

    BackCalculationPI_D::Parameter velocity_pid_params_{
        .kp = 0.0f,
        .ki = 0.0f,
        .kd = 0.0f,
        .control_frequency = x610_hardware::kTimerInterruptFreq,
        .manipulated_value_limit = 10.f,
        .feed_forward = 0.0f
    };
    BackCalculationPI_D::Parameter position_pid_params_{
        .kp = 0.0f,
        .ki = 0.0f,
        .kd = 0.0f,
        .control_frequency = x610_hardware::kTimerInterruptFreq,
        .manipulated_value_limit = 10.f,
        .feed_forward = 0.0f
    };

    BackCalculationPI_D velocity_pid_{velocity_pid_params_};
    BackCalculationPI_D position_pid_{position_pid_params_};
};

}

extern x610_controller::AngleController angle_controller;
