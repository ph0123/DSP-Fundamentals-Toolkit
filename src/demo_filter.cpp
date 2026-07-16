#include <iostream>
#include "dsp/biquad.hpp"
#include "dsp/wav.hpp"

int main(int argc, char** argv) {
    if (argc < 3) { std::cerr << "Usage: demo_filter in.wav out.wav\n"; return 1; }
    auto wav = dsp::read_wav(argv[1]);

    // High-pass 100 Hz: removes rumble/DC — first block in every speech pipeline
    dsp::Biquad hp(dsp::Biquad::Type::HighPass, 100.0f, float(wav.sample_rate));
    for (auto& s : wav.samples) s = hp.process(s);

    dsp::write_wav(argv[2], wav);
    std::cout << "Applied 100 Hz high-pass filter\n";
    return 0;
}