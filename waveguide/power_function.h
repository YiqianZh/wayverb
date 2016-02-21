#pragma once

#include "vec.h"

struct PowerFunction {
    virtual ~PowerFunction() noexcept = default;
    virtual float operator()(const Vec3f& a, const Vec3f& b) const = 0;
};

struct BasicPowerFunction final : public PowerFunction {
    float operator()(const Vec3f& a, const Vec3f& b) const override;
};

struct InversePowerFunction final : public PowerFunction {
    InversePowerFunction(float power);
    float operator()(const Vec3f& a, const Vec3f& b) const override;
    const float power;
};

struct InverseSquarePowerFunction final : public PowerFunction {
    InverseSquarePowerFunction(float power);
    float operator()(const Vec3f& a, const Vec3f& b) const override;
    const float power;
};

struct GaussianFunction final : public PowerFunction {
    GaussianFunction(float standard_deviation = 2);
    static float gaussian(const Vec3f& x, float sdev);
    float operator()(const Vec3f& a, const Vec3f& ex) const override;
    float standard_deviation;
};