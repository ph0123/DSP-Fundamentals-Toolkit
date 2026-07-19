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
    // mu  : step size, 0 < mu < 2 for stability (0.5–1.0 is typical).
    // eps : regularization delta added to ||x||^2. It is NOT just a
    //       divide-by-zero guard — it is the *floor* on the denominator.
    //       If it is too small, then whenever the reference goes quiet
    //       (||x||^2 -> 0) the update gain mu*e/||x||^2 explodes and the
    //       weights diverge. For audio normalized to [-1, 1], 1e-3 is a
    //       safe floor; raise it if the reference is very low level.
    Nlms(size_t taps, float mu = 0.5f, float eps = 1e-3f)
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