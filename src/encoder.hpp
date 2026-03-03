#include <iostream>
#include <cmath>
#include "board.hpp"
#include "common.hpp"

namespace x610_common {
class RotaryEncoder {
    static constexpr float TWO_PI = 2.0 * PI;
    const uint16_t pole_pairs;      // 極対数

public:
    RotaryEncoder(int pole_pairs)
        : pole_pairs(pole_pairs) {}

    void resetRotorPosition() {
        zero_offset_rot = (total_mech_angle_rad / TWO_PI);
    }

    void offsetElectricalAngle() {
        electrical_angle_offset_ = electrical_angle_;
    }

    void resetElectricalAngleOffset() {
        electrical_angle_offset_ = 0.f;
    }

    /**
     * @param sin 電気角に対するsin
     * @param cos 電気角に対するcos
     * @param dt 前回呼び出しからの経過時間 (s)
     * @return 算出された状態（速度と累積位置）
     */
    void updateFromSinCos(float sin, float cos, float dt) {
        electrical_angle_ = x610_hardware::cordic.atan2(sin, cos) - electrical_angle_offset_;
        sin_ = dsp_math::sin(electrical_angle_);
        cos_ = dsp_math::cos(electrical_angle_);
        updateRotorAngle(electrical_angle_, dt);
    }

    void udpateFromIncrementalEncoder() {

    }

    float getRotorPosition() const {
        return rotor_position_;
    }

    float getRotorVelocity() const {
        return rotor_velocity_;
    }

    float getElectricalAngle() const {
        return electrical_angle_;
    }

    float getElectricalAngleOffset() const {
        return electrical_angle_offset_;
    }

    float getElectricalAngleCosine() const {
        return cos_;
    }

    float getElectricalAngleSine() const {
        return sin_;
    }

private:
    void updateRotorAngle(float electrical_angle, float dt) {
        float current_mech_angle_rad = electrical_angle / pole_pairs;

        if (first_run) {
            last_mech_angle_rad = current_mech_angle_rad;
            first_run = false;
            rotor_position_ = 0.0;
            rotor_velocity_ = 0.0;
        }

        float diff = current_mech_angle_rad - last_mech_angle_rad;

        float mech_cycle = TWO_PI / pole_pairs;
        if (diff > mech_cycle / 2.0) {
            diff -= mech_cycle;
        } else if (diff < -mech_cycle / 2.0) {
            diff += mech_cycle;
        }

        total_mech_angle_rad += diff;
        last_mech_angle_rad = current_mech_angle_rad;

        rotor_position_ = (total_mech_angle_rad / TWO_PI) - zero_offset_rot;
        rotor_velocity_ = (dt > 0) ? (diff / TWO_PI) / dt : 0.0;
    }

private:
    float last_mech_angle_rad = 0.0;
    float total_mech_angle_rad = 0.0;
    float zero_offset_rot = 0.0;
    bool first_run = true;

    float rotor_velocity_; // [rotation/s]
    float rotor_position_; // [rotation]

    float cos_;
    float sin_;

    float electrical_angle_;
    float electrical_angle_offset_;

};

}
