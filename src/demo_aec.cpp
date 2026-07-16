#include <iostream>
#include <random>
#include "dsp/nlms.hpp"
#include "dsp/wav.hpp"

// Simulate: mic = echo(far_end) + near_end, then cancel the echo with NLMS.
int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: demo_aec far_end.wav near_end.wav out.wav\n";
        return 1;
    }
    auto far  = dsp::read_wav(argv[1]);
    auto near = dsp::read_wav(argv[2]);
    const size_t n = std::min(far.samples.size(), near.samples.size());

    // Simulated echo path: delay 200 samples + a couple of reflections
    std::vector<float> mic(n, 0.0f);
    for (size_t i = 0; i < n; ++i) {
        float echo = 0.0f;
        if (i >= 200) echo += 0.6f * far.samples[i - 200];
        if (i >= 350) echo += 0.3f * far.samples[i - 350];
        if (i >= 500) echo += 0.1f * far.samples[i - 500];
        mic[i] = echo + near.samples[i];
    }

    dsp::Nlms aec(1024, 0.5f);
    dsp::WavData out;
    out.sample_rate = far.sample_rate;
    out.samples.resize(n);
    for (size_t i = 0; i < n; ++i)
        out.samples[i] = aec.process(far.samples[i], mic[i]);

    dsp::write_wav(argv[3], out);

    // Report ERLE (Echo Return Loss Enhancement) on the tail (after convergence)
    double e_mic = 0, e_out = 0;
    for (size_t i = n / 2; i < n; ++i) { e_mic += mic[i]*mic[i]; e_out += out.samples[i]*out.samples[i]; }
    std::cout << "ERLE (approx, 2nd half): "
              << 10.0 * std::log10(e_mic / (e_out + 1e-12)) << " dB\n";
    return 0;
}