#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "DSP/AlgorithmicReverb.h"
#include "DSP/FairingSeparation.h"
#include "Utils/Parameters.h"

//==============================================================================
/**
 * SeshNx Cosmos - Algorithmic Reverb Plugin
 *
 * A cinematic reverb with unstable space simulation featuring:
 * - Stage 1: Diffusion Thrust (density + low-mid emphasis)
 * - Stage 2: Modulation Chaos (complex multi-LFO modulation)
 * - Fairing Separation: Tempo-synced transition effect
 */
class CosmosAudioProcessor : public juce::AudioProcessor
{
public:
    CosmosAudioProcessor();
    ~CosmosAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter access
    juce::AudioProcessorValueTreeState& getParameters() { return parameters; }

    // Visualization data
    float getDecayEnvelope() const { return reverb.getDecayEnvelope(); }
    float getFairingSeparationIntensity() const { return fairingSeparation.getIntensity(); }
    bool isFairingSeparationActive() const { return fairingSeparation.getIsActive(); }

    // Input/Output levels for metering
    float getInputLevel(int channel) const { return inputLevels[channel].load(); }
    float getOutputLevel(int channel) const { return outputLevels[channel].load(); }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;

    // Cached parameter pointers
    std::atomic<float>* nebulaPresetParam = nullptr;
    std::atomic<float>* decayParam = nullptr;
    std::atomic<float>* preDelayParam = nullptr;
    std::atomic<float>* highCutParam = nullptr;
    std::atomic<float>* lowCutParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* widthParam = nullptr;
    std::atomic<float>* diffusionThrustParam = nullptr;
    std::atomic<float>* modulationChaosParam = nullptr;
    std::atomic<float>* fairingEnabledParam = nullptr;
    std::atomic<float>* fairingSyncParam = nullptr;
    std::atomic<float>* inputGainParam = nullptr;
    std::atomic<float>* outputGainParam = nullptr;

    // DSP components
    Cosmos::AlgorithmicReverb reverb;
    Cosmos::FairingSeparation fairingSeparation;

    // Dry buffer for wet/dry mixing
    juce::AudioBuffer<float> dryBuffer;

    // Metering
    std::array<std::atomic<float>, 2> inputLevels = { 0.0f, 0.0f };
    std::array<std::atomic<float>, 2> outputLevels = { 0.0f, 0.0f };

    // Previous fairing state for edge detection
    bool prevFairingEnabled = false;

    // Previous nebula preset for change detection
    int lastNebulaPreset = -1;

    // Apply nebula preset to parameters
    void applyNebulaPreset(int presetIndex);

    // Smoothed parameter values
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedInputGain;
    juce::SmoothedValue<float> smoothedOutputGain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CosmosAudioProcessor)
};
