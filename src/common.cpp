#include "common.hpp"

namespace x610_common {

std::array<float, 2> kVectorU = {cos(u_angle), sin(u_angle)};
std::array<float, 2> kVectorV = {cos(v_angle), sin(v_angle)};
std::array<float, 2> kVectorW = {cos(w_angle), sin(w_angle)};

void UVW::update_from_ab(const AB& ab) {
    u = ab.a * kVectorU[0] + ab.b * kVectorU[1];
    v = ab.a * kVectorV[0] + ab.b * kVectorV[1];
    w = ab.a * kVectorW[0] + ab.b * kVectorW[1];
}

void AB::update_from_uvw(const UVW& uvw) {
    // Clarke変換
    a = (uvw.u * kVectorU[0] + uvw.v * kVectorV[0] + uvw.w * kVectorW[0]) / 1.5f;
    b = (uvw.u * kVectorU[1] + uvw.v * kVectorV[1] + uvw.w * kVectorW[1]) / 1.5f;
}

void AB::update_from_dq(const DQ& dq, const M2006EncoderValue& enc) {
    float theta = enc.angle;
    float c = cos(theta);
    float s = sin(theta);
    a = dq.d * c - dq.q * s;
    b = dq.d * s + dq.q * c;
    // a = dq.d * enc.cos - dq.q * enc.sin;
    // b = dq.d * enc.sin + dq.q * enc.cos;
}

void DQ::update_from_ab(const AB& ab, const M2006EncoderValue& enc) {
    float theta = enc.angle;
    float c = cos(theta);
    float s = sin(theta);
    d = ab.a * c + ab.b * s;
    q = -ab.a * s + ab.b * c;
}

}
