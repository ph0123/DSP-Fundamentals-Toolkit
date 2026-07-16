#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include "dsp/stft.hpp"
#include "dsp/wav.hpp"

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: demo_spectrogram in.wav out.pgm\n";
        return 1;
    }
    auto wav = dsp::read_wav(argv[1]);
    dsp::Stft stft(512, 128);
    auto spec = stft.forward(wav.samples);

    const size_t T = spec.size(), F = spec[0].size();
    std::vector<float> db(T * F);
    float mx = -1e9f;
    for (size_t t = 0; t < T; ++t)
        for (size_t f = 0; f < F; ++f) {
            float m = std::abs(spec[t][f]);
            db[t * F + f] = 20.0f * std::log10(m + 1e-9f);
            mx = std::max(mx, db[t * F + f]);
        }

    // PGM: rows = frequency (top = high), cols = time, 80 dB dynamic range
    std::ofstream img(argv[2]);
    img << "P2\n" << T << " " << F << "\n255\n";
    for (size_t f = F; f-- > 0;) {
        for (size_t t = 0; t < T; ++t) {
            float v = std::clamp((db[t * F + f] - (mx - 80.0f)) / 80.0f, 0.0f, 1.0f);
            img << int(v * 255) << " ";
        }
        img << "\n";
    }
    std::cout << "Wrote spectrogram: " << T << " frames x " << F << " bins\n";
    return 0;
}