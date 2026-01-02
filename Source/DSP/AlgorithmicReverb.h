#pragma once

#include "DiffusionNetwork.h"
#include "CombFilter.h"
#include "ModulationEngine.h"
#include <juce_dsp/juce_dsp.h>
#include <array>

namespace Cosmos
{

//==============================================================================
/**
 * Dense Algorithmic Reverb optimized for long, cinematic decay
 *
 * Architecture:
 * - Pre-delay line
 * - Diffusion network (Stage 1: Diffusion Thrust)
 * - 8 parallel modulated comb filters with Hadamard mixing
 * - Modulation engine (Stage 2: Modulation Chaos)
 * - High/Low shelving filters for tonal shaping
 * - True stereo processing with width control
 */
class AlgorithmicReverb
{
public:
    static constexpr int NumCombFilters = 8;

    AlgorithmicReverb() = default;

    void prepare(double sr, int maxBlockSize)
    {
        sampleRate = sr;
        blockSize = maxBlockSize;

        // Pre-delay: up to 500ms
        int maxPreDelaySamples = static_cast<int>(0.5 * sampleRate);
        for (int ch = 0; ch < 2; ++ch)
        {
            preDelayBuffer[ch].resize(static_cast<size_t>(maxPreDelaySamples));
            std::fill(preDelayBuffer[ch].begin(), preDelayBuffer[ch].end(), 0.0f);
        }
        preDelayWriteIndex = 0;

        // Initialize diffusion network
        diffusionNetwork.prepare(sampleRate, maxBlockSize);

        // Initialize comb filters with prime-based delay times
        // Delay times in ms - chosen for density without flutter echo
        const std::array<float, NumCombFilters> delayTimesMs = {
            29.7f, 37.1f, 41.1f, 43.7f, 47.3f, 53.0f, 59.3f, 67.1f
        };

        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < NumCombFilters; ++i)
            {
                // Slight stereo offset
                float offset = (ch == 0) ? 0.0f : 1.7f;
                int delaySamples = static_cast<int>((delayTimesMs[static_cast<size_t>(i)] + offset)
                                                    * sampleRate / 1000.0);

                combFilters[ch][static_cast<size_t>(i)].prepare(sampleRate, delaySamples + 200);
                combFilters[ch][static_cast<size_t>(i)].setDelayTime(static_cast<float>(delaySamples));
            }
        }

        // Initialize modulation engine
        modulationEngine.prepare(sampleRate);

        // Initialize filters
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
        spec.numChannels = 2;

        highCutFilter.prepare(spec);
        lowCutFilter.prepare(spec);

        updateFilters();
        updateDecay();
    }

    void reset()
    {
        for (int ch = 0; ch < 2; ++ch)
        {
            std::fill(preDelayBuffer[ch].begin(), preDelayBuffer[ch].end(), 0.0f);
            for (int i = 0; i < NumCombFilters; ++i)
            {
                combFilters[ch][static_cast<size_t>(i)].reset();
            }
        }
        preDelayWriteIndex = 0;
        diffusionNetwork.reset();
        modulationEngine.reset();
        highCutFilter.reset();
        lowCutFilter.reset();
    }

    // Set decay time in seconds
    void setDecay(float decaySeconds)
    {
        decayTime = juce::jlimit(0.5f, 30.0f, decaySeconds);
        updateDecay();
    }

    // Set pre-delay in milliseconds
    void setPreDelay(float preDelayMs)
    {
        preDelaySamples = static_cast<int>(preDelayMs * sampleRate / 1000.0);
        preDelaySamples = juce::jlimit(0, static_cast<int>(preDelayBuffer[0].size()) - 1,
                                       preDelaySamples);
    }

    // Set high cut frequency
    void setHighCut(float freqHz)
    {
        highCutFreq = juce::jlimit(1000.0f, 20000.0f, freqHz);
        updateFilters();
    }

    // Set low cut frequency
    void setLowCut(float freqHz)
    {
        lowCutFreq = juce::jlimit(20.0f, 500.0f, freqHz);
        updateFilters();
    }

    // Set stereo width (0 = mono, 1 = normal, 2 = extra wide)
    void setWidth(float w)
    {
        width = juce::jlimit(0.0f, 2.0f, w);
    }

    // Set diffusion thrust (Stage 1)
    void setDiffusionThrust(float thrust)
    {
        diffusionNetwork.setThrust(thrust);
    }

    // Set modulation chaos (Stage 2)
    void setModulationChaos(float chaos)
    {
        modulationEngine.setChaos(chaos);
    }

    // Get decay envelope value for visualization (0-1)
    float getDecayEnvelope() const { return decayEnvelope; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        int numSamples = buffer.getNumSamples();
        int numChannels = juce::jmin(buffer.getNumChannels(), 2);

        // Create wet buffer
        juce::AudioBuffer<float> wetBuffer(2, numSamples);
        wetBuffer.clear();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Update modulation engine
            modulationEngine.processSample();

            // Read from pre-delay
            int preDelayReadIndex = preDelayWriteIndex - preDelaySamples;
            if (preDelayReadIndex < 0)
                preDelayReadIndex += static_cast<int>(preDelayBuffer[0].size());

            float leftIn = (numChannels > 0) ? buffer.getSample(0, sample) : 0.0f;
            float rightIn = (numChannels > 1) ? buffer.getSample(1, sample) : leftIn;

            // Write to pre-delay
            preDelayBuffer[0][static_cast<size_t>(preDelayWriteIndex)] = leftIn;
            preDelayBuffer[1][static_cast<size_t>(preDelayWriteIndex)] = rightIn;
            preDelayWriteIndex = (preDelayWriteIndex + 1) % static_cast<int>(preDelayBuffer[0].size());

            // Read from pre-delay
            float leftDelayed = preDelayBuffer[0][static_cast<size_t>(preDelayReadIndex)];
            float rightDelayed = preDelayBuffer[1][static_cast<size_t>(preDelayReadIndex)];

            wetBuffer.setSample(0, sample, leftDelayed);
            wetBuffer.setSample(1, sample, rightDelayed);
        }

        // Apply diffusion network (Stage 1)
        diffusionNetwork.process(wetBuffer);

        // Process through comb filter bank with modulation
        for (int sample = 0; sample < numSamples; ++sample)
        {
            for (int ch = 0; ch < 2; ++ch)
            {
                float input = wetBuffer.getSample(ch, sample);
                float combSum = 0.0f;

                // Sum outputs from all comb filters
                for (int i = 0; i < NumCombFilters; ++i)
                {
                    // Get modulation for this comb filter
                    float modOffset = modulationEngine.getModulation(i);

                    // Process with modulation
                    float combOut = combFilters[ch][static_cast<size_t>(i)].processModulated(
                        input / static_cast<float>(NumCombFilters), modOffset);

                    // Apply Hadamard-style mixing (alternating signs)
                    float sign = ((i + ch) % 2 == 0) ? 1.0f : -1.0f;
                    combSum += combOut * sign;
                }

                wetBuffer.setSample(ch, sample, combSum);
            }
        }

        // Apply frequency-dependent damping
        juce::dsp::AudioBlock<float> wetBlock(wetBuffer);
        juce::dsp::ProcessContextReplacing<float> wetContext(wetBlock);
        highCutFilter.process(wetContext);
        lowCutFilter.process(wetContext);

        // Apply stereo width
        if (std::abs(width - 1.0f) > 0.01f)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                float left = wetBuffer.getSample(0, sample);
                float right = wetBuffer.getSample(1, sample);

                float mid = (left + right) * 0.5f;
                float side = (left - right) * 0.5f * width;

                wetBuffer.setSample(0, sample, mid + side);
                wetBuffer.setSample(1, sample, mid - side);
            }
        }

        // Update decay envelope for visualization
        float maxSample = 0.0f;
        for (int ch = 0; ch < wetBuffer.getNumChannels(); ++ch)
        {
            for (int sample = 0; sample < numSamples; ++sample)
            {
                maxSample = juce::jmax(maxSample, std::abs(wetBuffer.getSample(ch, sample)));
            }
        }
        decayEnvelope = decayEnvelope * 0.99f + maxSample * 0.01f;

        // Copy wet signal back to buffer
        for (int ch = 0; ch < numChannels; ++ch)
        {
            buffer.copyFrom(ch, 0, wetBuffer, ch, 0, numSamples);
        }
    }

private:
    void updateDecay()
    {
        // Calculate feedback coefficient for desired RT60
        // RT60 = -60dB decay time
        // For a comb filter: feedback = 10^(-3 * delayTime / RT60)
        // We use the average delay time

        const std::array<float, NumCombFilters> delayTimesMs = {
            29.7f, 37.1f, 41.1f, 43.7f, 47.3f, 53.0f, 59.3f, 67.1f
        };

        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < NumCombFilters; ++i)
            {
                float delayMs = delayTimesMs[static_cast<size_t>(i)] + (ch == 0 ? 0.0f : 1.7f);
                float delaySeconds = delayMs / 1000.0f;

                // Calculate feedback for RT60
                float feedback = std::pow(10.0f, -3.0f * delaySeconds / decayTime);
                feedback = juce::jlimit(0.0f, 0.998f, feedback);

                combFilters[ch][static_cast<size_t>(i)].setFeedback(feedback);

                // Set damping based on high cut (more damping = faster HF decay)
                float dampingAmount = 1.0f - (highCutFreq - 1000.0f) / 19000.0f;
                dampingAmount = juce::jlimit(0.0f, 0.7f, dampingAmount * 0.7f);
                combFilters[ch][static_cast<size_t>(i)].setDamping(dampingAmount);
            }
        }
    }

    void updateFilters()
    {
        *highCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(
            sampleRate, highCutFreq, 0.707f);

        *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(
            sampleRate, lowCutFreq, 0.707f);
    }

    double sampleRate = 44100.0;
    int blockSize = 512;

    // Pre-delay
    std::array<std::vector<float>, 2> preDelayBuffer;
    int preDelayWriteIndex = 0;
    int preDelaySamples = 0;

    // Diffusion network (Stage 1)
    DiffusionNetwork diffusionNetwork;

    // Comb filter bank
    std::array<std::array<CombFilter, NumCombFilters>, 2> combFilters;

    // Modulation engine (Stage 2)
    ModulationEngine modulationEngine;

    // Damping filters
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> highCutFilter;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> lowCutFilter;

    // Parameters
    float decayTime = 5.0f;
    float highCutFreq = 12000.0f;
    float lowCutFreq = 80.0f;
    float width = 1.0f;

    // Visualization
    float decayEnvelope = 0.0f;
};

} // namespace Cosmos
