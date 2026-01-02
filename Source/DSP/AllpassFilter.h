#pragma once

#include <juce_dsp/juce_dsp.h>
#include <vector>

namespace Cosmos
{

//==============================================================================
/**
 * Modulated Allpass Filter for reverb diffusion
 * Supports variable delay time with interpolation for smooth modulation
 */
class AllpassFilter
{
public:
    AllpassFilter() = default;

    void prepare(double sampleRate, int maxDelaySamples)
    {
        this->sampleRate = sampleRate;
        this->maxDelay = maxDelaySamples;

        buffer.resize(static_cast<size_t>(maxDelaySamples + 4)); // Extra for interpolation
        std::fill(buffer.begin(), buffer.end(), 0.0f);

        writeIndex = 0;
    }

    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex = 0;
    }

    void setDelayTime(float delaySamples)
    {
        currentDelay = juce::jlimit(1.0f, static_cast<float>(maxDelay), delaySamples);
    }

    void setFeedback(float fb)
    {
        feedback = juce::jlimit(-0.99f, 0.99f, fb);
    }

    float process(float input)
    {
        // Read with linear interpolation for smooth modulation
        float readPos = static_cast<float>(writeIndex) - currentDelay;
        if (readPos < 0.0f)
            readPos += static_cast<float>(buffer.size());

        int readIndex0 = static_cast<int>(readPos);
        int readIndex1 = (readIndex0 + 1) % static_cast<int>(buffer.size());
        float frac = readPos - static_cast<float>(readIndex0);

        float delayed = buffer[static_cast<size_t>(readIndex0)] * (1.0f - frac)
                      + buffer[static_cast<size_t>(readIndex1)] * frac;

        // Allpass structure: y[n] = -g*x[n] + x[n-D] + g*y[n-D]
        float output = -feedback * input + delayed;
        buffer[static_cast<size_t>(writeIndex)] = input + feedback * delayed;

        writeIndex = (writeIndex + 1) % static_cast<int>(buffer.size());

        return output;
    }

    // Process with external modulation offset (in samples)
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

        float output = -feedback * input + delayed;
        buffer[static_cast<size_t>(writeIndex)] = input + feedback * delayed;

        writeIndex = (writeIndex + 1) % static_cast<int>(buffer.size());

        return output;
    }

private:
    std::vector<float> buffer;
    int writeIndex = 0;
    int maxDelay = 0;
    float currentDelay = 100.0f;
    float feedback = 0.5f;
    double sampleRate = 44100.0;
};

} // namespace Cosmos
