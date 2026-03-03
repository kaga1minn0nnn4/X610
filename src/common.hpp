#pragma once

#include <array>
#include "board.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "arm_math.h"

#ifdef __cplusplus
}
#endif

namespace x610_common {

constexpr std::array<float, 2> kVectorU = {0.5f, 0.86602540378f};
constexpr std::array<float, 2> kVectorV = {-1.0f, 0.0f};
constexpr std::array<float, 2> kVectorW = {0.5f, -0.86602540378f};

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

    void update_from_dq(const DQ& dq, const float& cos, const float& sin);
};

struct DQ {
    float d;
    float q;

    void update_from_ab(const AB& ab, const float& cos, const float& sin);
};

}

namespace dsp_math {

inline float sqrt(float x) {
    float result;
    arm_sqrt_f32(x, &result);
    return result;
}

inline float sin(float x) {
    return arm_sin_f32(x);
}

inline float cos(float x) {
    return arm_cos_f32(x);
}

}
