#include <cstdlib>
#include <iostream>
#include <string>
#include "dsp/biquad.hpp"
#include "dsp/wav.hpp"

// Biquad IIR filter demo.
//
// Usage: demo_filter in.wav out.wav [type] [cutoff_hz] [q]
//   type      hp | lp        (default hp — high-pass)
//   cutoff_hz cutoff freq    (default 100)
//   q         resonance/Q    (default 0.7071 = Butterworth, maximally flat)
//
// Examples (tune on your own data without recompiling):
//   demo_filter in.wav out.wav                 # 100 Hz high-pass (remove rumble)
//   demo_filter in.wav out.wav lp 3400         # 3.4 kHz low-pass (telephone band)
//   demo_filter in.wav out.wav hp 300 2.0      # 300 Hz high-pass, resonant
int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: demo_filter in.wav out.wav [hp|lp] [cutoff_hz] [q]\n";
        return 1;
    }
    // --- tunable parameters (with defaults) ---
    std::string type_s = (argc > 3) ? argv[3] : "hp";
    float cutoff       = (argc > 4) ? std::atof(argv[4]) : 100.0f;
    float q            = (argc > 5) ? std::atof(argv[5]) : 0.7071f;

    auto type = dsp::Biquad::Type::HighPass;
    if (type_s == "lp" || type_s == "lowpass") type = dsp::Biquad::Type::LowPass;
    else if (type_s != "hp" && type_s != "highpass") {
        std::cerr << "Unknown filter type '" << type_s << "' (use hp or lp)\n";
        return 1;
    }

    auto wav = dsp::read_wav(argv[1]);

    dsp::Biquad filter(type, cutoff, float(wav.sample_rate), q);
    for (auto& s : wav.samples) s = filter.process(s);

    dsp::write_wav(argv[2], wav);
    std::cout << "Applied " << (type == dsp::Biquad::Type::LowPass ? "low" : "high")
              << "-pass @ " << cutoff << " Hz (Q=" << q << ")\n";
    return 0;
}
