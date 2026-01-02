#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "CosmosLookAndFeel.h"

namespace Cosmos
{

//==============================================================================
/**
 * Engine Knob - Stylized rotary control with rocket engine dial aesthetic
 *
 * Features:
 * - Glowing value arc
 * - Animated glow pulse
 * - Label with value display
 */
class EngineKnob : public juce::Component
{
public:
    enum class Style
    {
        Standard,   // Blue cosmic glow
        Thrust,     // Orange thrust glow
        Chaos,      // Violet modulation glow
        Fairing     // Cyan fairing glow
    };

    //==========================================================================
    EngineKnob(const juce::String& labelText, Style knobStyle = Style::Standard)
        : label(labelText), style(knobStyle)
    {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setName(labelText);
        addAndMakeVisible(slider);

        valueLabel.setJustificationType(juce::Justification::centred);
        valueLabel.setColour(juce::Label::textColourId, CosmosLookAndFeel::Colors::textPrimary);
        addAndMakeVisible(valueLabel);

        nameLabel.setText(labelText, juce::dontSendNotification);
        nameLabel.setJustificationType(juce::Justification::centred);
        nameLabel.setColour(juce::Label::textColourId, CosmosLookAndFeel::Colors::textSecondary);
        addAndMakeVisible(nameLabel);

        slider.onValueChange = [this]() { updateValueLabel(); };
    }

    //==========================================================================
    juce::Slider& getSlider() { return slider; }

    void setValueSuffix(const juce::String& suffix)
    {
        valueSuffix = suffix;
        updateValueLabel();
    }

    void setValuePrecision(int decimalPlaces)
    {
        precision = decimalPlaces;
        updateValueLabel();
    }

    //==========================================================================
    void resized() override
    {
        auto bounds = getLocalBounds();

        // Name label at top
        nameLabel.setBounds(bounds.removeFromTop(20));

        // Value label at bottom
        valueLabel.setBounds(bounds.removeFromBottom(20));

        // Slider takes remaining space
        slider.setBounds(bounds.reduced(4));

        updateValueLabel();
    }

    void paint(juce::Graphics& g) override
    {
        // Additional glow effect behind knob based on style
        auto sliderBounds = slider.getBounds().toFloat();
        auto centerX = sliderBounds.getCentreX();
        auto centerY = sliderBounds.getCentreY();
        auto radius = juce::jmin(sliderBounds.getWidth(), sliderBounds.getHeight()) / 2.0f;

        juce::Colour glowColor;
        switch (style)
        {
            case Style::Thrust:  glowColor = CosmosLookAndFeel::Colors::thrustOrange; break;
            case Style::Chaos:   glowColor = CosmosLookAndFeel::Colors::chaosViolet; break;
            case Style::Fairing: glowColor = CosmosLookAndFeel::Colors::fairingCyan; break;
            default:             glowColor = CosmosLookAndFeel::Colors::cosmicBlue; break;
        }

        // Outer glow (subtle pulse based on value)
        float normalizedValue = static_cast<float>(slider.getValue() - slider.getMinimum()) /
                               static_cast<float>(slider.getMaximum() - slider.getMinimum());

        float glowAlpha = 0.05f + normalizedValue * 0.1f;

        juce::ColourGradient glow(
            glowColor.withAlpha(glowAlpha), centerX, centerY,
            juce::Colours::transparentBlack, centerX + radius * 1.5f, centerY, true);

        g.setGradientFill(glow);
        g.fillEllipse(centerX - radius * 1.5f, centerY - radius * 1.5f,
                      radius * 3.0f, radius * 3.0f);
    }

private:
    void updateValueLabel()
    {
        juce::String valueText;

        if (precision == 0)
        {
            valueText = juce::String(static_cast<int>(slider.getValue()));
        }
        else
        {
            valueText = juce::String(slider.getValue(), precision);
        }

        valueText += valueSuffix;
        valueLabel.setText(valueText, juce::dontSendNotification);
    }

    juce::Slider slider;
    juce::Label valueLabel;
    juce::Label nameLabel;

    juce::String label;
    juce::String valueSuffix;
    int precision = 1;
    Style style;
};

} // namespace Cosmos
