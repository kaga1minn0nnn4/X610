#include "angle_controller.hpp"

namespace x610_controller {

void AngleController::config() {
    current_controller_.config();
    current_controller_.calibration();
}

void AngleController::update() {
    if (mode_ == AngleControlMode::none) {
        return;
    }

    float target_current = 0.f;
    switch (mode_) {
    case AngleControlMode::position:
        target_velocity_ = position_pid_.getManipulatedValue(current_controller_.getPosition(), target_position_);
    case AngleControlMode::velocity:
        target_current_ = velocity_pid_.getManipulatedValue(current_controller_.getVelocity(), target_velocity_);
    case AngleControlMode::current:
        target_current = target_current_;
        break;
    default:
        break;
    }

    current_controller_.setTargetCurrent(target_current);
}

void AngleController::setVelocityPIDGain(float kp, float ki, float kd) {
    velocity_pid_.setPGain(kp);
    velocity_pid_.setIGain(ki);
    velocity_pid_.setDGain(kd);
}

void AngleController::setPositionPIDGain(float kp, float ki, float kd) {
    position_pid_.setPGain(kp);
    position_pid_.setIGain(ki);
    position_pid_.setDGain(kd);
}

void AngleController::setVelocityPIDFeedForward(float feed_forward) {
    velocity_pid_.setFeedForward(feed_forward);
}

void AngleController::setPositionPIDFeedForward(float feed_forward) {
    position_pid_.setFeedForward(feed_forward);
}

void AngleController::setTargetCurrent(float current) {
    target_current_ = current;
    mode_ = AngleControlMode::current;
}

void AngleController::setTargetVelocity(float velocity) {
    target_velocity_ = velocity;
    mode_ = AngleControlMode::velocity;
}

void AngleController::setTargetPosition(float position) {
    target_position_ = position;
    mode_ = AngleControlMode::position;
}

void AngleController::calculateSpeedResponse(float current, float time) {
    disable();
    delay_ms(100);

    setTargetCurrent(0.f);

    enable();
    setTargetCurrent(current);

    delay_ms(static_cast<uint32_t>(time * 1000));

    setTargetCurrent(0.f);
    disable();

    x610_hardware::serial << "Velocity: " << current_controller_.getVelocity() << "\n";
}

void AngleController::enable() {
    position_pid_.resetStatus();
    velocity_pid_.resetStatus();
    current_controller_.enable();
}

void AngleController::disable() {
    current_controller_.disable();
}

void AngleController::resetPIDStatus() {
    velocity_pid_.resetStatus();
    position_pid_.resetStatus();
}

void AngleController::calibration() {
    current_controller_.calibration();
}

}

x610_controller::AngleController angle_controller;
