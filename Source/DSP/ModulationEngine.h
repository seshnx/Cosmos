#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>
#include <random>

namespace Cosmos
{

//==============================================================================
/**
 * Complex Multi-LFO Modulation Engine for reverb tail animation
 * Implements "Modulation Chaos" (Stage 2)
 *
 * Design goals:
 * - Avoid metallic artifacts through non-periodic, complex modulation
 * - Multiple LFOs with irrational frequency ratios (golden ratio based)
 * - Smooth, interpolated output suitable for delay line modulation
 * - Rich, organic movement even at high chaos settings
 */
class ModulationEngine
{
public:
    static constexpr int NumLFOs = 6;
    static constexpr int NumOutputs = 8;  // Modulation signals for 8 delay lines

    ModulationEngine() = default;

    void prepare(double sr)
    {
        sampleRate = sr;

        // Initialize LFOs with golden ratio-based frequency relationships
        // These irrational ratios prevent periodic repetition
        const float goldenRatio = 1.618033988749895f;
        const float baseFreq = 0.23f; // Very slow base frequency

        for (int i = 0; i < NumLFOs; ++i)
        {
            float freqMultiplier = std::pow(goldenRatio, static_cast<float>(i) * 0.7f);
            lfoFrequencies[static_cast<size_t>(i)] = baseFreq * freqMultiplier;
            lfoPhases[static_cast<size_t>(i)] = static_cast<float>(i) * 0.37f; // Spread initial phases

            // Randomize starting phases slightly for more organic behavior
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(0.0f, juce::MathConstants<float>::twoPi);
            lfoPhases[static_cast<size_t>(i)] += dist(gen) * 0.3f;
        }

        // Initialize drift (slow random walk)
        for (int i = 0; i < NumOutputs; ++i)
        {
            driftValues[static_cast<size_t>(i)] = 0.0f;
            driftTargets[static_cast<size_t>(i)] = 0.0f;
        }

        updateDriftTargets();
    }

    void reset()
    {
        for (int i = 0; i < NumLFOs; ++i)
        {
            lfoPhases[static_cast<size_t>(i)] = static_cast<float>(i) * 0.37f;
        }

        for (int i = 0; i < NumOutputs; ++i)
        {
            driftValues[static_cast<size_t>(i)] = 0.0f;
            driftTargets[static_cast<size_t>(i)] = 0.0f;
            smoothedOutputs[static_cast<size_t>(i)] = 0.0f;
        }
    }

    // Set chaos amount (0-1)
    // Affects modulation rate, depth, and complexity
    void setChaos(float chaos)
    {
        chaosAmount = juce::jlimit(0.0f, 1.0f, chaos);

        // Update LFO frequencies based on chaos
        const float goldenRatio = 1.618033988749895f;
        float baseFreq = 0.15f + chaosAmount * 0.5f; // 0.15 to 0.65 Hz

        for (int i = 0; i < NumLFOs; ++i)
        {
            float freqMultiplier = std::pow(goldenRatio, static_cast<float>(i) * 0.7f);
            lfoFrequencies[static_cast<size_t>(i)] = baseFreq * freqMultiplier;
        }

        // Update modulation depth
        maxDepthSamples = 20.0f + chaosAmount * 60.0f; // 20 to 80 samples max deviation
    }

    // Get modulation offset for a specific delay line (in samples)
    float getModulation(int outputIndex) const
    {
        if (outputIndex < 0 || outputIndex >= NumOutputs)
            return 0.0f;

        return smoothedOutputs[static_cast<size_t>(outputIndex)];
    }

    // Process one sample worth of modulation
    void processSample()
    {
        // Update LFO phases
        for (int i = 0; i < NumLFOs; ++i)
        {
            float phaseIncrement = lfoFrequencies[static_cast<size_t>(i)]
                                 / static_cast<float>(sampleRate);
            lfoPhases[static_cast<size_t>(i)] += phaseIncrement * juce::MathConstants<float>::twoPi;

            if (lfoPhases[static_cast<size_t>(i)] > juce::MathConstants<float>::twoPi)
                lfoPhases[static_cast<size_t>(i)] -= juce::MathConstants<float>::twoPi;
        }

        // Update drift (very slow random walk)
        driftCounter++;
        if (driftCounter > static_cast<int>(sampleRate * 2.0)) // Update every 2 seconds
        {
            updateDriftTargets();
            driftCounter = 0;
        }

        // Smooth drift towards targets
        float driftSmooth = 0.9999f;
        for (int i = 0; i < NumOutputs; ++i)
        {
            driftValues[static_cast<size_t>(i)] = driftValues[static_cast<size_t>(i)] * driftSmooth
                                                + driftTargets[static_cast<size_t>(i)] * (1.0f - driftSmooth);
        }

        // Calculate output modulations by mixing LFOs
        // Each output uses a unique combination for maximum decorrelation
        for (int out = 0; out < NumOutputs; ++out)
        {
            float modValue = 0.0f;

            // Mix multiple LFOs with different weights
            for (int lfo = 0; lfo < NumLFOs; ++lfo)
            {
                // Create unique mixing matrix using prime-based weights
                float weight = getMixWeight(out, lfo);

                // Use different wave shapes for different LFOs
                float lfoValue = getLFOValue(lfo);
                modValue += lfoValue * weight;
            }

            // Add drift component for extra complexity at high chaos
            modValue += driftValues[static_cast<size_t>(out)] * chaosAmount * 0.3f;

            // Scale to sample range
            modValue *= maxDepthSamples;

            // Smooth output to prevent clicks
            float smoothCoeff = 0.995f;
            smoothedOutputs[static_cast<size_t>(out)] = smoothedOutputs[static_cast<size_t>(out)] * smoothCoeff
                                                      + modValue * (1.0f - smoothCoeff);
        }
    }

private:
    float getLFOValue(int lfoIndex) const
    {
        float phase = lfoPhases[static_cast<size_t>(lfoIndex)];

        // Use different wave shapes for complexity
        switch (lfoIndex % 4)
        {
            case 0: // Sine
                return std::sin(phase);

            case 1: // Smoothed triangle
            {
                float tri = (phase < juce::MathConstants<float>::pi)
                          ? (2.0f * phase / juce::MathConstants<float>::pi - 1.0f)
                          : (3.0f - 2.0f * phase / juce::MathConstants<float>::pi);
                // Cubic smoothing
                return tri * tri * tri * 0.5f + tri * 0.5f;
            }

            case 2: // Sine with harmonics (richer)
                return std::sin(phase) * 0.7f + std::sin(phase * 2.0f) * 0.2f
                     + std::sin(phase * 3.0f) * 0.1f;

            case 3: // Asymmetric sine
            {
                float s = std::sin(phase);
                return s * (1.0f + 0.3f * s * s); // Slightly asymmetric
            }

            default:
                return std::sin(phase);
        }
    }

    float getMixWeight(int outputIndex, int lfoIndex) const
    {
        // Create pseudo-random but deterministic mixing weights
        // Using prime numbers for good distribution
        static const int primes[] = { 7, 11, 13, 17, 19, 23, 29, 31 };

        int seed = primes[outputIndex % 8] * (lfoIndex + 1) + outputIndex;
        float weight = (static_cast<float>(seed % 100) / 100.0f) - 0.5f;

        // Ensure first LFOs have more influence
        weight *= (1.0f - static_cast<float>(lfoIndex) * 0.12f);

        return weight;
    }

    void updateDriftTargets()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

        for (int i = 0; i < NumOutputs; ++i)
        {
            driftTargets[static_cast<size_t>(i)] = dist(gen);
        }
    }

    double sampleRate = 44100.0;
    float chaosAmount = 0.3f;
    float maxDepthSamples = 40.0f;

    std::array<float, NumLFOs> lfoPhases = {};
    std::array<float, NumLFOs> lfoFrequencies = {};

    std::array<float, NumOutputs> driftValues = {};
    std::array<float, NumOutputs> driftTargets = {};
    std::array<float, NumOutputs> smoothedOutputs = {};

    int driftCounter = 0;
};

} // namespace Cosmos
