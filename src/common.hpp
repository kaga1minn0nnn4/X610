#pragma once

#include <array>
#include <math.h>

#include "RUR_STM_CommonLib/math/units.hpp"

namespace x610_common {

constexpr float u_angle = 60.f / 180.f * math::PI;
constexpr float v_angle = u_angle + 2.0f/3.0f*math::PI;
constexpr float w_angle = v_angle + 2.0f/3.0f*math::PI;

extern std::array<float, 2> kVectorU;
extern std::array<float, 2> kVectorV;
extern std::array<float, 2> kVectorW;

// constexpr std::array<float, 2> kVectorU = {0.5f, 0.86602540378f};
// constexpr std::array<float, 2> kVectorV = {-1.0f, 0.0f};
// constexpr std::array<float, 2> kVectorW = {0.5f, -0.86602540378f};

struct M2006EncoderValue {
    float cos;
    float sin;
    float angle;

    void update_angle() {
        angle = atan2(sin, cos);
    }
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

}
