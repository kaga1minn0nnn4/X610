#pragma once

#include <array>
#include <math.h>

namespace x610_common {

// constexpr std::array<float, 2> kVectorU = {1.0f, 0.0f};
// constexpr std::array<float, 2> kVectorV = {-0.5f, 0.86602540378f};
// constexpr std::array<float, 2> kVectorW = {-0.5f, -0.86602540378f};

constexpr std::array<float, 2> kVectorU = {0.5f, 0.86602540378f};
constexpr std::array<float, 2> kVectorV = {-1.0f, 0.0f};
constexpr std::array<float, 2> kVectorW = {0.5f, -0.86602540378f};

// constexpr std::array<float, 2> kVectorU = {0.5f, -0.86602540378f};
// constexpr std::array<float, 2> kVectorV = {0.5f, 0.86602540378f};
// constexpr std::array<float, 2> kVectorW = {-1.0f, 0.0f};

// constexpr std::array<float, 2> kVectorU = {-0.5f, 0.86602540378f};
// constexpr std::array<float, 2> kVectorV = {-0.5f, -0.86602540378f};
// constexpr std::array<float, 2> kVectorW = {1.0f, 0.0f};


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
