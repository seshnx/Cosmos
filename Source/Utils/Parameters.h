#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace Cosmos
{

//==============================================================================
// Nebula Presets - Real nebulas with unique reverb characteristics
//==============================================================================
namespace NebulaPresets
{
    struct NebulaCharacter
    {
        const char* name;
        const char* description;
        float decay;           // seconds
        float diffusion;       // 0-100%
        float chaos;           // 0-100%
        float highCut;         // Hz
        float lowCut;          // Hz
        float width;           // 0-200%
        float preDelay;        // ms
    };

    // Real nebulas with creative reverb interpretations
    inline const std::array<NebulaCharacter, 12> presets = {{
        // 0 - Default/Manual
        { "Manual",
          "Custom settings - adjust parameters freely",
          5.0f, 50.0f, 30.0f, 12000.0f, 80.0f, 100.0f, 20.0f },

        // 1 - Pillars of Creation (Eagle Nebula M16)
        { "Pillars of Creation",
          "Towering columns of gas and dust - massive, slow-building reverb with deep low-end presence",
          15.0f, 75.0f, 25.0f, 8000.0f, 40.0f, 140.0f, 80.0f },

        // 2 - Crab Nebula (M1)
        { "Crab Nebula",
          "Supernova remnant with pulsar core - energetic, chaotic modulation with bright harmonics",
          8.0f, 60.0f, 85.0f, 16000.0f, 100.0f, 160.0f, 15.0f },

        // 3 - Orion Nebula (M42)
        { "Orion Nebula",
          "Stellar nursery with swirling gases - warm, enveloping decay with gentle modulation",
          12.0f, 80.0f, 40.0f, 10000.0f, 60.0f, 180.0f, 40.0f },

        // 4 - Helix Nebula (Eye of God)
        { "Helix Nebula",
          "Planetary nebula - circular, focused reverb with precise stereo imaging",
          6.0f, 55.0f, 20.0f, 14000.0f, 120.0f, 90.0f, 25.0f },

        // 5 - Horsehead Nebula (Barnard 33)
        { "Horsehead Nebula",
          "Dark nebula silhouette - deep, mysterious decay with subdued highs",
          18.0f, 70.0f, 35.0f, 6000.0f, 50.0f, 120.0f, 100.0f },

        // 6 - Ring Nebula (M57)
        { "Ring Nebula",
          "Perfect ring structure - balanced, symmetrical reverb with medium decay",
          7.0f, 65.0f, 30.0f, 11000.0f, 90.0f, 100.0f, 30.0f },

        // 7 - Carina Nebula
        { "Carina Nebula",
          "Massive star-forming region - expansive, dramatic reverb with intense dynamics",
          20.0f, 90.0f, 55.0f, 9000.0f, 45.0f, 200.0f, 60.0f },

        // 8 - Lagoon Nebula (M8)
        { "Lagoon Nebula",
          "Emission nebula with dark rifts - smooth, liquid decay with subtle movement",
          10.0f, 75.0f, 45.0f, 13000.0f, 70.0f, 150.0f, 35.0f },

        // 9 - Veil Nebula
        { "Veil Nebula",
          "Delicate supernova remnant - ethereal, wispy decay with high diffusion",
          14.0f, 95.0f, 50.0f, 15000.0f, 100.0f, 170.0f, 50.0f },

        // 10 - Cat's Eye Nebula (NGC 6543)
        { "Cat's Eye Nebula",
          "Complex planetary nebula - intricate, detailed reverb with focused center",
          5.0f, 45.0f, 60.0f, 18000.0f, 150.0f, 80.0f, 10.0f },

        // 11 - Tarantula Nebula (30 Doradus)
        { "Tarantula Nebula",
          "Most luminous nebula known - extremely bright, aggressive reverb with maximum spread",
          25.0f, 85.0f, 75.0f, 7000.0f, 35.0f, 200.0f, 120.0f }
    }};

    inline const juce::StringArray getNames()
    {
        juce::StringArray names;
        for (const auto& preset : presets)
            names.add(preset.name);
        return names;
    }

    inline const NebulaCharacter& getPreset(int index)
    {
        return presets[static_cast<size_t>(juce::jlimit(0, static_cast<int>(presets.size()) - 1, index))];
    }

    inline int getNumPresets() { return static_cast<int>(presets.size()); }
}

//==============================================================================
// Parameter IDs
//==============================================================================
namespace ParamIDs
{
    // Nebula preset selector
    inline const juce::String nebulaPreset { "nebulaPreset" };
    // Core Reverb Controls
    inline const juce::String decay { "decay" };               // Deep Space Decay
    inline const juce::String preDelay { "preDelay" };         // Launch Pre-Delay
    inline const juce::String highCut { "highCut" };           // High frequency damping
    inline const juce::String lowCut { "lowCut" };             // Low frequency damping
    inline const juce::String mix { "mix" };                   // Wet/Dry mix
    inline const juce::String width { "width" };               // Stereo width

    // Stage 1: Diffusion Thrust
    inline const juce::String diffusionThrust { "diffusionThrust" };

    // Stage 2: Modulation Chaos
    inline const juce::String modulationChaos { "modulationChaos" };

    // Fairing Separation (Transition FX)
    inline const juce::String fairingEnabled { "fairingEnabled" };
    inline const juce::String fairingSync { "fairingSync" };   // Tempo sync division

    // Input/Output
    inline const juce::String inputGain { "inputGain" };
    inline const juce::String outputGain { "outputGain" };
}

//==============================================================================
// Default Values
//==============================================================================
namespace Defaults
{
    // Core
    constexpr float decay = 5.0f;           // seconds (long, spacey default)
    constexpr float preDelay = 20.0f;       // ms
    constexpr float highCut = 12000.0f;     // Hz
    constexpr float lowCut = 80.0f;         // Hz
    constexpr float mix = 35.0f;            // percent
    constexpr float width = 100.0f;         // percent

    // Stage 1 & 2
    constexpr float diffusionThrust = 50.0f;    // percent
    constexpr float modulationChaos = 30.0f;    // percent

    // Fairing
    constexpr bool fairingEnabled = false;
    constexpr int fairingSync = 2;              // 1 bar default

    // I/O
    constexpr float inputGain = 0.0f;       // dB
    constexpr float outputGain = 0.0f;      // dB
}

//==============================================================================
// Parameter Ranges
//==============================================================================
namespace Ranges
{
    // Decay: 0.5s to 30s (exponential for long tails)
    constexpr float decayMin = 0.5f;
    constexpr float decayMax = 30.0f;
    constexpr float decaySkew = 0.4f;

    // Pre-delay: 0ms to 500ms
    constexpr float preDelayMin = 0.0f;
    constexpr float preDelayMax = 500.0f;

    // Filters
    constexpr float highCutMin = 1000.0f;
    constexpr float highCutMax = 20000.0f;
    constexpr float lowCutMin = 20.0f;
    constexpr float lowCutMax = 500.0f;

    // Gain
    constexpr float gainMin = -24.0f;
    constexpr float gainMax = 12.0f;
}

//==============================================================================
// Tempo Sync Options for Fairing Separation
//==============================================================================
namespace FairingSync
{
    inline const juce::StringArray options = {
        "1/4",      // 0 - Quarter note
        "1/2",      // 1 - Half note
        "1 Bar",    // 2 - One bar
        "2 Bars"    // 3 - Two bars
    };

    // Returns duration in beats
    inline float getBeats(int index)
    {
        switch (index)
        {
            case 0: return 1.0f;    // 1/4 = 1 beat
            case 1: return 2.0f;    // 1/2 = 2 beats
            case 2: return 4.0f;    // 1 bar = 4 beats
            case 3: return 8.0f;    // 2 bars = 8 beats
            default: return 4.0f;
        }
    }
}

//==============================================================================
// Parameter Layout Creation
//==============================================================================
inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Nebula Preset Selector
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParamIDs::nebulaPreset, 1 },
        "Nebula",
        NebulaPresets::getNames(),
        0));  // Default to "Manual"

    // Decay (Deep Space Decay) - skewed for long tails
    auto decayRange = juce::NormalisableRange<float>(
        Ranges::decayMin, Ranges::decayMax, 0.01f);
    decayRange.setSkewForCentre(3.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::decay, 1 },
        "Deep Space Decay",
        decayRange,
        Defaults::decay,
        juce::AudioParameterFloatAttributes().withLabel("s")));

    // Pre-Delay (Launch Pre-Delay)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::preDelay, 1 },
        "Launch Pre-Delay",
        juce::NormalisableRange<float>(Ranges::preDelayMin, Ranges::preDelayMax, 0.1f),
        Defaults::preDelay,
        juce::AudioParameterFloatAttributes().withLabel("ms")));

    // High Cut filter (frequency-skewed)
    auto highCutRange = juce::NormalisableRange<float>(
        Ranges::highCutMin, Ranges::highCutMax, 1.0f);
    highCutRange.setSkewForCentre(5000.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::highCut, 1 },
        "High Cut",
        highCutRange,
        Defaults::highCut,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // Low Cut filter (frequency-skewed)
    auto lowCutRange = juce::NormalisableRange<float>(
        Ranges::lowCutMin, Ranges::lowCutMax, 1.0f);
    lowCutRange.setSkewForCentre(100.0f);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::lowCut, 1 },
        "Low Cut",
        lowCutRange,
        Defaults::lowCut,
        juce::AudioParameterFloatAttributes().withLabel("Hz")));

    // Mix
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::mix, 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        Defaults::mix,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Width
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::width, 1 },
        "Width",
        juce::NormalisableRange<float>(0.0f, 200.0f, 0.1f),
        Defaults::width,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Stage 1: Diffusion Thrust
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::diffusionThrust, 1 },
        "Diffusion Thrust",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        Defaults::diffusionThrust,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Stage 2: Modulation Chaos
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::modulationChaos, 1 },
        "Modulation Chaos",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        Defaults::modulationChaos,
        juce::AudioParameterFloatAttributes().withLabel("%")));

    // Fairing Separation Toggle
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ ParamIDs::fairingEnabled, 1 },
        "Fairing Separation",
        Defaults::fairingEnabled));

    // Fairing Sync Selector
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ ParamIDs::fairingSync, 1 },
        "Fairing Sync",
        FairingSync::options,
        Defaults::fairingSync));

    // Input Gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::inputGain, 1 },
        "Input Gain",
        juce::NormalisableRange<float>(Ranges::gainMin, Ranges::gainMax, 0.1f),
        Defaults::inputGain,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    // Output Gain
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ ParamIDs::outputGain, 1 },
        "Output Gain",
        juce::NormalisableRange<float>(Ranges::gainMin, Ranges::gainMax, 0.1f),
        Defaults::outputGain,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    return { params.begin(), params.end() };
}

} // namespace Cosmos
