#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "dsp/stft.hpp"
#include "dsp/wav.hpp"

// STFT spectrogram demo -> PGM image.
//
// Usage: demo_spectrogram in.wav out.pgm [frame_size] [hop_size] [range_db]
//   frame_size  FFT/window length, power of 2  (default 512 = 32 ms @ 16 kHz)
//   hop_size    step between frames            (default 128 = 75% overlap)
//   range_db    dynamic range shown            (default 80)
//
// Trade-off to explore on your own data: a larger frame_size gives finer
// frequency resolution but blurs time; a smaller one does the opposite.
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: demo_spectrogram in.wav out.pgm [frame_size] [hop_size] [range_db]\n";
        return 1;
    }
    // --- tunable parameters (with defaults) ---
    size_t frame_size = (argc > 3) ? std::atoi(argv[3]) : 512;
    size_t hop_size   = (argc > 4) ? std::atoi(argv[4]) : 128;
    float  range_db   = (argc > 5) ? std::atof(argv[5]) : 80.0f;

    auto wav = dsp::read_wav(argv[1]);
    dsp::Stft stft(frame_size, hop_size);
    auto spec = stft.forward(wav.samples);
    if (spec.empty()) {
        std::cerr << "Input shorter than one frame (" << frame_size << " samples)\n";
        return 1;
    }

    const size_t T = spec.size(), F = spec[0].size();
    std::vector<float> db(T * F);
    float mx = -1e9f;
    for (size_t t = 0; t < T; ++t)
        for (size_t f = 0; f < F; ++f) {
            float m = std::abs(spec[t][f]);
            db[t * F + f] = 20.0f * std::log10(m + 1e-9f);
            mx = std::max(mx, db[t * F + f]);
        }

    // PGM: rows = frequency (top = high), cols = time
    std::ofstream img(argv[2]);
    img << "P2\n" << T << " " << F << "\n255\n";
    for (size_t f = F; f-- > 0;) {
        for (size_t t = 0; t < T; ++t) {
            float v = std::clamp((db[t * F + f] - (mx - range_db)) / range_db, 0.0f, 1.0f);
            img << int(v * 255) << " ";
        }
        img << "\n";
    }
    std::cout << "Wrote spectrogram: " << T << " frames x " << F << " bins"
              << " (frame=" << frame_size << ", hop=" << hop_size
              << ", range=" << range_db << " dB)\n";
    return 0;
}
