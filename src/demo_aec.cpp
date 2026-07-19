#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include "dsp/nlms.hpp"
#include "dsp/wav.hpp"

// Acoustic echo cancellation demo (single-talk).
//
// Scenario: the far-end signal is played out of a loudspeaker and leaks back
// into the microphone through the room (delay + reflections). The near-end
// talker is silent, so the mic captures echo only. NLMS uses the far-end as a
// reference to learn the echo path and subtract the echo from the mic.
//
//   mic = echo(far) + measurement noise      <- what the microphone records
//   out = mic - estimated_echo               <- after cancellation (~silence)
//
// We write BOTH signals so you can listen to the difference, and report ERLE
// (Echo Return Loss Enhancement): how much echo energy was removed, in dB.
//
// NOTE: this is single-talk on purpose. When far-end AND near-end talk at the
// same time ("double-talk"), plain NLMS adapts onto the near-end speech and
// cancellation degrades — real systems add a double-talk detector to freeze
// adaptation. The bundled aec_in_near.wav is a continuous double-talk clip, so
// it is not used here; it is kept as data for experimenting with that problem.
// Usage: demo_aec far_end.wav mic_echo.wav echo_cancelled.wav [taps] [mu] [eps]
//   taps  adaptive-filter length   (default 512 — must cover the echo tail)
//   mu    step size, 0<mu<2        (default 0.7 — bigger = faster but jitterier)
//   eps   regularizer floor        (default 1e-3 — too small and it diverges)
//
// Examples (tune on your own far-end recording):
//   demo_aec far.wav mic.wav out.wav               # defaults
//   demo_aec far.wav mic.wav out.wav 1024 1.0      # longer filter, faster adapt
//   demo_aec far.wav mic.wav out.wav 256 0.3 1e-2  # short, slow, heavily damped
int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: demo_aec far_end.wav mic_echo.wav echo_cancelled.wav"
                     " [taps] [mu] [eps]\n";
        return 1;
    }
    // --- tunable NLMS parameters (with defaults) ---
    size_t taps = (argc > 4) ? std::atoi(argv[4]) : 512;
    float  mu   = (argc > 5) ? std::atof(argv[5]) : 0.7f;
    float  eps  = (argc > 6) ? std::atof(argv[6]) : 1e-3f;

    auto far = dsp::read_wav(argv[1]);
    const size_t n = far.samples.size();

    // Simulated echo path: 200-sample delay + two weaker reflections,
    // plus a low measurement-noise floor so ERLE is realistic (not just
    // limited by float precision).
    uint32_t rng = 12345u;
    auto noise = [&]() {
        rng = rng * 1664525u + 1013904223u;
        return 1e-4f * (static_cast<int32_t>(rng) / 2147483648.0f);
    };
    dsp::WavData mic;
    mic.sample_rate = far.sample_rate;
    mic.samples.resize(n);
    for (size_t i = 0; i < n; ++i) {
        float echo = 0.0f;
        if (i >= 200) echo += 0.6f * far.samples[i - 200];
        if (i >= 350) echo += 0.3f * far.samples[i - 350];
        if (i >= 500) echo += 0.1f * far.samples[i - 500];
        mic.samples[i] = echo + noise();
    }

    // taps must cover the echo tail (the simulated path here is 500 samples).
    dsp::Nlms aec(taps, mu, eps);
    dsp::WavData out;
    out.sample_rate = far.sample_rate;
    out.samples.resize(n);
    for (size_t i = 0; i < n; ++i)
        out.samples[i] = aec.process(far.samples[i], mic.samples[i]);

    dsp::write_wav(argv[2], mic);
    dsp::write_wav(argv[3], out);

    // ERLE on the tail (after convergence): 10*log10(E[mic] / E[out]).
    double e_mic = 0, e_out = 0;
    for (size_t i = n / 2; i < n; ++i) {
        e_mic += mic.samples[i] * mic.samples[i];
        e_out += out.samples[i] * out.samples[i];
    }
    std::cout << "Wrote " << argv[2] << " (mic + echo) and " << argv[3] << " (cancelled)\n";
    std::cout << "NLMS: taps=" << taps << " mu=" << mu << " eps=" << eps << "\n";
    std::cout << "ERLE (converged, 2nd half): "
              << 10.0 * std::log10(e_mic / (e_out + 1e-12)) << " dB\n";
    return 0;
}
