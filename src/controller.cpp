#include "controller.hpp"
namespace x610_controller {

void BLDCMotorCurrentController::config() {
    x610_hardware::adcs[0].configIT(std::bind(&BLDCMotorCurrentController::trgoHandler, this));

    for (auto& adc : x610_hardware::adcs) {
        adc.calibration();
        adc.enable();
        adc.startConversion();
        delay_ms(50);
    }

    for (auto& opamp : x610_hardware::opamps) {
        opamp.enable();
    }

    x610_hardware::pwm_timer.setStart(true);
    x610_hardware::it_timer.setStart(true);

    delay_ms(1000);

    for (int i = 0; i < 10; i++) {
        raw_current_uvw_offset_[0] += x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_2) / 10.0f; // u
        raw_current_uvw_offset_[1] += x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_1) / 10.0f; // v
        raw_current_uvw_offset_[2] += x610_hardware::adcs[0].getInjectionData(peripheral::adcv2::InjectionChannel::_1) / 10.0f; // w
        delay_ms(100);
    }
    x610_hardware::serial << "Offset: \n";
    x610_hardware::serial << raw_current_uvw_offset_[0] << ",";
    x610_hardware::serial << raw_current_uvw_offset_[1] << ",";
    x610_hardware::serial << raw_current_uvw_offset_[2] << "\n";

    x610_hardware::serial << "Vector: \n";
    x610_hardware::serial << "U: " << x610_common::kVectorU[0] << ", " << x610_common::kVectorU[1] << "\n";
    x610_hardware::serial << "V: " << x610_common::kVectorV[0] << ", " << x610_common::kVectorV[1] << "\n";
    x610_hardware::serial << "W: " << x610_common::kVectorW[0] << ", " << x610_common::kVectorW[1] << "\n";

    encoder.resetRotorPosition();

    is_configuration_ = false;
}

void BLDCMotorCurrentController::calibration() {
    x610_hardware::serial << "Calibration..." << "\n";

    encoder.resetElectricalAngleOffset();
    is_calibration_ = true;
    enableDriver();

    delay_ms(500);

    encoder.offsetElectricalAngle();

    disableDriver();

    x610_hardware::serial << "Encoder offset: " << encoder.getElectricalAngleOffset() << "\n";

    is_calibration_ = false;
}

void BLDCMotorCurrentController::calculateSpeedResponse(float current, float time) {
    mode = ControlMode::calculate_speed_response;
    disableDriver();
    delay_ms(100);

    setTargetCurrent(0.f);

    enableDriver();
    setTargetCurrent(current);

    delay_ms(static_cast<uint32_t>(time * 1000));

    setTargetCurrent(0.f);
    disableDriver();

    x610_hardware::serial << "Velocity: " << getVelocity() << "\n";
}

void BLDCMotorCurrentController::controlTask() {
    float d_man_value = d_pid_.getManipulatedValue(current_dq_.d, 0.0f);
    float q_man_value = q_pid_.getManipulatedValue(current_dq_.q, target_current_);

    x610_common::DQ dq;
    x610_common::AB ab;
    x610_common::UVW uvw;

    if (is_calibration_) {
        ab.a = 0.1;
        ab.b = 0.0;
        uvw.update_from_ab(ab);
    } else {
        float norm = dsp_math::sqrt(d_man_value*d_man_value + q_man_value*q_man_value);
        if (norm > 1.0) {
            d_man_value /= norm;
            q_man_value /= norm;
        }
        dq.d = d_man_value;
        dq.q = q_man_value;

        ab.update_from_dq(dq, encoder.getElectricalAngleCosine(), encoder.getElectricalAngleSine());
        uvw.update_from_ab(ab);
    }

    uvw.u = std::clamp<float>(uvw.u, -1.0, 1.0);
    uvw.v = std::clamp<float>(uvw.v, -1.0, 1.0);
    uvw.w = std::clamp<float>(uvw.w, -1.0, 1.0);

    x610_hardware::pwms[0].setDuty(- uvw.u * kDutyMax);
    x610_hardware::pwms[1].setDuty(- uvw.v * kDutyMax);
    x610_hardware::pwms[2].setDuty(- uvw.w * kDutyMax);
}

void BLDCMotorCurrentController::update() {
    switch (mode) {
    case ControlMode::calculate_speed_response:
        break;
    case ControlMode::velocity:
        target_current_ = velocity_pid_.getManipulatedValue(velocity_, target_velocity_);
        break;
    case ControlMode::position:
        target_current_ = velocity_pid_.getManipulatedValue(velocity_, position_pid_.getManipulatedValue(position_, target_position_));
        break;
    default:
        break;
    }
}

void BLDCMotorCurrentController::setMotorBehavior(board::x610::MotorBehavior behavior) {
    switch (behavior) {
    case board::x610::MotorBehavior::enable:
        d_pid_.resetStatus();
        q_pid_.resetStatus();
        velocity_pid_.resetStatus();
        position_pid_.resetStatus();
        enableDriver();
        break;
    case board::x610::MotorBehavior::disable:
        disableDriver();
        break;
    default:
        break;
    }
}

void BLDCMotorCurrentController::enableDriver() {
    // なんかドライバONにした後にPWM出力始めないとnFAULT吐いて落ちる

    for (auto& pwm : x610_hardware::pwms) {
        pwm.setEnable(false);
    }
    delay_ms(10);

    x610_hardware::drvoff.write(false);

    delay_ms(100);

    for (auto& pwm : x610_hardware::pwms) {
        pwm.setEnable(true);
    }
}

void BLDCMotorCurrentController::disableDriver() {
    x610_hardware::drvoff.write(true);
}

void BLDCMotorCurrentController::updateSensorValue() {
	current_uvw_.u = -(x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_2) - raw_current_uvw_offset_[0]) * x610_hardware::kCurrentMagnification;
	current_uvw_.v = -(x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_1) - raw_current_uvw_offset_[1]) * x610_hardware::kCurrentMagnification;
	current_uvw_.w = -(x610_hardware::adcs[0].getInjectionData(peripheral::adcv2::InjectionChannel::_1) - raw_current_uvw_offset_[2]) * x610_hardware::kCurrentMagnification;

    float cos_raw = x610_hardware::sensor_value_raw[0] * x610_hardware::kADCMagnification - 1.0f;
    float sin_raw = x610_hardware::sensor_value_raw[1] * x610_hardware::kADCMagnification - 1.0f;
    float norm = dsp_math::sqrt(sin_raw*sin_raw + cos_raw*cos_raw);
    float cos_norm;
    float sin_norm;
	if (norm > 0.0f) {
		sin_norm = sin_raw / norm;
		cos_norm = cos_raw / norm;
	} else {
		sin_norm = 0.0f;
        cos_norm = 1.0f;
	}
    encoder.updateFromSinCos(sin_norm, cos_norm, x610_hardware::kCurrentControlT);

    current_ab_.update_from_uvw(current_uvw_);

    x610_common::DQ current_dq_raw;
    current_dq_raw.update_from_ab(current_ab_, encoder.getElectricalAngleCosine(), encoder.getElectricalAngleSine());
    // LPF
    current_dq_.d = current_dq_raw.d * kLPFAlpha + current_dq_.d * (1 - kLPFAlpha);
    current_dq_.q = current_dq_raw.q * kLPFAlpha + current_dq_.q * (1 - kLPFAlpha);

    position_ = encoder.getRotorPosition();
    velocity_ = encoder.getRotorVelocity();

    for (auto& adc : x610_hardware::adcs) {
        adc.update();
    }
}

}

x610_controller::BLDCMotorCurrentController current_controller;
