#pragma once
#include <cmath>

namespace dsp {

// Direct Form II Transposed biquad — numerically robust, standard in audio
class Biquad {
public:
    enum class Type { LowPass, HighPass };

    Biquad(Type type, float fc, float fs, float q = 0.7071f) {
        const float w0    = 2.0f * float(M_PI) * fc / fs;
        const float alpha = std::sin(w0) / (2.0f * q);
        const float cw    = std::cos(w0);
        float b0, b1, b2, a0, a1, a2;

        if (type == Type::LowPass) {
            b0 = (1 - cw) / 2; b1 = 1 - cw; b2 = (1 - cw) / 2;
        } else {  // HighPass
            b0 = (1 + cw) / 2; b1 = -(1 + cw); b2 = (1 + cw) / 2;
        }
        a0 = 1 + alpha; a1 = -2 * cw; a2 = 1 - alpha;

        b0_ = b0 / a0; b1_ = b1 / a0; b2_ = b2 / a0;
        a1_ = a1 / a0; a2_ = a2 / a0;
    }

    // Process one sample — this is how real-time audio works: sample by sample
    float process(float x) {
        const float y = b0_ * x + z1_;
        z1_ = b1_ * x - a1_ * y + z2_;
        z2_ = b2_ * x - a2_ * y;
        return y;
    }

private:
    float b0_, b1_, b2_, a1_, a2_;
    float z1_ = 0.0f, z2_ = 0.0f;  // filter state
};

}  // namespace dsp