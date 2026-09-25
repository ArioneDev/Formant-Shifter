# Formant Shifter VST3

- 2048-point FFT with 4x overlap (`pfft~ ... 2048 4`)
- polyphonic spectral formant translation in cents
- peak-based spectral envelope extraction using the gen~ codebox algorithm
- adjustable peak-region width (`envelope`, 0-16; default 10)
- dry/wet mix (0-100%; default 100%)
- optional wet limiter

The overlap-add output uses JUCE's inverse-FFT scaling exactly once. This is
important for a fully wet signal: applying an additional `1 / FFT size` gain
would make the processed output effectively silent.


## Build

This is a JUCE/CMake VST3 project. CMake downloads JUCE 8.0.8 during configure:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

An MSVC or Clang C++ toolchain is required.
