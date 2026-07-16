#pragma once
#include <vector>

namespace dsp {

// Normalized Least Mean Squares adaptive filter.
// Classic use: acoustic echo cancellation.
//   x = far-end reference (what the loudspeaker plays)
//   d = microphone signal (near-end speech + echo of x)
//   output e = d - y_hat  -> echo removed, near-end speech remains
class Nlms {
public:
    Nlms(size_t taps, float mu = 0.5f, float eps = 1e-6f)
        : w_(taps, 0.0f), x_(taps, 0.0f), mu_(mu), eps_(eps) {}

    float process(float x_new, float d) {
        // Shift reference signal into delay line
        for (size_t i = x_.size() - 1; i > 0; --i) x_[i] = x_[i - 1];
        x_[0] = x_new;

        // Estimate echo: y_hat = w^T x
        float y_hat = 0.0f, energy = eps_;
        for (size_t i = 0; i < w_.size(); ++i) {
            y_hat  += w_[i] * x_[i];
            energy += x_[i] * x_[i];
        }

        const float e = d - y_hat;  // error = cleaned signal

        // NLMS update: w += mu * e * x / ||x||^2
        const float g = mu_ * e / energy;
        for (size_t i = 0; i < w_.size(); ++i) w_[i] += g * x_[i];

        return e;
    }

private:
    std::vector<float> w_, x_;
    float mu_, eps_;
};

}  // namespace dsp