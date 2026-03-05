#include "current_controller.hpp"


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
    x610_hardware::serial << "U: " << raw_current_uvw_offset_[0] * x610_hardware::kADCMagnification << "[V]" << ",";
    x610_hardware::serial << "V: " << raw_current_uvw_offset_[1] * x610_hardware::kADCMagnification << "[V]" << ",";
    x610_hardware::serial << "W: " << raw_current_uvw_offset_[2] * x610_hardware::kADCMagnification << "[V]" << "\n";

    encoder.resetRotorPosition();

    is_configuration_ = false;
}

void BLDCMotorCurrentController::calibration() {
    x610_hardware::serial << "Calibration..." << "\n";

    encoder.resetElectricalAngleOffset();
    mode_ = Mode::calibration;
    enable();

    delay_ms(500);

    encoder.offsetElectricalAngle();

    disable();

    x610_hardware::serial << "Encoder offset: " << encoder.getElectricalAngleOffset() << "\n";

    mode_ = Mode::current;
}

void BLDCMotorCurrentController::controlTask() {
    float d_man_value = d_pid_.getManipulatedValue(current_dq_.d, 0.0f);
    float q_man_value = q_pid_.getManipulatedValue(current_dq_.q, target_current_);
    float norm = dsp_math::sqrt(d_man_value*d_man_value + q_man_value*q_man_value);
    if (norm > 1.0) {
        d_man_value /= norm;
        q_man_value /= norm;
    }

    x610_common::DQ dq;
    x610_common::AB ab;
    x610_common::UVW uvw;

    switch (mode_) {
    case Mode::voltage:
        dq.d = target_voltage_d_;
        dq.q = target_voltage_q_;
        ab.update_from_dq(dq, encoder.getElectricalAngleCosine(), encoder.getElectricalAngleSine());
        uvw.update_from_ab(ab);
        break;
    case Mode::current:
        dq.d = d_man_value;
        dq.q = q_man_value;
        ab.update_from_dq(dq, encoder.getElectricalAngleCosine(), encoder.getElectricalAngleSine());
        uvw.update_from_ab(ab);
        break;
    case Mode::calibration:
        ab.a = 0.1;
        ab.b = 0.0;
        uvw.update_from_ab(ab);
        break;
    default:
        break;
    }

    setDuty(uvw);
}

void BLDCMotorCurrentController::setDuty(const x610_common::UVW& uvw) {
    x610_common::UVW uvw_;

    uvw_.u = std::clamp<float>(uvw.u, -1.0, 1.0);
    uvw_.v = std::clamp<float>(uvw.v, -1.0, 1.0);
    uvw_.w = std::clamp<float>(uvw.w, -1.0, 1.0);

    x610_hardware::pwms[0].setDuty(- uvw_.u * kDutyMax);
    x610_hardware::pwms[1].setDuty(- uvw_.v * kDutyMax);
    x610_hardware::pwms[2].setDuty(- uvw_.w * kDutyMax);
}

void BLDCMotorCurrentController::enable() {
    d_pid_.resetStatus();
    q_pid_.resetStatus();
    x610_hardware::enable_driver();
}

void BLDCMotorCurrentController::disable() {
    x610_hardware::disable_driver();
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

void BLDCMotorCurrentController::trgoHandler() {
    uint32_t start = delay_getCount();

    updateSensorValue();
    if(!is_configuration_) {
        controlTask();
    }

    ctl_count_ = delay_getCount() - start;
}

}
