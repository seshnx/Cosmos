#pragma once

#include "AllpassFilter.h"
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace Cosmos
{

//==============================================================================
/**
 * Multi-stage Diffusion Network for early reflections
 * Implements "Diffusion Thrust" (Stage 1) with density control
 * and low-mid frequency emphasis for "thrust" character
 */
class DiffusionNetwork
{
public:
    static constexpr int NumStages = 8;      // Number of allpass stages
    static constexpr int NumChannels = 2;    // Stereo

    DiffusionNetwork() = default;

    void prepare(double sr, int maxBlockSize)
    {
        sampleRate = sr;

        // Prime-based delay times for inharmonic diffusion (in samples at 44.1kHz)
        // Scaled for sample rate
        const std::array<float, NumStages> delayTimesMs = {
            1.3f, 2.1f, 3.4f, 5.5f, 8.9f, 14.4f, 23.3f, 37.7f
        };

        for (int ch = 0; ch < NumChannels; ++ch)
        {
            for (int i = 0; i < NumStages; ++i)
            {
                // Slight offset between channels for stereo decorrelation
                float offset = (ch == 0) ? 0.0f : 0.07f;
                int delaySamples = static_cast<int>((delayTimesMs[static_cast<size_t>(i)] + offset)
                                                    * sampleRate / 1000.0);
                allpassFilters[static_cast<size_t>(ch)][static_cast<size_t>(i)].prepare(sampleRate, delaySamples + 100);
                allpassFilters[static_cast<size_t>(ch)][static_cast<size_t>(i)].setDelayTime(static_cast<float>(delaySamples));
                allpassFilters[static_cast<size_t>(ch)][static_cast<size_t>(i)].setFeedback(0.5f);
            }
        }

        // Low-mid emphasis filter (shelf boost around 200-800Hz)
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
        spec.numChannels = NumChannels;

        lowMidFilter.prepare(spec);
        updateThrustFilter();
    }

    void reset()
    {
        for (int ch = 0; ch < NumChannels; ++ch)
        {
            for (int i = 0; i < NumStages; ++i)
            {
                allpassFilters[static_cast<size_t>(ch)][static_cast<size_t>(i)].reset();
            }
        }
        lowMidFilter.reset();
    }

    // Set diffusion thrust amount (0-1)
    // Affects density and low-mid emphasis
    void setThrust(float thrust)
    {
        thrustAmount = juce::jlimit(0.0f, 1.0f, thrust);

        // Map thrust to feedback coefficients
        // Higher thrust = more diffusion density
        float baseFeedback = 0.3f + thrustAmount * 0.45f; // 0.3 to 0.75

        for (int ch = 0; ch < NumChannels; ++ch)
        {
            for (int i = 0; i < NumStages; ++i)
            {
                // Vary feedback slightly per stage for complexity
                float stageFeedback = baseFeedback + (static_cast<float>(i) / NumStages) * 0.1f;
                allpassFilters[static_cast<size_t>(ch)][static_cast<size_t>(i)].setFeedback(
                    juce::jlimit(0.0f, 0.75f, stageFeedback));
            }
        }

        updateThrustFilter();
    }

    // Get the number of active stages based on thrust
    int getActiveStages() const
    {
        // At low thrust, use fewer stages; at high thrust, use all
        return static_cast<int>(2 + thrustAmount * (NumStages - 2));
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int activeStages = getActiveStages();

        for (int ch = 0; ch < juce::jmin(buffer.getNumChannels(), NumChannels); ++ch)
        {
            float* data = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                float sample = data[i];

                // Process through active allpass stages
                for (int stage = 0; stage < activeStages; ++stage)
                {
                    sample = allpassFilters[static_cast<size_t>(ch)][static_cast<size_t>(stage)].process(sample);
                }

                data[i] = sample;
            }
        }

        // Apply low-mid emphasis filter when thrust is engaged
        if (thrustAmount > 0.01f)
        {
            juce::dsp::AudioBlock<float> block(buffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            lowMidFilter.process(context);
        }
    }

private:
    void updateThrustFilter()
    {
        // Low shelf boost for "thrust" effect - emphasizes 200-800Hz range
        float boostDb = thrustAmount * 6.0f; // 0 to 6dB boost

        *lowMidFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(
            sampleRate,
            400.0f,     // Shelf frequency
            0.7f,       // Q
            juce::Decibels::decibelsToGain(boostDb)
        );
    }

    std::array<std::array<AllpassFilter, NumStages>, NumChannels> allpassFilters;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> lowMidFilter;

    double sampleRate = 44100.0;
    float thrustAmount = 0.5f;
};

} // namespace Cosmos
