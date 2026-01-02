#pragma once

#include "PluginProcessor.h"
#include "UI/CosmosLookAndFeel.h"
#include "UI/StarfieldVisualizer.h"
#include "UI/EngineKnob.h"
#include "UI/DecayCurveDisplay.h"
#include "UI/NebulaSelectorPanel.h"
#include "Utils/Parameters.h"
#include "BinaryData.h"

//==============================================================================
/**
 * SeshNx Cosmos Plugin Editor
 *
 * Space-themed UI with:
 * - Animated starfield background
 * - Engine dial knobs for all parameters
 * - Decay curve visualizer
 * - Stage 1/2 controls with distinctive styling
 * - Fairing separation controls
 */
class CosmosAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   public juce::Timer
{
public:
    CosmosAudioProcessorEditor(CosmosAudioProcessor&);
    ~CosmosAudioProcessorEditor() override;

    //==========================================================================
    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    CosmosAudioProcessor& audioProcessor;

    // Company logo
    juce::Image companyLogo;

    // Custom look and feel
    Cosmos::CosmosLookAndFeel cosmosLookAndFeel;

    // Background visualizer
    Cosmos::StarfieldVisualizer starfield;

    // Decay curve display
    Cosmos::DecayCurveDisplay decayCurve;

    // Core controls
    Cosmos::EngineKnob decayKnob { "DECAY", Cosmos::EngineKnob::Style::Standard };
    Cosmos::EngineKnob preDelayKnob { "PRE-DELAY", Cosmos::EngineKnob::Style::Standard };
    Cosmos::EngineKnob highCutKnob { "HIGH CUT", Cosmos::EngineKnob::Style::Standard };
    Cosmos::EngineKnob lowCutKnob { "LOW CUT", Cosmos::EngineKnob::Style::Standard };
    Cosmos::EngineKnob mixKnob { "MIX", Cosmos::EngineKnob::Style::Standard };
    Cosmos::EngineKnob widthKnob { "WIDTH", Cosmos::EngineKnob::Style::Standard };

    // Stage controls
    Cosmos::EngineKnob thrustKnob { "THRUST", Cosmos::EngineKnob::Style::Thrust };
    Cosmos::EngineKnob chaosKnob { "CHAOS", Cosmos::EngineKnob::Style::Chaos };

    // I/O controls
    Cosmos::EngineKnob inputGainKnob { "INPUT", Cosmos::EngineKnob::Style::Standard };
    Cosmos::EngineKnob outputGainKnob { "OUTPUT", Cosmos::EngineKnob::Style::Standard };

    // Fairing separation controls
    juce::TextButton fairingButton { "FAIRING SEPARATION" };
    juce::ComboBox fairingSyncCombo;
    juce::Label fairingSyncLabel { {}, "SYNC" };

    // Nebula selector panel
    Cosmos::NebulaSelectorPanel nebulaPanel;

    // Section labels
    juce::Label titleLabel { {}, "COSMOS" };
    juce::Label subtitleLabel { {}, "ALGORITHMIC REVERB" };
    juce::Label stage1Label { {}, "STAGE 1: DIFFUSION" };
    juce::Label stage2Label { {}, "STAGE 2: MODULATION" };
    juce::Label coreLabel { {}, "SPACE CONTROLS" };
    juce::Label fairingLabel { {}, "TRANSITION FX" };

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preDelayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> widthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thrustAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> chaosAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fairingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> fairingSyncAttachment;

    //==========================================================================
    void setupKnobs();
    void setupLabels();
    void setupFairingControls();
    void setupNebulaSelector();
    void attachParameters();
    void applyNebulaPresetToUI(int presetIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CosmosAudioProcessorEditor)
};
