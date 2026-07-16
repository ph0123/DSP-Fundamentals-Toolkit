#pragma once
#include <cmath>
#include <complex>
#include <stdexcept>
#include <vector>

namespace dsp {

using cpx = std::complex<float>;

// In-place iterative radix-2 FFT. n must be a power of 2.
inline void fft(std::vector<cpx>& a, bool inverse = false) {
    const size_t n = a.size();
    if (n == 0 || (n & (n - 1)) != 0)
        throw std::runtime_error("FFT size must be a power of 2");

    // Bit-reversal permutation
    for (size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }

    // Butterfly stages
    for (size_t len = 2; len <= n; len <<= 1) {
        const float ang = 2.0f * float(M_PI) / len * (inverse ? 1.0f : -1.0f);
        const cpx wlen(std::cos(ang), std::sin(ang));
        for (size_t i = 0; i < n; i += len) {
            cpx w(1.0f, 0.0f);
            for (size_t k = 0; k < len / 2; ++k) {
                cpx u = a[i + k];
                cpx v = a[i + k + len / 2] * w;
                a[i + k]           = u + v;
                a[i + k + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
    if (inverse)
        for (auto& x : a) x /= static_cast<float>(n);
}

// Real-input FFT convenience: returns first n/2+1 bins (the rest are conjugate-symmetric)
inline std::vector<cpx> rfft(const std::vector<float>& x) {
    std::vector<cpx> a(x.begin(), x.end());
    fft(a);
    a.resize(x.size() / 2 + 1);
    return a;
}

}  // namespace dsp