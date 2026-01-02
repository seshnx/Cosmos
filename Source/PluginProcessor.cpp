#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CosmosAudioProcessor::CosmosAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("CosmosParams"),
                 Cosmos::createParameterLayout())
{
    // Cache parameter pointers for real-time access
    nebulaPresetParam = parameters.getRawParameterValue(Cosmos::ParamIDs::nebulaPreset);
    decayParam = parameters.getRawParameterValue(Cosmos::ParamIDs::decay);
    preDelayParam = parameters.getRawParameterValue(Cosmos::ParamIDs::preDelay);
    highCutParam = parameters.getRawParameterValue(Cosmos::ParamIDs::highCut);
    lowCutParam = parameters.getRawParameterValue(Cosmos::ParamIDs::lowCut);
    mixParam = parameters.getRawParameterValue(Cosmos::ParamIDs::mix);
    widthParam = parameters.getRawParameterValue(Cosmos::ParamIDs::width);
    diffusionThrustParam = parameters.getRawParameterValue(Cosmos::ParamIDs::diffusionThrust);
    modulationChaosParam = parameters.getRawParameterValue(Cosmos::ParamIDs::modulationChaos);
    fairingEnabledParam = parameters.getRawParameterValue(Cosmos::ParamIDs::fairingEnabled);
    fairingSyncParam = parameters.getRawParameterValue(Cosmos::ParamIDs::fairingSync);
    inputGainParam = parameters.getRawParameterValue(Cosmos::ParamIDs::inputGain);
    outputGainParam = parameters.getRawParameterValue(Cosmos::ParamIDs::outputGain);
}

CosmosAudioProcessor::~CosmosAudioProcessor() {}

//==============================================================================
const juce::String CosmosAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CosmosAudioProcessor::acceptsMidi() const { return false; }
bool CosmosAudioProcessor::producesMidi() const { return false; }
bool CosmosAudioProcessor::isMidiEffect() const { return false; }

double CosmosAudioProcessor::getTailLengthSeconds() const
{
    // Return current decay time as tail length
    return static_cast<double>(decayParam->load());
}

//==============================================================================
int CosmosAudioProcessor::getNumPrograms() { return 1; }
int CosmosAudioProcessor::getCurrentProgram() { return 0; }
void CosmosAudioProcessor::setCurrentProgram(int) {}
const juce::String CosmosAudioProcessor::getProgramName(int) { return {}; }
void CosmosAudioProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================================
void CosmosAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Prepare DSP components
    reverb.prepare(sampleRate, samplesPerBlock);
    fairingSeparation.prepare(sampleRate, samplesPerBlock);

    // Prepare dry buffer
    dryBuffer.setSize(2, samplesPerBlock);

    // Initialize smoothed values
    smoothedMix.reset(sampleRate, 0.05);  // 50ms smoothing
    smoothedInputGain.reset(sampleRate, 0.02);
    smoothedOutputGain.reset(sampleRate, 0.02);

    smoothedMix.setCurrentAndTargetValue(mixParam->load() / 100.0f);
    smoothedInputGain.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(inputGainParam->load()));
    smoothedOutputGain.setCurrentAndTargetValue(
        juce::Decibels::decibelsToGain(outputGainParam->load()));
}

void CosmosAudioProcessor::releaseResources()
{
    reverb.reset();
    fairingSeparation.reset();
}

bool CosmosAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Support mono->stereo and stereo->stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // Input can be mono or stereo
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void CosmosAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    int numSamples = buffer.getNumSamples();

    // Check for nebula preset changes
    int currentNebulaPreset = static_cast<int>(nebulaPresetParam->load());
    if (currentNebulaPreset != lastNebulaPreset && currentNebulaPreset > 0)
    {
        // Nebula preset changed (and not "Manual")
        applyNebulaPreset(currentNebulaPreset);
        lastNebulaPreset = currentNebulaPreset;
    }
    else if (currentNebulaPreset == 0)
    {
        // Manual mode - track it but don't apply
        lastNebulaPreset = 0;
    }

    // Get parameter values
    float decay = decayParam->load();
    float preDelay = preDelayParam->load();
    float highCut = highCutParam->load();
    float lowCut = lowCutParam->load();
    float mix = mixParam->load() / 100.0f;
    float width = widthParam->load() / 100.0f;
    float diffusionThrust = diffusionThrustParam->load() / 100.0f;
    float modulationChaos = modulationChaosParam->load() / 100.0f;
    bool fairingEnabled = fairingEnabledParam->load() > 0.5f;
    int fairingSync = static_cast<int>(fairingSyncParam->load());
    float inputGain = juce::Decibels::decibelsToGain(inputGainParam->load());
    float outputGain = juce::Decibels::decibelsToGain(outputGainParam->load());

    // Update smoothed values
    smoothedMix.setTargetValue(mix);
    smoothedInputGain.setTargetValue(inputGain);
    smoothedOutputGain.setTargetValue(outputGain);

    // Apply input gain
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            data[i] *= smoothedInputGain.getNextValue();
        }
        // Reset for next channel
        smoothedInputGain.setCurrentAndTargetValue(inputGain);
    }

    // Update input meters
    for (int ch = 0; ch < juce::jmin(buffer.getNumChannels(), 2); ++ch)
    {
        float maxLevel = 0.0f;
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            maxLevel = juce::jmax(maxLevel, std::abs(data[i]));
        }
        inputLevels[static_cast<size_t>(ch)].store(maxLevel);
    }

    // Store dry signal
    dryBuffer.makeCopyOf(buffer);

    // Update reverb parameters
    reverb.setDecay(decay);
    reverb.setPreDelay(preDelay);
    reverb.setHighCut(highCut);
    reverb.setLowCut(lowCut);
    reverb.setWidth(width);
    reverb.setDiffusionThrust(diffusionThrust);
    reverb.setModulationChaos(modulationChaos);

    // Process reverb
    reverb.process(buffer);

    // Handle Fairing Separation
    if (fairingEnabled && !prevFairingEnabled)
    {
        // Rising edge - trigger effect
        // Get tempo from playhead
        if (auto* playHead = getPlayHead())
        {
            if (auto posInfo = playHead->getPosition())
            {
                if (posInfo->getBpm())
                {
                    fairingSeparation.setBPM(*posInfo->getBpm());
                }
            }
        }

        fairingSeparation.setSyncBeats(Cosmos::FairingSync::getBeats(fairingSync));
        fairingSeparation.trigger();
    }
    prevFairingEnabled = fairingEnabled;

    // Process fairing separation on the input (applied to wet signal)
    fairingSeparation.process(buffer);

    // Mix wet/dry
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* wetData = buffer.getWritePointer(ch);
        const float* dryData = dryBuffer.getReadPointer(juce::jmin(ch, dryBuffer.getNumChannels() - 1));

        for (int i = 0; i < numSamples; ++i)
        {
            float mixValue = smoothedMix.getNextValue();
            wetData[i] = dryData[i] * (1.0f - mixValue) + wetData[i] * mixValue;
        }
        // Reset for next channel
        smoothedMix.setCurrentAndTargetValue(mix);
    }

    // Apply output gain
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        float* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            data[i] *= smoothedOutputGain.getNextValue();
        }
        smoothedOutputGain.setCurrentAndTargetValue(outputGain);
    }

    // Update output meters
    for (int ch = 0; ch < juce::jmin(buffer.getNumChannels(), 2); ++ch)
    {
        float maxLevel = 0.0f;
        const float* data = buffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            maxLevel = juce::jmax(maxLevel, std::abs(data[i]));
        }
        outputLevels[static_cast<size_t>(ch)].store(maxLevel);
    }
}

//==============================================================================
bool CosmosAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* CosmosAudioProcessor::createEditor()
{
    return new CosmosAudioProcessorEditor(*this);
}

//==============================================================================
void CosmosAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void CosmosAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
void CosmosAudioProcessor::applyNebulaPreset(int presetIndex)
{
    const auto& preset = Cosmos::NebulaPresets::getPreset(presetIndex);

    // Apply preset values to parameters
    if (auto* param = parameters.getParameter(Cosmos::ParamIDs::decay))
        param->setValueNotifyingHost(param->convertTo0to1(preset.decay));

    if (auto* param = parameters.getParameter(Cosmos::ParamIDs::preDelay))
        param->setValueNotifyingHost(param->convertTo0to1(preset.preDelay));

    if (auto* param = parameters.getParameter(Cosmos::ParamIDs::highCut))
        param->setValueNotifyingHost(param->convertTo0to1(preset.highCut));

    if (auto* param = parameters.getParameter(Cosmos::ParamIDs::lowCut))
        param->setValueNotifyingHost(param->convertTo0to1(preset.lowCut));

    if (auto* param = parameters.getParameter(Cosmos::ParamIDs::width))
        param->setValueNotifyingHost(param->convertTo0to1(preset.width));

    if (auto* param = parameters.getParameter(Cosmos::ParamIDs::diffusionThrust))
        param->setValueNotifyingHost(param->convertTo0to1(preset.diffusion));

    if (auto* param = parameters.getParameter(Cosmos::ParamIDs::modulationChaos))
        param->setValueNotifyingHost(param->convertTo0to1(preset.chaos));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CosmosAudioProcessor();
}
