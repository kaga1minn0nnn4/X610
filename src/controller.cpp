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

    for (auto& pwm : x610_hardware::pwms) {
        pwm.setEnable(true);
        pwm.setDuty(0.f);
    }

    delay_ms(1000);

    for (int i = 0; i < 10; i++) {
        raw_current_uvw_offset_[0] += x610_hardware::adcs[0].getInjectionData(peripheral::adcv2::InjectionChannel::_1) / 10.0f;
        raw_current_uvw_offset_[1] += x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_1) / 10.0f;
        raw_current_uvw_offset_[2] += x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_2) / 10.0f;
        delay_ms(100);
    }

    x610_hardware::esc_control_timer.setStart(true);
}

float duty_out = 0.1f;
float freq_out = 10.0f;
void outputDuty(float alpha, float beta){
	float norm = sqrt(alpha * alpha + beta * beta);
	if(norm > 1.0f) {
		alpha /= norm;
		beta /= norm;
		norm = 1.0f;
	}
	float u = alpha * kVectorU[0] + beta * kVectorU[1];
	float v = alpha * kVectorV[0] + beta * kVectorV[1];
	float w = alpha * kVectorW[0] + beta * kVectorW[1];

    x610_hardware::pwms[0].setDuty(u * 100);
    x610_hardware::pwms[1].setDuty(v * 100);
    x610_hardware::pwms[2].setDuty(w * 100);
}



void BLDCMotorController::controlTask() {
    DQ dq;
    AB ab;
    UVW uvw;
    dq.d = 0.f;
    dq.q = 0.3f;
    ab.update_from_dq(dq, m2006_enc_);
    uvw.update_from_ab(ab);

    x610_hardware::pwms[0].setDuty(uvw.u * 100);
    x610_hardware::pwms[1].setDuty(uvw.w * 100);
    x610_hardware::pwms[2].setDuty(uvw.v * 100);
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
    x610_hardware::drvoff.write(false);
}

void BLDCMotorController::disableDriver() {
    x610_hardware::drvoff.write(true);
}

void BLDCMotorController::updateSensorValue() {
	current_uvw_.u = -(x610_hardware::adcs[0].getInjectionData(peripheral::adcv2::InjectionChannel::_1) - raw_current_uvw_offset_[0]) * x610_hardware::kCurrentMagnification;
	current_uvw_.v = -(x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_1) - raw_current_uvw_offset_[1]) * x610_hardware::kCurrentMagnification;
	current_uvw_.w = -(x610_hardware::adcs[1].getInjectionData(peripheral::adcv2::InjectionChannel::_2) - raw_current_uvw_offset_[2]) * x610_hardware::kCurrentMagnification;

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

    current_ab_.update_from_uvw(current_uvw_);
    current_dq_.update_from_ab(current_ab_, m2006_enc_);

    for (auto& adc : x610_hardware::adcs) {
        adc.update();
    }
}

}

board::x610::receiver::BLDCMotorController controller;
