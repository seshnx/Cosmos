#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace Cosmos
{

//==============================================================================
/**
 * Space-themed Look and Feel for SeshNx Cosmos
 *
 * Theme: Deep space, black/blue gradients, glowing readouts
 * Inspired by: Rocket engine dials, spacecraft HUDs
 */
class CosmosLookAndFeel : public juce::LookAndFeel_V4
{
public:
    //==========================================================================
    // Color Palette
    //==========================================================================
    struct Colors
    {
        // Background gradients
        static inline const juce::Colour deepSpace { 0xff0a0a12 };
        static inline const juce::Colour darkBlue { 0xff0d1a2d };
        static inline const juce::Colour midBlue { 0xff1a3050 };

        // Accent colors (glow effects)
        static inline const juce::Colour cosmicBlue { 0xff4a9eff };
        static inline const juce::Colour nebulaBlue { 0xff2d7dd2 };
        static inline const juce::Colour starWhite { 0xffeef4ff };
        static inline const juce::Colour thrustOrange { 0xffff6b35 };
        static inline const juce::Colour chaosViolet { 0xff9b59b6 };
        static inline const juce::Colour fairingCyan { 0xff00d4aa };

        // UI elements
        static inline const juce::Colour dialBackground { 0xff1a1a28 };
        static inline const juce::Colour dialRing { 0xff2a3a50 };
        static inline const juce::Colour textPrimary { 0xffeef4ff };
        static inline const juce::Colour textSecondary { 0xff8899aa };
        static inline const juce::Colour textDim { 0xff556677 };

        // Meters
        static inline const juce::Colour meterGreen { 0xff00cc66 };
        static inline const juce::Colour meterYellow { 0xffffcc00 };
        static inline const juce::Colour meterRed { 0xffff3366 };
    };

    //==========================================================================
    CosmosLookAndFeel()
    {
        // Set default colors
        setColour(juce::Slider::backgroundColourId, Colors::dialBackground);
        setColour(juce::Slider::thumbColourId, Colors::cosmicBlue);
        setColour(juce::Slider::trackColourId, Colors::nebulaBlue);
        setColour(juce::Slider::rotarySliderFillColourId, Colors::cosmicBlue);
        setColour(juce::Slider::rotarySliderOutlineColourId, Colors::dialRing);

        setColour(juce::Label::textColourId, Colors::textPrimary);

        setColour(juce::TextButton::buttonColourId, Colors::dialBackground);
        setColour(juce::TextButton::buttonOnColourId, Colors::nebulaBlue);
        setColour(juce::TextButton::textColourOffId, Colors::textSecondary);
        setColour(juce::TextButton::textColourOnId, Colors::textPrimary);

        setColour(juce::ComboBox::backgroundColourId, Colors::dialBackground);
        setColour(juce::ComboBox::textColourId, Colors::textPrimary);
        setColour(juce::ComboBox::outlineColourId, Colors::dialRing);
        setColour(juce::ComboBox::arrowColourId, Colors::cosmicBlue);

        setColour(juce::PopupMenu::backgroundColourId, Colors::darkBlue);
        setColour(juce::PopupMenu::textColourId, Colors::textPrimary);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, Colors::nebulaBlue);
        setColour(juce::PopupMenu::highlightedTextColourId, Colors::starWhite);

        setColour(juce::ToggleButton::textColourId, Colors::textPrimary);
        setColour(juce::ToggleButton::tickColourId, Colors::fairingCyan);
        setColour(juce::ToggleButton::tickDisabledColourId, Colors::textDim);
    }

    //==========================================================================
    // Rotary Slider (Engine Dial Style)
    //==========================================================================
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centreX = bounds.getCentreX();
        auto centreY = bounds.getCentreY();
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Determine accent color based on slider name
        juce::Colour accentColor = Colors::cosmicBlue;
        juce::String name = slider.getName().toLowerCase();

        if (name.contains("thrust") || name.contains("diffusion"))
            accentColor = Colors::thrustOrange;
        else if (name.contains("chaos") || name.contains("modulation"))
            accentColor = Colors::chaosViolet;
        else if (name.contains("fairing"))
            accentColor = Colors::fairingCyan;

        // Outer glow
        g.setColour(accentColor.withAlpha(0.15f));
        g.fillEllipse(rx - 4, ry - 4, rw + 8, rw + 8);

        // Background circle
        g.setColour(Colors::dialBackground);
        g.fillEllipse(rx, ry, rw, rw);

        // Outer ring
        g.setColour(Colors::dialRing);
        g.drawEllipse(rx, ry, rw, rw, 2.0f);

        // Value arc
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY,
                               radius - 6.0f, radius - 6.0f,
                               0.0f,
                               rotaryStartAngle,
                               angle,
                               true);

        g.setColour(accentColor);
        g.strokePath(valueArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Glow effect on arc
        g.setColour(accentColor.withAlpha(0.3f));
        g.strokePath(valueArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        // Pointer/needle
        juce::Path pointer;
        auto pointerLength = radius * 0.6f;
        auto pointerThickness = 3.0f;

        pointer.addRectangle(-pointerThickness / 2, -pointerLength, pointerThickness, pointerLength);

        g.setColour(Colors::starWhite);
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        // Center dot
        auto dotRadius = radius * 0.15f;
        g.setColour(Colors::dialBackground);
        g.fillEllipse(centreX - dotRadius, centreY - dotRadius, dotRadius * 2, dotRadius * 2);
        g.setColour(accentColor);
        g.drawEllipse(centreX - dotRadius, centreY - dotRadius, dotRadius * 2, dotRadius * 2, 1.5f);

        // Tick marks
        g.setColour(Colors::textDim);
        int numTicks = 11;
        for (int i = 0; i < numTicks; ++i)
        {
            float tickAngle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * i / (numTicks - 1);
            float tickInnerRadius = radius - 2.0f;
            float tickOuterRadius = radius + 2.0f;

            float x1 = centreX + tickInnerRadius * std::sin(tickAngle);
            float y1 = centreY - tickInnerRadius * std::cos(tickAngle);
            float x2 = centreX + tickOuterRadius * std::sin(tickAngle);
            float y2 = centreY - tickOuterRadius * std::cos(tickAngle);

            g.drawLine(x1, y1, x2, y2, 1.0f);
        }
    }

    //==========================================================================
    // Linear Slider (Vertical meter style)
    //==========================================================================
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        juce::ignoreUnused(minSliderPos, maxSliderPos);

        if (style == juce::Slider::LinearVertical)
        {
            auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();

            // Track background
            auto trackWidth = 8.0f;
            auto trackX = bounds.getCentreX() - trackWidth / 2;

            g.setColour(Colors::dialBackground);
            g.fillRoundedRectangle(trackX, bounds.getY(), trackWidth, bounds.getHeight(), 4.0f);

            // Value fill
            float fillHeight = bounds.getBottom() - sliderPos;
            g.setColour(Colors::cosmicBlue);
            g.fillRoundedRectangle(trackX, sliderPos, trackWidth, fillHeight, 4.0f);

            // Glow
            g.setColour(Colors::cosmicBlue.withAlpha(0.3f));
            g.fillRoundedRectangle(trackX - 2, sliderPos, trackWidth + 4, fillHeight, 6.0f);
        }
        else
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos,
                                              minSliderPos, maxSliderPos, style, slider);
        }
    }

    //==========================================================================
    // Button styling
    //==========================================================================
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        auto cornerSize = 6.0f;

        juce::Colour baseColor = backgroundColour;
        juce::Colour glowColor = Colors::cosmicBlue;

        // Check for fairing button
        if (button.getName().toLowerCase().contains("fairing"))
        {
            glowColor = Colors::fairingCyan;
        }

        if (button.getToggleState())
        {
            baseColor = glowColor.withAlpha(0.3f);

            // Active glow
            g.setColour(glowColor.withAlpha(0.2f));
            g.fillRoundedRectangle(bounds.expanded(2), cornerSize + 2);
        }
        else if (shouldDrawButtonAsHighlighted)
        {
            baseColor = baseColor.brighter(0.1f);
        }

        if (shouldDrawButtonAsDown)
        {
            baseColor = baseColor.brighter(0.2f);
        }

        g.setColour(baseColor);
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(button.getToggleState() ? glowColor : Colors::dialRing);
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    }

    //==========================================================================
    // ComboBox styling
    //==========================================================================
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override
    {
        juce::ignoreUnused(buttonX, buttonY, buttonW, buttonH, isButtonDown);

        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);
        auto cornerSize = 4.0f;

        g.setColour(Colors::dialBackground);
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(Colors::dialRing);
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);

        // Arrow
        auto arrowZone = juce::Rectangle<int>(width - 20, 0, 20, height).toFloat();
        juce::Path arrow;
        arrow.addTriangle(arrowZone.getCentreX() - 4, arrowZone.getCentreY() - 2,
                          arrowZone.getCentreX() + 4, arrowZone.getCentreY() - 2,
                          arrowZone.getCentreX(), arrowZone.getCentreY() + 4);

        g.setColour(Colors::cosmicBlue);
        g.fillPath(arrow);
    }

    //==========================================================================
    // Label styling
    //==========================================================================
    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        g.setColour(label.findColour(juce::Label::textColourId));

        auto font = getLabelFont(label);
        g.setFont(font);

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         juce::jmax(1, (int)(textArea.getHeight() / font.getHeight())),
                         label.getMinimumHorizontalScale());
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(juce::FontOptions(14.0f));
    }

    //==========================================================================
    // Fonts
    //==========================================================================
    juce::Font getTextButtonFont(juce::TextButton&, int) override
    {
        return juce::Font(juce::FontOptions(13.0f));
    }

    juce::Font getComboBoxFont(juce::ComboBox&) override
    {
        return juce::Font(juce::FontOptions(13.0f));
    }

    juce::Font getSliderPopupFont(juce::Slider&) override
    {
        return juce::Font(juce::FontOptions(12.0f));
    }
};

} // namespace Cosmos
