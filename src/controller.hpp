#pragma once

#include "RUR_CANBoard_Base/board/X610.hpp"
#include "board.hpp"

namespace board::x610::receiver {

// constexpr std::array<float, 2> kVectorU = {1.0f, 0.0f};
// constexpr std::array<float, 2> kVectorV = {-0.5f, 0.86602540378f};
// constexpr std::array<float, 2> kVectorW = {-0.5f, -0.86602540378f};

constexpr std::array<float, 2> kVectorU = {0.5f, 0.86602540378f};
constexpr std::array<float, 2> kVectorV = {-1.0f, 0.0f};
constexpr std::array<float, 2> kVectorW = {0.5f, -0.86602540378f};


struct M2006EncoderValue {
    float cos;
    float sin;
};

struct UVW;
struct AB;
struct DQ;

struct UVW {
    float u;
    float v;
    float w;

    void update_from_ab(const AB& ab);
};

struct AB {
    float a;
    float b;

    void update_from_uvw(const UVW& uvw);

    void update_from_dq(const DQ& dq, const M2006EncoderValue& enc);
};

struct DQ {
    float d;
    float q;

    void update_from_ab(const AB& ab, const M2006EncoderValue& enc);
};

class BLDCMotorController {
    static constexpr uint16_t kDutyMax = 90;

public:
    void config();

    void calculateSpeedResponse(float current, float time);

    void setVoltage(float voltage);
    void setTargetCurrent(float current);
    void setTargetVelocity(float velocity);
    void setTargetPosition(float position);

    void artificialCommutationControl();

    void positionInitialize(float velocity);

    void setMotorBehavior(MotorBehavior behavior);

    float getCurrent() const { return current_; }
    float getVelocity() const { return velocity_; }
    float getPosition() const { return position_; }

    void printSensorValue() {
        for (size_t i = 0 ; i < enc_logs_.size() ; i++) {
            x610_hardware::serial << uvw_logs_[i].u << "," << uvw_logs_[i].v << "," << uvw_logs_[i].w << ",";
            x610_hardware::serial << dq_logs_[i].d << "," << dq_logs_[i].q << ",";
            x610_hardware::serial << enc_logs_[i].cos << "," << enc_logs_[i].sin << "\n";
            delay_ms(1);
        }
    }

    void enableprint() {
        x610_hardware::serial << "Begin...\n";
        enable_print_ = true;
    }


    void updateSensorValue();
    void resetcount() {ctl_count_ = 0;}
private:
    void controlTask();

    void enableDriver();
    void disableDriver();

private:
    bool enable_print_ = false;

    float current_ = 0.f;
    float velocity_ = 0.f;
    float position_ = 0.f;

    std::array<float, 3> raw_current_uvw_offset_;

    std::array<UVW, 500> uvw_logs_;
    std::array<M2006EncoderValue, 500> enc_logs_;
    std::array<DQ, 500> dq_logs_;
    uint16_t logs_count_ = 0;


    UVW current_uvw_;
    AB current_ab_;
    DQ current_dq_;

    M2006EncoderValue m2006_enc_;

    float raw_cos_;
    float raw_sin_;

    float target_voltage_;

    uint32_t ctl_count_;
};

}

extern board::x610::receiver::BLDCMotorController controller;
