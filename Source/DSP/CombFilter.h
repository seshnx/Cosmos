#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

namespace Cosmos
{

//==============================================================================
/**
 * Lowpass Feedback Comb Filter for reverb decay
 * Includes integrated damping filter for frequency-dependent decay
 */
class CombFilter
{
public:
    CombFilter() = default;

    void prepare(double sr, int maxDelaySamples)
    {
        sampleRate = sr;
        maxDelay = maxDelaySamples;

        buffer.resize(static_cast<size_t>(maxDelaySamples + 4));
        std::fill(buffer.begin(), buffer.end(), 0.0f);

        writeIndex = 0;
        filterState = 0.0f;
    }

    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex = 0;
        filterState = 0.0f;
    }

    void setDelayTime(float delaySamples)
    {
        currentDelay = juce::jlimit(1.0f, static_cast<float>(maxDelay), delaySamples);
    }

    void setFeedback(float fb)
    {
        feedback = juce::jlimit(0.0f, 0.999f, fb);
    }

    // Set damping coefficient (0 = no damping, 1 = full damping)
    void setDamping(float damp)
    {
        damping = juce::jlimit(0.0f, 0.999f, damp);
    }

    float process(float input)
    {
        // Read from delay line with interpolation
        float readPos = static_cast<float>(writeIndex) - currentDelay;
        if (readPos < 0.0f)
            readPos += static_cast<float>(buffer.size());

        int readIndex0 = static_cast<int>(readPos);
        int readIndex1 = (readIndex0 + 1) % static_cast<int>(buffer.size());
        float frac = readPos - static_cast<float>(readIndex0);

        float delayed = buffer[static_cast<size_t>(readIndex0)] * (1.0f - frac)
                      + buffer[static_cast<size_t>(readIndex1)] * frac;

        // Apply one-pole lowpass filter for frequency-dependent decay
        filterState = delayed * (1.0f - damping) + filterState * damping;

        // Write back with feedback
        buffer[static_cast<size_t>(writeIndex)] = input + filterState * feedback;

        writeIndex = (writeIndex + 1) % static_cast<int>(buffer.size());

        return delayed;
    }

    // Process with modulation
    float processModulated(float input, float modOffset)
    {
        float modulatedDelay = currentDelay + modOffset;
        modulatedDelay = juce::jlimit(1.0f, static_cast<float>(maxDelay), modulatedDelay);

        float readPos = static_cast<float>(writeIndex) - modulatedDelay;
        if (readPos < 0.0f)
            readPos += static_cast<float>(buffer.size());

        int readIndex0 = static_cast<int>(readPos);
        int readIndex1 = (readIndex0 + 1) % static_cast<int>(buffer.size());
        float frac = readPos - static_cast<float>(readIndex0);

        float delayed = buffer[static_cast<size_t>(readIndex0)] * (1.0f - frac)
                      + buffer[static_cast<size_t>(readIndex1)] * frac;

        filterState = delayed * (1.0f - damping) + filterState * damping;
        buffer[static_cast<size_t>(writeIndex)] = input + filterState * feedback;

        writeIndex = (writeIndex + 1) % static_cast<int>(buffer.size());

        return delayed;
    }

private:
    std::vector<float> buffer;
    int writeIndex = 0;
    int maxDelay = 0;
    float currentDelay = 1000.0f;
    float feedback = 0.7f;
    float damping = 0.3f;
    float filterState = 0.0f;
    double sampleRate = 44100.0;
};

} // namespace Cosmos
