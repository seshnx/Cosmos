#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "CosmosLookAndFeel.h"
#include <array>

namespace Cosmos
{

//==============================================================================
/**
 * Decay Curve Display
 *
 * Shows a visual representation of the reverb decay envelope
 * with animated glow effects.
 */
class DecayCurveDisplay : public juce::Component,
                          public juce::Timer
{
public:
    static constexpr int HistorySize = 128;

    //==========================================================================
    DecayCurveDisplay()
    {
        setOpaque(false);
        history.fill(0.0f);
        startTimerHz(30);
    }

    ~DecayCurveDisplay() override
    {
        stopTimer();
    }

    //==========================================================================
    void setDecayEnvelope(float envelope)
    {
        currentEnvelope = juce::jlimit(0.0f, 1.0f, envelope);
    }

    void setDecayTime(float seconds)
    {
        decayTimeSeconds = juce::jmax(0.1f, seconds);
    }

    //==========================================================================
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2);

        // Background
        g.setColour(CosmosLookAndFeel::Colors::dialBackground);
        g.fillRoundedRectangle(bounds, 4.0f);

        // Border
        g.setColour(CosmosLookAndFeel::Colors::dialRing);
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Draw grid lines
        g.setColour(CosmosLookAndFeel::Colors::textDim.withAlpha(0.2f));
        for (int i = 1; i < 4; ++i)
        {
            float x = bounds.getX() + bounds.getWidth() * i / 4.0f;
            g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
        }
        for (int i = 1; i < 3; ++i)
        {
            float y = bounds.getY() + bounds.getHeight() * i / 3.0f;
            g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
        }

        // Draw decay curve
        juce::Path curvePath;
        juce::Path glowPath;

        float graphWidth = bounds.getWidth() - 8;
        float graphHeight = bounds.getHeight() - 8;
        float graphX = bounds.getX() + 4;
        float graphY = bounds.getY() + 4;

        bool pathStarted = false;

        for (int i = 0; i < HistorySize; ++i)
        {
            float x = graphX + (static_cast<float>(i) / (HistorySize - 1)) * graphWidth;
            float normalizedValue = history[static_cast<size_t>(i)];
            float y = graphY + graphHeight * (1.0f - normalizedValue);

            if (!pathStarted)
            {
                curvePath.startNewSubPath(x, y);
                pathStarted = true;
            }
            else
            {
                curvePath.lineTo(x, y);
            }
        }

        // Create filled area
        glowPath = curvePath;
        glowPath.lineTo(graphX + graphWidth, graphY + graphHeight);
        glowPath.lineTo(graphX, graphY + graphHeight);
        glowPath.closeSubPath();

        // Draw glow fill
        juce::ColourGradient fillGradient(
            CosmosLookAndFeel::Colors::cosmicBlue.withAlpha(0.3f),
            0, graphY,
            CosmosLookAndFeel::Colors::cosmicBlue.withAlpha(0.0f),
            0, graphY + graphHeight,
            false);
        g.setGradientFill(fillGradient);
        g.fillPath(glowPath);

        // Draw curve line
        g.setColour(CosmosLookAndFeel::Colors::cosmicBlue);
        g.strokePath(curvePath, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

        // Draw glow on curve
        g.setColour(CosmosLookAndFeel::Colors::cosmicBlue.withAlpha(0.4f));
        g.strokePath(curvePath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

        // Draw current value indicator (bright dot at end)
        float lastX = graphX + graphWidth;
        float lastY = graphY + graphHeight * (1.0f - history[HistorySize - 1]);

        // Glow
        g.setColour(CosmosLookAndFeel::Colors::starWhite.withAlpha(0.3f));
        g.fillEllipse(lastX - 6, lastY - 6, 12, 12);

        // Core
        g.setColour(CosmosLookAndFeel::Colors::starWhite);
        g.fillEllipse(lastX - 3, lastY - 3, 6, 6);

        // Draw decay time label
        g.setColour(CosmosLookAndFeel::Colors::textSecondary);
        g.setFont(juce::Font(juce::FontOptions(11.0f)));

        juce::String timeText = juce::String(decayTimeSeconds, 1) + "s";
        g.drawText(timeText, bounds.removeFromBottom(16).removeFromRight(40),
                   juce::Justification::centredRight);

        g.drawText("DECAY", bounds.removeFromTop(16).removeFromLeft(60),
                   juce::Justification::centredLeft);
    }

    //==========================================================================
    void timerCallback() override
    {
        // Shift history
        for (int i = 0; i < HistorySize - 1; ++i)
        {
            history[static_cast<size_t>(i)] = history[static_cast<size_t>(i + 1)];
        }

        // Add current value
        history[HistorySize - 1] = currentEnvelope;

        repaint();
    }

private:
    std::array<float, HistorySize> history;
    float currentEnvelope = 0.0f;
    float decayTimeSeconds = 5.0f;
};

} // namespace Cosmos
