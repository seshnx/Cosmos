#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

namespace Cosmos
{

//==============================================================================
/**
 * Fairing Separation - Tempo-synced Transition Effect
 *
 * When engaged, applies a dramatic momentary effect:
 * - Bandpass filter sweep (low to high or high to low)
 * - Short delay with stereo widening
 * - Gain swell
 *
 * Synchronized to DAW tempo using beat duration
 */
class FairingSeparation
{
public:
    FairingSeparation() = default;

    void prepare(double sr, int maxBlockSize)
    {
        sampleRate = sr;

        // Initialize bandpass filter
        juce::dsp::ProcessSpec spec;
        spec.sampleRate = sampleRate;
        spec.maximumBlockSize = static_cast<juce::uint32>(maxBlockSize);
        spec.numChannels = 2;

        bandpassFilter.prepare(spec);
        updateBandpassFilter(1000.0f); // Initial center frequency

        // Initialize short delay for widening effect (up to 50ms per channel)
        int maxDelaySamples = static_cast<int>(0.05 * sampleRate);
        for (int ch = 0; ch < 2; ++ch)
        {
            delayBuffer[ch].resize(static_cast<size_t>(maxDelaySamples));
            std::fill(delayBuffer[ch].begin(), delayBuffer[ch].end(), 0.0f);
        }
        delayWriteIndex = 0;
    }

    void reset()
    {
        bandpassFilter.reset();
        for (int ch = 0; ch < 2; ++ch)
        {
            std::fill(delayBuffer[ch].begin(), delayBuffer[ch].end(), 0.0f);
        }
        delayWriteIndex = 0;
        currentPhase = 0.0f;
        isActive = false;
        gainEnvelope = 0.0f;
    }

    // Set the tempo sync duration in beats
    void setSyncBeats(float beats)
    {
        syncBeats = juce::jlimit(1.0f, 8.0f, beats);
    }

    // Set BPM from host
    void setBPM(double bpm)
    {
        currentBPM = juce::jmax(20.0, bpm);
    }

    // Trigger the fairing separation effect
    void trigger()
    {
        if (!isActive)
        {
            isActive = true;
            currentPhase = 0.0f;
            sweepDirection = (lastSweepDirection > 0) ? -1 : 1; // Alternate direction
            lastSweepDirection = sweepDirection;
        }
    }

    // Release the effect (can be called to stop early)
    void release()
    {
        // Effect will naturally fade out
    }

    // Check if effect is currently active
    bool getIsActive() const { return isActive; }

    // Get current effect intensity for visualization (0-1)
    float getIntensity() const { return gainEnvelope; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        if (!isActive && gainEnvelope < 0.001f)
            return;

        int numSamples = buffer.getNumSamples();

        // Calculate effect duration in samples
        double beatsPerSecond = currentBPM / 60.0;
        double durationSeconds = syncBeats / beatsPerSecond;
        double durationSamples = durationSeconds * sampleRate;
        float phaseIncrement = 1.0f / static_cast<float>(durationSamples);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Update phase
            if (isActive)
            {
                currentPhase += phaseIncrement;
                if (currentPhase >= 1.0f)
                {
                    currentPhase = 1.0f;
                    isActive = false;
                }
            }

            // Calculate envelope (attack-sustain-release shape)
            float targetEnvelope = 0.0f;
            if (isActive)
            {
                if (currentPhase < 0.1f)
                {
                    // Attack (10% of duration)
                    targetEnvelope = currentPhase / 0.1f;
                }
                else if (currentPhase < 0.7f)
                {
                    // Sustain
                    targetEnvelope = 1.0f;
                }
                else
                {
                    // Release (30% of duration)
                    targetEnvelope = (1.0f - currentPhase) / 0.3f;
                }
            }

            // Smooth envelope
            gainEnvelope = gainEnvelope * 0.999f + targetEnvelope * 0.001f;

            // Calculate bandpass sweep frequency
            float sweepPhase = currentPhase;
            if (sweepDirection < 0)
                sweepPhase = 1.0f - sweepPhase;

            // Exponential frequency sweep from 200Hz to 8kHz
            float minFreq = 200.0f;
            float maxFreq = 8000.0f;
            float sweepFreq = minFreq * std::pow(maxFreq / minFreq, sweepPhase);

            // Only update filter periodically to avoid CPU spikes
            if (sample % 32 == 0)
            {
                updateBandpassFilter(sweepFreq);
            }

            // Calculate delay times for stereo widening
            // Offset increases with envelope
            float maxDelayMs = 15.0f * gainEnvelope;
            int leftDelaySamples = static_cast<int>(maxDelayMs * 0.3f * sampleRate / 1000.0f);
            int rightDelaySamples = static_cast<int>(maxDelayMs * sampleRate / 1000.0f);

            // Process delay/widening
            float leftIn = buffer.getSample(0, sample);
            float rightIn = buffer.getNumChannels() > 1 ? buffer.getSample(1, sample) : leftIn;

            // Write to delay buffers
            delayBuffer[0][static_cast<size_t>(delayWriteIndex)] = leftIn;
            delayBuffer[1][static_cast<size_t>(delayWriteIndex)] = rightIn;

            // Read from delay buffers
            int leftReadIndex = delayWriteIndex - leftDelaySamples;
            int rightReadIndex = delayWriteIndex - rightDelaySamples;
            if (leftReadIndex < 0) leftReadIndex += static_cast<int>(delayBuffer[0].size());
            if (rightReadIndex < 0) rightReadIndex += static_cast<int>(delayBuffer[1].size());

            float leftDelayed = delayBuffer[0][static_cast<size_t>(leftReadIndex)];
            float rightDelayed = delayBuffer[1][static_cast<size_t>(rightReadIndex)];

            delayWriteIndex = (delayWriteIndex + 1) % static_cast<int>(delayBuffer[0].size());

            // Mix dry with widened signal based on envelope
            float wideMix = gainEnvelope * 0.5f;
            float leftOut = leftIn * (1.0f - wideMix) + leftDelayed * wideMix;
            float rightOut = rightIn * (1.0f - wideMix) + rightDelayed * wideMix;

            // Cross-feed for extra width
            leftOut += rightDelayed * wideMix * 0.3f;
            rightOut += leftDelayed * wideMix * 0.3f;

            buffer.setSample(0, sample, leftOut);
            if (buffer.getNumChannels() > 1)
                buffer.setSample(1, sample, rightOut);
        }

        // Apply bandpass filter sweep
        if (gainEnvelope > 0.01f)
        {
            // Create temporary buffer for filtering
            juce::AudioBuffer<float> filterBuffer(buffer.getNumChannels(), numSamples);

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                filterBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
            }

            juce::dsp::AudioBlock<float> block(filterBuffer);
            juce::dsp::ProcessContextReplacing<float> context(block);
            bandpassFilter.process(context);

            // Mix filtered signal based on envelope
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                for (int sample = 0; sample < numSamples; ++sample)
                {
                    float dry = buffer.getSample(ch, sample);
                    float filtered = filterBuffer.getSample(ch, sample);
                    float mixed = dry * (1.0f - gainEnvelope * 0.7f) + filtered * gainEnvelope * 0.7f;
                    buffer.setSample(ch, sample, mixed);
                }
            }
        }

        // Apply subtle gain boost during effect
        float gainBoost = 1.0f + gainEnvelope * 0.3f;
        buffer.applyGain(gainBoost);
    }

private:
    void updateBandpassFilter(float centerFreq)
    {
        centerFreq = juce::jlimit(100.0f, 15000.0f, centerFreq);

        // Bandpass using cascaded LP and HP
        *bandpassFilter.state = *juce::dsp::IIR::Coefficients<float>::makeBandPass(
            sampleRate,
            centerFreq,
            2.0f  // Q for resonant sweep
        );
    }

    double sampleRate = 44100.0;
    double currentBPM = 120.0;
    float syncBeats = 4.0f;

    bool isActive = false;
    float currentPhase = 0.0f;
    float gainEnvelope = 0.0f;

    int sweepDirection = 1;
    int lastSweepDirection = -1;

    // Bandpass filter for sweep
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                   juce::dsp::IIR::Coefficients<float>> bandpassFilter;

    // Delay buffers for widening
    std::array<std::vector<float>, 2> delayBuffer;
    int delayWriteIndex = 0;
};

} // namespace Cosmos
