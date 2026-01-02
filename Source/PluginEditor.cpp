#include "PluginEditor.h"

//==============================================================================
CosmosAudioProcessorEditor::CosmosAudioProcessorEditor(CosmosAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Load company logo
    if (BinaryData::company_logo_png != nullptr && BinaryData::company_logo_pngSize > 0)
    {
        companyLogo = juce::ImageCache::getFromMemory(BinaryData::company_logo_png,
                                                       BinaryData::company_logo_pngSize);
    }

    // Apply custom look and feel
    setLookAndFeel(&cosmosLookAndFeel);

    // Add starfield background
    addAndMakeVisible(starfield);

    // Add decay curve display
    addAndMakeVisible(decayCurve);

    // Setup components
    setupKnobs();
    setupLabels();
    setupFairingControls();
    setupNebulaSelector();
    attachParameters();

    // Set initial size
    setSize(900, 600);
    setResizable(true, true);
    setResizeLimits(700, 500, 1200, 800);

    // Start timer for UI updates
    startTimerHz(30);
}

CosmosAudioProcessorEditor::~CosmosAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
void CosmosAudioProcessorEditor::setupKnobs()
{
    // Core knobs
    addAndMakeVisible(decayKnob);
    decayKnob.setValueSuffix(" s");
    decayKnob.setValuePrecision(1);

    addAndMakeVisible(preDelayKnob);
    preDelayKnob.setValueSuffix(" ms");
    preDelayKnob.setValuePrecision(0);

    addAndMakeVisible(highCutKnob);
    highCutKnob.setValueSuffix(" Hz");
    highCutKnob.setValuePrecision(0);

    addAndMakeVisible(lowCutKnob);
    lowCutKnob.setValueSuffix(" Hz");
    lowCutKnob.setValuePrecision(0);

    addAndMakeVisible(mixKnob);
    mixKnob.setValueSuffix("%");
    mixKnob.setValuePrecision(0);

    addAndMakeVisible(widthKnob);
    widthKnob.setValueSuffix("%");
    widthKnob.setValuePrecision(0);

    // Stage knobs
    addAndMakeVisible(thrustKnob);
    thrustKnob.setValueSuffix("%");
    thrustKnob.setValuePrecision(0);

    addAndMakeVisible(chaosKnob);
    chaosKnob.setValueSuffix("%");
    chaosKnob.setValuePrecision(0);

    // I/O knobs
    addAndMakeVisible(inputGainKnob);
    inputGainKnob.setValueSuffix(" dB");
    inputGainKnob.setValuePrecision(1);

    addAndMakeVisible(outputGainKnob);
    outputGainKnob.setValueSuffix(" dB");
    outputGainKnob.setValuePrecision(1);
}

void CosmosAudioProcessorEditor::setupLabels()
{
    // Title (left-aligned for header)
    titleLabel.setFont(juce::Font(juce::FontOptions(24.0f).withStyle("Bold")));
    titleLabel.setColour(juce::Label::textColourId, Cosmos::CosmosLookAndFeel::Colors::starWhite);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    subtitleLabel.setColour(juce::Label::textColourId, Cosmos::CosmosLookAndFeel::Colors::textSecondary);
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(subtitleLabel);

    // Section labels
    auto setupSectionLabel = [this](juce::Label& label, juce::Colour color) {
        label.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
        label.setColour(juce::Label::textColourId, color);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    setupSectionLabel(stage1Label, Cosmos::CosmosLookAndFeel::Colors::thrustOrange);
    setupSectionLabel(stage2Label, Cosmos::CosmosLookAndFeel::Colors::chaosViolet);
    setupSectionLabel(coreLabel, Cosmos::CosmosLookAndFeel::Colors::cosmicBlue);
    setupSectionLabel(fairingLabel, Cosmos::CosmosLookAndFeel::Colors::fairingCyan);
}

void CosmosAudioProcessorEditor::setupFairingControls()
{
    // Fairing button
    fairingButton.setName("fairing");
    fairingButton.setClickingTogglesState(true);
    addAndMakeVisible(fairingButton);

    // Sync combo
    fairingSyncCombo.addItemList(Cosmos::FairingSync::options, 1);
    fairingSyncCombo.setSelectedId(3); // Default to 1 bar
    addAndMakeVisible(fairingSyncCombo);

    // Sync label
    fairingSyncLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    fairingSyncLabel.setColour(juce::Label::textColourId, Cosmos::CosmosLookAndFeel::Colors::textSecondary);
    fairingSyncLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(fairingSyncLabel);
}

void CosmosAudioProcessorEditor::setupNebulaSelector()
{
    // Nebula selector panel
    addAndMakeVisible(nebulaPanel);

    // Get initial value from parameter
    if (auto* param = audioProcessor.getParameters().getParameter(Cosmos::ParamIDs::nebulaPreset))
    {
        int currentIndex = static_cast<int>(param->convertFrom0to1(param->getValue()));
        nebulaPanel.setCurrentIndex(currentIndex);
        starfield.setNebulaIndex(currentIndex);
    }

    // Connect selector to parameter and update background
    nebulaPanel.onNebulaChanged = [this](int index)
    {
        if (auto* param = audioProcessor.getParameters().getParameter(Cosmos::ParamIDs::nebulaPreset))
        {
            param->setValueNotifyingHost(param->convertTo0to1(static_cast<float>(index)));
        }
        // Update starfield background
        starfield.setNebulaIndex(index);

        // Apply preset parameters immediately (skip Manual mode at index 0)
        if (index > 0)
        {
            applyNebulaPresetToUI(index);
        }
    };
}

void CosmosAudioProcessorEditor::attachParameters()
{
    auto& params = audioProcessor.getParameters();

    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Cosmos::ParamIDs::decay, decayKnob.getSlider());

    preDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Cosmos::ParamIDs::preDelay, preDelayKnob.getSlider());

    highCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Cosmos::ParamIDs::highCut, highCutKnob.getSlider());

    lowCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Cosmos::ParamIDs::lowCut, lowCutKnob.getSlider());

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Cosmos::ParamIDs::mix, mixKnob.getSlider());

    widthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Cosmos::ParamIDs::width, widthKnob.getSlider());

    thrustAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Cosmos::ParamIDs::diffusionThrust, thrustKnob.getSlider());

    chaosAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Cosmos::ParamIDs::modulationChaos, chaosKnob.getSlider());

    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Cosmos::ParamIDs::inputGain, inputGainKnob.getSlider());

    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        params, Cosmos::ParamIDs::outputGain, outputGainKnob.getSlider());

    fairingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        params, Cosmos::ParamIDs::fairingEnabled, fairingButton);

    fairingSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        params, Cosmos::ParamIDs::fairingSync, fairingSyncCombo);
}

//==============================================================================
void CosmosAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Starfield handles the background
    auto bounds = getLocalBounds();

    // Header bar background
    const int headerHeight = 55;
    auto headerArea = bounds.removeFromTop(headerHeight);
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.fillRect(headerArea);

    // Header border line
    g.setColour(Cosmos::CosmosLookAndFeel::Colors::cosmicBlue.withAlpha(0.5f));
    g.drawLine(0.0f, static_cast<float>(headerHeight), static_cast<float>(getWidth()),
               static_cast<float>(headerHeight), 1.5f);

    // Semi-transparent panels for sections
    auto drawPanel = [&g](juce::Rectangle<int> area, juce::Colour borderColor) {
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillRoundedRectangle(area.toFloat(), 8.0f);
        g.setColour(borderColor.withAlpha(0.3f));
        g.drawRoundedRectangle(area.toFloat(), 8.0f, 1.0f);
    };

    // Calculate panel areas (approximate - will match layout)
    int panelPadding = 10;
    int topHeight = headerHeight + 5;
    int stageHeight = 140;
    int coreHeight = 160;

    // Stage 1 panel (left)
    auto stage1Area = juce::Rectangle<int>(panelPadding, topHeight + panelPadding,
                                            getWidth() / 2 - panelPadding * 2, stageHeight);
    drawPanel(stage1Area, Cosmos::CosmosLookAndFeel::Colors::thrustOrange);

    // Stage 2 panel (right)
    auto stage2Area = juce::Rectangle<int>(getWidth() / 2 + panelPadding, topHeight + panelPadding,
                                            getWidth() / 2 - panelPadding * 2, stageHeight);
    drawPanel(stage2Area, Cosmos::CosmosLookAndFeel::Colors::chaosViolet);

    // Core controls panel
    auto coreArea = juce::Rectangle<int>(panelPadding, topHeight + stageHeight + panelPadding * 2,
                                          getWidth() - panelPadding * 2, coreHeight);
    drawPanel(coreArea, Cosmos::CosmosLookAndFeel::Colors::cosmicBlue);

    // Nebula panel (full width)
    int nebulaY = topHeight + stageHeight + coreHeight + panelPadding * 3;
    auto nebulaArea = juce::Rectangle<int>(panelPadding, nebulaY, getWidth() - panelPadding * 2, 50);
    // Nebula panel draws its own background

    // Bottom row panels
    int bottomY = nebulaY + 60;
    int bottomWidth = (getWidth() - panelPadding * 3) / 2;

    // Fairing panel (left)
    auto fairingArea = juce::Rectangle<int>(panelPadding, bottomY, bottomWidth, 70);
    drawPanel(fairingArea, Cosmos::CosmosLookAndFeel::Colors::fairingCyan);

    // I/O panel (right)
    auto ioArea = juce::Rectangle<int>(panelPadding * 2 + bottomWidth, bottomY, bottomWidth, 70);
    drawPanel(ioArea, Cosmos::CosmosLookAndFeel::Colors::textSecondary);
}

void CosmosAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    // Draw company logo centered in header
    if (!companyLogo.isValid())
        return;

    const int headerHeight = 55;
    const float logoHeight = 35.0f;
    const float logoAspect = static_cast<float>(companyLogo.getWidth()) /
                             static_cast<float>(companyLogo.getHeight());
    const float logoWidth = logoHeight * logoAspect;

    const float logoX = (getWidth() - logoWidth) * 0.5f;
    const float logoY = (headerHeight - logoHeight) * 0.5f;

    juce::Rectangle<float> logoBounds(logoX, logoY, logoWidth, logoHeight);
    g.drawImage(companyLogo, logoBounds, juce::RectanglePlacement::centred);
}

void CosmosAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    int padding = 10;
    int knobSize = 90;
    const int headerHeight = 55;

    // Starfield covers entire background
    starfield.setBounds(bounds);

    // Header area with title on left
    auto headerArea = bounds.removeFromTop(headerHeight);
    auto titleSection = headerArea.reduced(20, 0);
    titleLabel.setBounds(titleSection.removeFromTop(30).withTrimmedTop(8));
    subtitleLabel.setBounds(titleSection.removeFromTop(18));

    bounds.removeFromTop(5); // Gap after header

    // Stage controls row
    auto stageRow = bounds.removeFromTop(140);

    // Stage 1 (left half)
    auto stage1Area = stageRow.removeFromLeft(stageRow.getWidth() / 2).reduced(padding);
    stage1Label.setBounds(stage1Area.removeFromTop(20));
    auto stage1KnobArea = stage1Area;
    thrustKnob.setBounds(stage1KnobArea.removeFromLeft(knobSize + 20).reduced(5));

    // Decay curve in Stage 1 area
    decayCurve.setBounds(stage1KnobArea.reduced(5, 10));

    // Stage 2 (right half)
    auto stage2Area = stageRow.reduced(padding);
    stage2Label.setBounds(stage2Area.removeFromTop(20));
    chaosKnob.setBounds(stage2Area.removeFromLeft(knobSize + 20).reduced(5));

    // Core controls row
    auto coreRow = bounds.removeFromTop(160).reduced(padding);
    coreLabel.setBounds(coreRow.removeFromTop(20));

    // 6 knobs in a row
    int coreKnobWidth = (coreRow.getWidth() - padding * 5) / 6;
    decayKnob.setBounds(coreRow.removeFromLeft(coreKnobWidth).reduced(2));
    preDelayKnob.setBounds(coreRow.removeFromLeft(coreKnobWidth).reduced(2));
    highCutKnob.setBounds(coreRow.removeFromLeft(coreKnobWidth).reduced(2));
    lowCutKnob.setBounds(coreRow.removeFromLeft(coreKnobWidth).reduced(2));
    mixKnob.setBounds(coreRow.removeFromLeft(coreKnobWidth).reduced(2));
    widthKnob.setBounds(coreRow.reduced(2));

    // Bottom row: Nebula panel (wide) on top, then Fairing + I/O below
    auto nebulaRow = bounds.removeFromTop(70).reduced(padding);
    nebulaPanel.setBounds(nebulaRow);

    auto bottomRow = bounds.removeFromTop(90).reduced(padding);

    // Fairing section (left half)
    auto fairingArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 2).reduced(padding);
    fairingLabel.setBounds(fairingArea.removeFromTop(20));

    auto fairingControlsArea = fairingArea;
    fairingButton.setBounds(fairingControlsArea.removeFromLeft(180).reduced(5, 10));

    auto syncArea = fairingControlsArea.removeFromLeft(100);
    fairingSyncLabel.setBounds(syncArea.removeFromTop(20));
    fairingSyncCombo.setBounds(syncArea.reduced(5, 5));

    // I/O section (right half)
    auto ioArea = bottomRow.reduced(padding);

    juce::Label* ioLabel = new juce::Label({}, "I/O");
    ioLabel->setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
    ioLabel->setColour(juce::Label::textColourId, Cosmos::CosmosLookAndFeel::Colors::textSecondary);
    ioLabel->setJustificationType(juce::Justification::centred);
    ioLabel->setBounds(ioArea.removeFromTop(20));
    addAndMakeVisible(ioLabel);

    int ioKnobWidth = ioArea.getWidth() / 2;
    inputGainKnob.setBounds(ioArea.removeFromLeft(ioKnobWidth).reduced(5));
    outputGainKnob.setBounds(ioArea.reduced(5));
}

void CosmosAudioProcessorEditor::timerCallback()
{
    // Update visualizers with processor data
    starfield.setDecayEnvelope(audioProcessor.getDecayEnvelope());
    starfield.setModulationChaos(chaosKnob.getSlider().getValue() / 100.0f);
    starfield.setFairingSeparationActive(audioProcessor.isFairingSeparationActive());
    starfield.setFairingSeparationIntensity(audioProcessor.getFairingSeparationIntensity());

    decayCurve.setDecayEnvelope(audioProcessor.getDecayEnvelope());
    decayCurve.setDecayTime(static_cast<float>(decayKnob.getSlider().getValue()));
}

void CosmosAudioProcessorEditor::applyNebulaPresetToUI(int presetIndex)
{
    const auto& preset = Cosmos::NebulaPresets::getPreset(presetIndex);
    auto& params = audioProcessor.getParameters();

    // Apply preset values to parameters (UI will update via attachments)
    if (auto* param = params.getParameter(Cosmos::ParamIDs::decay))
        param->setValueNotifyingHost(param->convertTo0to1(preset.decay));

    if (auto* param = params.getParameter(Cosmos::ParamIDs::preDelay))
        param->setValueNotifyingHost(param->convertTo0to1(preset.preDelay));

    if (auto* param = params.getParameter(Cosmos::ParamIDs::highCut))
        param->setValueNotifyingHost(param->convertTo0to1(preset.highCut));

    if (auto* param = params.getParameter(Cosmos::ParamIDs::lowCut))
        param->setValueNotifyingHost(param->convertTo0to1(preset.lowCut));

    if (auto* param = params.getParameter(Cosmos::ParamIDs::width))
        param->setValueNotifyingHost(param->convertTo0to1(preset.width));

    if (auto* param = params.getParameter(Cosmos::ParamIDs::diffusionThrust))
        param->setValueNotifyingHost(param->convertTo0to1(preset.diffusion));

    if (auto* param = params.getParameter(Cosmos::ParamIDs::modulationChaos))
        param->setValueNotifyingHost(param->convertTo0to1(preset.chaos));
}
