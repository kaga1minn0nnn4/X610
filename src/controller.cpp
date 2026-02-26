#include "controller.hpp"

namespace board::x610::receiver {


void UVW::update_from_ab(const AB& ab) {
    u = ab.a * kVectorU[0] + ab.b * kVectorU[1];
    v = ab.a * kVectorV[0] + ab.b * kVectorV[1];
    w = ab.a * kVectorW[0] + ab.b * kVectorW[1];
}

void AB::update_from_uvw(const UVW& uvw) {
    // Clarke変換
    a = uvw.u * kVectorU[0] + uvw.v * kVectorV[0] + uvw.w * kVectorW[0];
    b = uvw.u * kVectorU[1] + uvw.v * kVectorV[1] + uvw.w * kVectorW[1];
}

void AB::update_from_dq(const DQ& dq, const M2006EncoderValue& enc) {
    a = dq.d * enc.cos - dq.q * enc.sin;
    b = dq.d * enc.sin + dq.q * enc.cos;
}

void DQ::update_from_ab(const AB& ab, const M2006EncoderValue& enc) {
    d = ab.a * enc.cos + ab.b * enc.sin;
    q = -ab.a * enc.sin + ab.b * enc.cos;
}

void BLDCMotorController::config() {

    for (auto& adc : x610_hardware::adcs) {
        adc.calibration();
        adc.enable();
        adc.startConversion();
        delay_ms(50);
    }

    for (auto& opamp : x610_hardware::opamps) {
        opamp.enable();
    }

    x610_hardware::esc_control_timer.configIT(G4::TIM::InterruptMode::update, 1, std::bind(&BLDCMotorController::controlTask, this));

    x610_hardware::pwm_timer.setStart(true);
    x610_hardware::it_timer.setStart(true);

    delay_ms(1000);

    for (int i = 0; i < 10; i++) {
        raw_current_uvw_offset_[0] += x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_2) / 10.0f; // u
        raw_current_uvw_offset_[1] += x610_hardware::adcs[0].getInjectionData(peripheral::adcv2::InjectionChannel::_1) / 10.0f; // v
        raw_current_uvw_offset_[2] += x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_1) / 10.0f; // w
        delay_ms(100);
    }
    x610_hardware::serial << "Offset: \n";
    x610_hardware::serial << raw_current_uvw_offset_[0] << ",";
    x610_hardware::serial << raw_current_uvw_offset_[1] << ",";
    x610_hardware::serial << raw_current_uvw_offset_[2] << "\n";

    // x610_hardware::esc_control_timer.setStart(true);
}

void BLDCMotorController::setVoltage(float voltage) {
    target_voltage_ = voltage;
}

void BLDCMotorController::controlTask() {

}

void BLDCMotorController::setMotorBehavior(MotorBehavior behavior) {
    switch (behavior) {
    case MotorBehavior::enable:
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
	current_uvw_.v = -(x610_hardware::adcs[0].getInjectionData(peripheral::adcv2::InjectionChannel::_1) - raw_current_uvw_offset_[1]) * x610_hardware::kCurrentMagnification;
	current_uvw_.w = -(x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_1) - raw_current_uvw_offset_[2]) * x610_hardware::kCurrentMagnification;

    float sin_raw = x610_hardware::sensor_value_raw[0] * x610_hardware::kADCMagnification - 1.0f;
    float cos_raw = x610_hardware::sensor_value_raw[1] * x610_hardware::kADCMagnification - 1.0f;
    float norm = sqrt(sin_raw*sin_raw + cos_raw*cos_raw);
	if (norm > 0.0f) {
		m2006_enc_.sin = sin_raw / norm;
		m2006_enc_.cos = cos_raw / norm;
	} else {
		m2006_enc_.sin = 0.0f;
		m2006_enc_.cos = 1.0f;
	}

    // current_ab_.update_from_uvw(current_uvw_);
    // current_dq_.update_from_ab(current_ab_, m2006_enc_);

    for (auto& adc : x610_hardware::adcs) {
        adc.update();
    }
    // ctl_count_++;
    // DQ dq;
    // AB ab;
    // UVW uvw;
    // dq.d = 0.f;
    // dq.q = target_voltage_;
    // ab.update_from_dq(dq, m2006_enc_);
    // uvw.update_from_ab(ab);

    // uvw.u = std::clamp<float>(uvw.u, -1.0, 1.0);
    // uvw.v = std::clamp<float>(uvw.v, -1.0, 1.0);
    // uvw.w = std::clamp<float>(uvw.w, -1.0, 1.0);

    // x610_hardware::pwms[0].setDuty(uvw.u * kDutyMax);
    // x610_hardware::pwms[1].setDuty(uvw.v * kDutyMax);
    // x610_hardware::pwms[2].setDuty(uvw.w * kDutyMax);
    static uint16_t count = 0;
    if (enable_print_) {
        if (count++ > 10) {
            count = 0;
            enc_logs_[logs_count_] = m2006_enc_;
            uvw_logs_[logs_count_++] = current_uvw_;

            if (logs_count_ == 1000) {
                enable_print_ = false;
                x610_hardware::serial << "Finish\n";
                logs_count_ = 0;
            }
        }
    }
}

}

board::x610::receiver::BLDCMotorController controller;
