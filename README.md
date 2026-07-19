# DSP Toolkit — Audio Signal Processing Fundamentals in Modern C++
[![CI](https://github.com/ph0123/DSP-Fundamentals-Toolkit/actions/workflows/ci.yml/badge.svg)](https://github.com/ph0123/DSP-Fundamentals-Toolkit/actions/workflows/ci.yml)

From-scratch implementations of the core building blocks of real-time audio
processing — no external DSP libraries. Built as a learning-in-public project
to go deep on the fundamentals behind speech and communication systems.

![Spectrogram of speech](spectrogram.png)

## What's inside

| Module | File | Notes |
|---|---|---|
| WAV I/O | `include/dsp/wav.hpp` | 16-bit PCM mono reader/writer |
| FFT | `include/dsp/fft.hpp` | Iterative radix-2 Cooley–Tukey, in-place, O(N log N) |
| STFT / iSTFT | `include/dsp/stft.hpp` | Hann window, overlap-add reconstruction with COLA normalization |
| Biquad filters | `include/dsp/biquad.hpp` | RBJ Audio EQ Cookbook (low-pass / high-pass), Direct Form II Transposed |
| Adaptive filter | `include/dsp/nlms.hpp` | NLMS — demonstrated on acoustic echo cancellation |
| Ring buffer | `include/dsp/ring_buffer.hpp` | Lock-free SPSC, safe for real-time audio callbacks |
| data | `*.wav*` | Downloaded from [here](https://docs.espressif.com/projects/esp-sr/en/latest/esp32/acoustic_echo_cancellation/README.html)|


## Demos

```bash
mkdir build && cd build
cmake .. && make

# 1. Spectrogram of a speech file (writes a PGM image)
./demo_spectrogram ../data/aec_test_sr.wav spectrogram.pgm ; echo "Exit code: $?"

# 2. 100 Hz high-pass filter (removes rumble/DC — first block of any speech pipeline)
./demo_filter ../data/aec_test_sr.wav filtered.wav; echo "Exit code: $?"

# 3. Acoustic echo cancellation with a simulated echo path
./demo_aec ../data/aec_in_far.wav ../data/aec_in_near.wav echo_cancelled.wav ; echo "Exit code: $?"
# Prints ERLE (Echo Return Loss Enhancement)
```

## Results

- Echo cancellation: **[XX] dB ERLE** after convergence on a simulated
  3-reflection echo path (1024-tap NLMS, μ = 0.5)
- FFT round-trip error `ifft(fft(x)) − x`: **< [1e-6]** (see tests)

## Why from scratch?

Libraries like FFTW or KissFFT are obviously what you'd use in production.
The point here was to be able to answer *why*, not just *how*:

- **FFT** — why divide-and-conquer over even/odd samples turns O(N²) into
  O(N log N), and why a real input signal only needs N/2+1 bins
  (conjugate symmetry).
- **Windowing** — why cutting a signal into frames without a window causes
  spectral leakage, and what the Hann window trades away for it.
- **Overlap-add** — why the analysis/synthesis windows and hop size must
  satisfy the COLA condition for perfect reconstruction.
- **IIR vs FIR** — biquads are cheap (5 multiplies/sample) but have
  non-linear phase; FIR gives linear phase at much higher cost.
- **NLMS** — why normalizing the update by the reference-signal energy makes
  convergence speed independent of input level, and why echo cancellation
  is fundamentally *easier* than noise suppression: you have a reference
  signal for what the loudspeaker played.

## Build requirements

- C++17, CMake ≥ 3.16
- No external dependencies

## Tests (optional)

```bash
cd build && ctest
```

Covers: FFT of a 1 kHz sine peaks at the correct bin; `istft(stft(x)) ≈ x`;
biquad frequency response at cutoff ≈ −3 dB.

## License

MIT
