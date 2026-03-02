#include <iostream>
#include <cmath>
#include "arm_math.h"

class BLDCPositionTracker {
    static constexpr float TWO_PI = 2.0 * PI;
    const uint16_t pole_pairs;      // 極対数

public:
    BLDCPositionTracker(int pole_pairs)
        : pole_pairs(pole_pairs) {}

    void resetPosition() {
        zero_offset_rot = (total_mech_angle_rad / TWO_PI);
    }

    /**
     * @param electrical_angle 電気角 [-PI, PI] (rad)
     * @param dt 前回呼び出しからの経過時間 (s)
     * @return 算出された状態（速度と累積位置）
     */
    void update(float electrical_angle, float dt) {
        float current_mech_angle_rad = electrical_angle / pole_pairs;

        if (first_run) {
            last_mech_angle_rad = current_mech_angle_rad;
            first_run = false;
            position_ = 0.0;
            velocity_ = 0.0;
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

        position_ = (total_mech_angle_rad / TWO_PI) - zero_offset_rot;
        velocity_ = (dt > 0) ? (diff / TWO_PI) / dt : 0.0;
    }

    float getPosition() const {
        return position_;
    }

    float getVelocity() const {
        return velocity_;
    }

private:
    float last_mech_angle_rad = 0.0;
    float total_mech_angle_rad = 0.0;
    float zero_offset_rot = 0.0;
    bool first_run = true;

    float velocity_; // [rotation/s]
    float position_; // [rotation]
};
