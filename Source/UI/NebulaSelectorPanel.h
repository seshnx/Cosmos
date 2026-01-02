/*
  ==============================================================================
    Cosmos - Cinematic Space Reverb
    NebulaSelectorPanel - Horizontal panel with dropdown and description
    (Nebula images are displayed as plugin background)
  ==============================================================================
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Utils/Parameters.h"

namespace Cosmos
{

//==============================================================================
/**
 * Horizontal nebula selector panel with:
 * - Dropdown combo box for selection
 * - Description text area
 * (Images displayed as plugin background via StarfieldVisualizer)
 */
class NebulaSelectorPanel : public juce::Component
{
public:
    NebulaSelectorPanel()
    {
        // Setup combo box
        for (int i = 0; i < NebulaPresets::getNumPresets(); ++i)
        {
            nebulaCombo.addItem(NebulaPresets::getPreset(i).name, i + 1);
        }
        nebulaCombo.setSelectedId(1);
        nebulaCombo.onChange = [this]()
        {
            updateDisplay();
            if (onNebulaChanged)
                onNebulaChanged(nebulaCombo.getSelectedId() - 1);
        };
        addAndMakeVisible(nebulaCombo);

        // Setup description label
        descriptionLabel.setFont(juce::FontOptions().withHeight(12.0f));
        descriptionLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFCCDDEE));
        descriptionLabel.setJustificationType(juce::Justification::centredLeft);
        descriptionLabel.setMinimumHorizontalScale(1.0f);
        addAndMakeVisible(descriptionLabel);

        // Initial update
        updateDisplay();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Semi-transparent background
        g.setColour(juce::Colour(0xFF0A0A1A).withAlpha(0.85f));
        g.fillRoundedRectangle(bounds, 6.0f);

        // Border
        g.setColour(juce::Colour(0xFF4A6A9A).withAlpha(0.6f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(10, 6);

        // Left section: Dropdown (fixed width)
        auto dropdownArea = bounds.removeFromLeft(160);
        nebulaCombo.setBounds(dropdownArea.reduced(0, (dropdownArea.getHeight() - 26) / 2));

        bounds.removeFromLeft(15); // Spacing

        // Right section: Description (remaining space)
        descriptionLabel.setBounds(bounds);
    }

    void setCurrentIndex(int index)
    {
        if (index >= 0 && index < NebulaPresets::getNumPresets())
        {
            nebulaCombo.setSelectedId(index + 1, juce::dontSendNotification);
            updateDisplay();
        }
    }

    int getCurrentIndex() const
    {
        return nebulaCombo.getSelectedId() - 1;
    }

    juce::ComboBox& getComboBox() { return nebulaCombo; }

    std::function<void(int)> onNebulaChanged;

private:
    juce::ComboBox nebulaCombo;
    juce::Label descriptionLabel;

    void updateDisplay()
    {
        int index = nebulaCombo.getSelectedId() - 1;
        if (index >= 0 && index < NebulaPresets::getNumPresets())
        {
            const auto& preset = NebulaPresets::getPreset(index);
            descriptionLabel.setText(preset.description, juce::dontSendNotification);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NebulaSelectorPanel)
};

} // namespace Cosmos
