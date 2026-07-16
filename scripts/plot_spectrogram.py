#!/usr/bin/env python3
import sys
import os
import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy import signal

def plot_spectrogram(wav_path, output_png):
    # 1. Read WAV file
    sample_rate, data = wavfile.read(wav_path)
    
    # If stereo (2 channels), select only the first channel
    if len(data.shape) > 1:
        data = data[:, 0]
        
    # Normalize amplitude to [-1.0, 1.0] depending on input bit depth
    if data.dtype == np.int16:
        data = data.astype(np.float32) / 32768.0
    elif data.dtype == np.int32:
        data = data.astype(np.float32) / 2147483648.0

    # 2. Compute Spectrogram using Short-Time Fourier Transform (STFT)
    # nperseg=512 corresponds to the frame_size in your C++ code
    # noverlap=384 corresponds to hop_size = 128 (512 - 128 = 384)
    frequencies, times, spectrogram = signal.spectrogram(
        data, 
        fs=sample_rate, 
        window='hann', 
        nperseg=512, 
        noverlap=384
    )

    # Convert amplitude to Decibel (dB) scale
    # Add 1e-10 to prevent taking the log of zero
    spectrogram_db = 10 * np.log10(spectrogram + 1e-10)

    # 3. Plot the spectrogram
    plt.figure(figsize=(10, 6))
    
    # Draw the spectrogram using pcolormesh (X-axis: time, Y-axis: frequency)
    # cmap='viridis' or 'magma' are beautiful standard colormaps for audio
    plt.pcolormesh(times, frequencies, spectrogram_db, shading='gouraud', cmap='magma')
    
    # Limit the dB display range (from maximum down to 80 dB)
    max_db = np.max(spectrogram_db)
    plt.clim(max_db - 80, max_db)

    # Add plot labels and title
    plt.title(f"Spectrogram of {os.path.basename(wav_path)}", fontsize=14, fontweight='bold')
    plt.xlabel("Time (seconds)", fontsize=12)
    plt.ylabel("Frequency (Hz)", fontsize=12)
    plt.ylim(0, sample_rate / 2)  # Limit Y-axis to Nyquist frequency
    
    # Add colorbar to indicate magnitude in dB
    cbar = plt.colorbar()
    cbar.set_label("Magnitude (dB)", fontsize=12)

    plt.tight_layout()
    
    # 4. Save plot as a PNG image
    plt.savefig(output_png, dpi=150)
    plt.close()
    print(f"[SUCCESS] Saved spectrogram to: {output_png}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python3 plot_spectrogram.py <path_to_wav_file> <output_image_name.png>")
        sys.exit(1)
        
    wav_path = sys.argv[1]
    output_png = sys.argv[2]
    
    if not os.path.exists(wav_path):
        print(f"[ERROR] WAV file not found: {wav_path}")
        sys.exit(1)
        
    plot_spectrogram(wav_path, output_png)