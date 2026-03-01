#include "controller.hpp"

namespace board::x610::receiver {

void BLDCMotorController::config() {
    x610_hardware::adcs[0].configIT(std::bind(&BLDCMotorController::trgoHandler, this));

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

}

void BLDCMotorController::setVoltage(float voltage) {
    target_voltage_ = voltage;
}

void BLDCMotorController::controlTask() {
    float d_man_value = d_pid_.getManipulatedValue(current_dq_.d, 0.0f);
    float q_man_value = q_pid_.getManipulatedValue(current_dq_.q, target_voltage_);

    x610_common::DQ dq;
    x610_common::AB ab;
    x610_common::UVW uvw;

    if (is_calibration_) {
        ab.a = 0.1;
        ab.b = 0.0;
        uvw.update_from_ab(ab);
    } else {
        float norm = sqrt(d_man_value*d_man_value + q_man_value*q_man_value);
        if (norm > 1.0) {
            d_man_value /= norm;
            q_man_value /= norm;
        }
        dq.d = d_man_value;
        dq.q = q_man_value;

        ab.update_from_dq(dq, m2006_enc_);
        uvw.update_from_ab(ab);
    }

    uvw.u = std::clamp<float>(uvw.u, -1.0, 1.0);
    uvw.v = std::clamp<float>(uvw.v, -1.0, 1.0);
    uvw.w = std::clamp<float>(uvw.w, -1.0, 1.0);

    x610_hardware::pwms[0].setDuty(- uvw.u * kDutyMax);
    x610_hardware::pwms[1].setDuty(- uvw.v * kDutyMax);
    x610_hardware::pwms[2].setDuty(- uvw.w * kDutyMax);

    static uint32_t count = 0;
    if (count++ > 2000) {
        count = 0;
        x610_hardware::serial << current_dq_.d << ", " << current_dq_.q << "\n";
        // x610_hardware::serial << m2006_enc_.angle << "\n";
    }
}

void BLDCMotorController::setMotorBehavior(MotorBehavior behavior) {
    switch (behavior) {
    case MotorBehavior::enable:
        d_pid_.resetStatus();
        q_pid_.resetStatus();
        enableDriver();
        break;
    case MotorBehavior::disable:
        disableDriver();
        break;
    default:
        break;
    }
}

void BLDCMotorController::enableDriver() {
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

void BLDCMotorController::disableDriver() {
    x610_hardware::drvoff.write(true);
}

void BLDCMotorController::updateSensorValue() {
	current_uvw_.u = -(x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_2) - raw_current_uvw_offset_[0]) * x610_hardware::kCurrentMagnification;
	current_uvw_.v = -(x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_1) - raw_current_uvw_offset_[1]) * x610_hardware::kCurrentMagnification;
	current_uvw_.w = -(x610_hardware::adcs[0].getInjectionData(peripheral::adcv2::InjectionChannel::_1) - raw_current_uvw_offset_[2]) * x610_hardware::kCurrentMagnification;

    float cos_raw = x610_hardware::sensor_value_raw[0] * x610_hardware::kADCMagnification - 1.0f;
    float sin_raw = x610_hardware::sensor_value_raw[1] * x610_hardware::kADCMagnification - 1.0f;
    float norm = sqrt(sin_raw*sin_raw + cos_raw*cos_raw);
	if (norm > 0.0f) {
		m2006_enc_.sin = sin_raw / norm;
		m2006_enc_.cos = cos_raw / norm;
	} else {
		m2006_enc_.sin = 0.0f;
		m2006_enc_.cos = 1.0f;
	}
    m2006_enc_.update_angle();
    position_ = m2006_enc_.angle;

    current_ab_.update_from_uvw(current_uvw_);
    current_dq_.update_from_ab(current_ab_, m2006_enc_);

    for (auto& adc : x610_hardware::adcs) {
        adc.update();
    }

    // if (enable_print_) {
    //     enc_logs_[logs_count_] = m2006_enc_;
    //     dq_logs_[logs_count_] = current_dq_;
    //     uvw_logs_[logs_count_++] = current_uvw_;

    //     if (logs_count_ == enc_logs_.size()) {
    //         enable_print_ = false;
    //         x610_hardware::serial << "Finish\n";
    //         logs_count_ = 0;
    //     }
    // }
}

}

board::x610::receiver::BLDCMotorController controller;
