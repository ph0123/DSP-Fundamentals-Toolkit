#pragma once
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace dsp {

struct WavData {
    uint32_t sample_rate = 16000;
    std::vector<float> samples;  // normalized to [-1, 1]
};

#pragma pack(push, 1)
struct WavHeader {
    char     riff[4];        // "RIFF"
    uint32_t chunk_size;
    char     wave[4];        // "WAVE"
    char     fmt[4];         // "fmt "
    uint32_t fmt_size;       // 16 for PCM
    uint16_t audio_format;   // 1 = PCM
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};
#pragma pack(pop)

inline WavData read_wav(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open " + path);

    WavHeader h{};
    f.read(reinterpret_cast<char*>(&h), sizeof(h));
    if (std::string(h.riff, 4) != "RIFF" || std::string(h.wave, 4) != "WAVE")
        throw std::runtime_error("Not a WAV file");
    if (h.audio_format != 1 || h.bits_per_sample != 16 || h.num_channels != 1)
        throw std::runtime_error("Only 16-bit PCM mono supported");

    // Skip extra fmt bytes if any, then find "data" chunk
    f.seekg(20 + h.fmt_size, std::ios::beg);
    char id[4]; uint32_t size = 0;
    while (f.read(id, 4) && f.read(reinterpret_cast<char*>(&size), 4)) {
        if (std::string(id, 4) == "data") break;
        f.seekg(size, std::ios::cur);
    }

    std::vector<int16_t> raw(size / 2);
    f.read(reinterpret_cast<char*>(raw.data()), size);

    WavData out;
    out.sample_rate = h.sample_rate;
    out.samples.resize(raw.size());
    for (size_t i = 0; i < raw.size(); ++i)
        out.samples[i] = raw[i] / 32768.0f;
    return out;
}

inline void write_wav(const std::string& path, const WavData& w) {
    std::ofstream f(path, std::ios::binary);
    const uint32_t data_size = static_cast<uint32_t>(w.samples.size() * 2);

    WavHeader h{};
    std::copy_n("RIFF", 4, h.riff);
    std::copy_n("WAVE", 4, h.wave);
    std::copy_n("fmt ", 4, h.fmt);
    h.chunk_size      = 36 + data_size;
    h.fmt_size        = 16;
    h.audio_format    = 1;
    h.num_channels    = 1;
    h.sample_rate     = w.sample_rate;
    h.bits_per_sample = 16;
    h.block_align     = 2;
    h.byte_rate       = w.sample_rate * 2;

    f.write(reinterpret_cast<const char*>(&h), sizeof(h));
    f.write("data", 4);
    f.write(reinterpret_cast<const char*>(&data_size), 4);
    for (float s : w.samples) {
        float c = std::max(-1.0f, std::min(1.0f, s));
        int16_t v = static_cast<int16_t>(c * 32767.0f);
        f.write(reinterpret_cast<const char*>(&v), 2);
    }
}

}  // namespace dsp