# Formant Shifter VST3

This project recreates the signal path visible in `ap_formant shifter.amxd` and
the supplied `fft-formant-shift-gen-m4l.maxpat` gen~ source:

- 2048-point FFT with 4x overlap (`pfft~ ... 2048 4`)
- polyphonic spectral formant translation in cents
- peak-based spectral envelope extraction using the gen~ codebox algorithm
- adjustable peak-region width (`envelope`, 0-16; default 10)
- dry/wet mix (0-100%; default 100%)
- optional wet limiter

The overlap-add output uses JUCE's inverse-FFT scaling exactly once. This is
important for a fully wet signal: applying an additional `1 / FFT size` gain
would make the processed output effectively silent.

The supplied `.amxd` references `fft-formant-shift-gen-m4l`; the accompanying
`.maxpat` now provides its exact gen~ codebox. The implementation follows that
codebox: it finds one local maximum per width-sized region, linearly
interpolates the maxima into an envelope, shifts the envelope by
`2^(formantShiftSemitones / 12)`, and multiplies it by the original
spectrum/envelope detail.

## Build

This is a JUCE/CMake VST3 project. CMake downloads JUCE 8.0.8 during configure:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

An MSVC or Clang C++ toolchain is required. The current machine has CMake but
does not expose a C++ compiler, so compilation cannot be completed here until
Visual Studio Build Tools (Desktop C++ workload) or LLVM is installed.
