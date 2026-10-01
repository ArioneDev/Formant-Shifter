# Formant Shifter

![License](https://img.shields.io/badge/license-GPL-blue) ![Language](https://img.shields.io/badge/language-c++-brightgreen) [![Ask Me Anything !](https://img.shields.io/badge/Ask%20me-anything-1abc9c.svg)](https://GitHub.com/Naereen/ama) ![](https://komarev.com/ghpvc/?username=ArioneDev)

![1.1.0](https://bee-reg-ab.imagency.cn/mr/6787/26/6abdb7e7525d7.png)

- two modes : music / vocal
- 2048-point FFT with 4x overlap (`pfft~ ... 2048 4`)
- polyphonic spectral formant translation in cents
- adjustable peak-region width (`envelope`, 0-16; default 10)
- dry/wet mix (0-100%; default 100%)

## Build

This is a JUCE/CMake VST3 project. CMake downloads JUCE 8.0.8 during configure:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

An MSVC or Clang C++ toolchain is required.
