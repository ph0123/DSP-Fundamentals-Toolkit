#pragma once
#include <cmath>
#include <vector>
#include "fft.hpp"

namespace dsp {

inline std::vector<float> hann_window(size_t n) {
    std::vector<float> w(n);
    for (size_t i = 0; i < n; ++i)
        w[i] = 0.5f * (1.0f - std::cos(2.0f * float(M_PI) * i / (n - 1)));
    return w;
}

struct Stft {
    size_t frame_size;   // e.g. 512 (32 ms @ 16 kHz)
    size_t hop_size;     // e.g. 128 (8 ms)
    std::vector<float> window;

    Stft(size_t frame, size_t hop)
        : frame_size(frame), hop_size(hop), window(hann_window(frame)) {}

    // Returns [num_frames][frame_size/2+1] complex spectra
    std::vector<std::vector<cpx>> forward(const std::vector<float>& x) const {
        std::vector<std::vector<cpx>> spec;
        if (x.size() < frame_size) return spec;
        for (size_t start = 0; start + frame_size <= x.size(); start += hop_size) {
            std::vector<float> frame(frame_size);
            for (size_t i = 0; i < frame_size; ++i)
                frame[i] = x[start + i] * window[i];
            spec.push_back(rfft(frame));
        }
        return spec;
    }

    // Inverse STFT with overlap-add (assumes same window, COLA-compliant hop)
    std::vector<float> inverse(const std::vector<std::vector<cpx>>& spec) const {
        if (spec.empty()) return {};
        const size_t n_bins = frame_size / 2 + 1;
        const size_t out_len = (spec.size() - 1) * hop_size + frame_size;
        std::vector<float> out(out_len, 0.0f), norm(out_len, 1e-8f);

        for (size_t t = 0; t < spec.size(); ++t) {
            // Rebuild full spectrum from half (conjugate symmetry)
            std::vector<cpx> full(frame_size);
            for (size_t k = 0; k < n_bins; ++k) full[k] = spec[t][k];
            for (size_t k = n_bins; k < frame_size; ++k)
                full[k] = std::conj(full[frame_size - k]);
            fft(full, /*inverse=*/true);

            const size_t start = t * hop_size;
            for (size_t i = 0; i < frame_size; ++i) {
                out[start + i]  += full[i].real() * window[i];
                norm[start + i] += window[i] * window[i];
            }
        }
        for (size_t i = 0; i < out_len; ++i) out[i] /= norm[i];
        return out;
    }
};

}  // namespace dsp