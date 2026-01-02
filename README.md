# SeshNx Cosmos

**Algorithmic Reverb - Cinematic Space Simulation**

A dense algorithmic reverb optimized for long, highly modulated decay times with unique dual-stage controls and tempo-synced transition effects.

**Part of the SeshNx Plugin Suite by Amalia Media LLC**

## Features

### Core Reverb Engine
- Dense, true stereo algorithmic reverb
- 8 parallel modulated comb filters with Hadamard mixing
- Multi-stage diffusion network for early reflections
- High/Low frequency damping with shelving filters
- Decay times from 0.5s to 30s
- Pre-delay up to 500ms
- Stereo width control (0-200%)

### Stage 1: Diffusion Thrust
Increases density and applies frequency-dependent filtering to the early reflections:
- Variable allpass diffusion stages (2-8 active stages)
- Low-mid shelf boost for "thrust" character
- Creates denser, warmer diffusion as thrust increases

### Stage 2: Modulation Chaos
Complex, non-periodic pitch and time modulation in the reverb tail:
- 6 LFOs with golden ratio-based frequency relationships
- Multiple waveform shapes (sine, smoothed triangle, asymmetric)
- Slow random drift for extra complexity
- Designed to avoid metallic artifacts

### Fairing Separation (Transition FX)
Tempo-synced dramatic effect for builds and transitions:
- Bandpass filter sweep (alternating directions)
- Short delay with stereo widening
- Sync options: 1/4, 1/2, 1 bar, 2 bars
- Receives DAW tempo for accurate sync

## Controls

| Control | Range | Description |
|---------|-------|-------------|
| **Decay** | 0.5s - 30s | Deep Space Decay time (RT60) |
| **Pre-Delay** | 0 - 500ms | Launch Pre-Delay |
| **High Cut** | 1kHz - 20kHz | High frequency damping |
| **Low Cut** | 20Hz - 500Hz | Low frequency roll-off |
| **Mix** | 0 - 100% | Wet/Dry balance |
| **Width** | 0 - 200% | Stereo width |
| **Thrust** | 0 - 100% | Stage 1: Diffusion density |
| **Chaos** | 0 - 100% | Stage 2: Modulation complexity |
| **Fairing** | Toggle | Enable transition effect |
| **Sync** | 1/4 - 2 bars | Fairing duration |
| **Input/Output** | -24 to +12dB | Gain staging |

## UI Theme

Space-flight inspired design:
- Deep space black/blue color palette
- Animated starfield background (reacts to parameters)
- Glowing engine-dial style knobs
- Color-coded controls by function:
  - **Blue**: Core reverb controls
  - **Orange**: Stage 1 (Thrust)
  - **Violet**: Stage 2 (Chaos)
  - **Cyan**: Fairing Separation

## Build Instructions

### Requirements
- CMake 3.22+
- C++17 compatible compiler
- JUCE 8.0.0 (downloaded automatically)

### Build Commands

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

### Output Locations
```
Cosmos_artefacts/
├── Release/
│   ├── VST3/Cosmos.vst3
│   ├── AU/Cosmos.component  (macOS only)
│   └── Standalone/Cosmos
```

## Architecture

```
Source/
├── PluginProcessor.cpp/h    # Main audio engine
├── PluginEditor.cpp/h       # UI implementation
├── DSP/
│   ├── AllpassFilter.h      # Modulated allpass for diffusion
│   ├── CombFilter.h         # Lowpass feedback comb
│   ├── DiffusionNetwork.h   # Stage 1 diffusion (Thrust)
│   ├── ModulationEngine.h   # Stage 2 multi-LFO (Chaos)
│   ├── AlgorithmicReverb.h  # Main reverb algorithm
│   └── FairingSeparation.h  # Tempo-synced transition FX
├── UI/
│   ├── CosmosLookAndFeel.h  # Space theme styling
│   ├── StarfieldVisualizer.h # Animated background
│   ├── EngineKnob.h         # Custom rotary control
│   └── DecayCurveDisplay.h  # Decay visualization
└── Utils/
    └── Parameters.h         # Parameter definitions
```

## Technical Notes

- **Thread Safety**: All parameters use atomic access from audio thread
- **Modulation Design**: Golden ratio frequency relationships prevent periodic artifacts
- **Tempo Sync**: Fairing Separation reads host tempo via AudioPlayHead
- **Oversampling**: Not required - algorithm designed for alias-free operation
- **CPU Efficiency**: Inline implementations for critical DSP paths

---

## Version

**v1.0.0**

---

## License

Copyright (c) 2024 Amalia Media LLC. All rights reserved.

Proprietary software - Distribution prohibited without explicit permission.

---

## Support

For technical support, bug reports, or feature requests, please contact the development team through official SeshNx channels.

---

*Part of the [SeshNx Plugin Suite](https://seshnx.com) by Amalia Media LLC*
