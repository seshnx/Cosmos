/*
  ==============================================================================
    Cosmos - Cinematic Space Reverb
    NebulaSelectorPopup - Custom popup for selecting nebula presets with descriptions
  ==============================================================================
*/

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Utils/Parameters.h"

namespace Cosmos
{

//==============================================================================
/**
 * A single nebula item in the popup list
 */
class NebulaListItem : public juce::Component
{
public:
    NebulaListItem(int index, const NebulaPresets::NebulaCharacter& nebula, bool isSelected)
        : nebulaIndex(index), character(nebula), selected(isSelected)
    {
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background
        if (isMouseOver)
        {
            g.setColour(juce::Colour(0xFF3A4A6A));
            g.fillRoundedRectangle(bounds.reduced(2.0f), 4.0f);
        }
        else if (selected)
        {
            g.setColour(juce::Colour(0xFF2A3A5A));
            g.fillRoundedRectangle(bounds.reduced(2.0f), 4.0f);
        }

        // Selection indicator
        if (selected)
        {
            g.setColour(juce::Colour(0xFF6699FF));
            g.fillRoundedRectangle(bounds.getX() + 4, bounds.getY() + 8, 3, bounds.getHeight() - 16, 1.5f);
        }

        // Nebula name
        g.setColour(selected ? juce::Colour(0xFF88BBFF) : juce::Colours::white);
        g.setFont(juce::FontOptions().withHeight(14.0f).withStyle("Bold"));
        g.drawText(character.name, bounds.reduced(14, 4).removeFromTop(18), juce::Justification::left);

        // Description
        g.setColour(juce::Colour(0xFFAABBCC));
        g.setFont(juce::FontOptions().withHeight(11.0f));
        auto descBounds = bounds.reduced(14, 4);
        descBounds.removeFromTop(18);
        g.drawText(character.description, descBounds, juce::Justification::left, true);
    }

    void mouseEnter(const juce::MouseEvent&) override
    {
        isMouseOver = true;
        repaint();
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        isMouseOver = false;
        repaint();
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (e.mouseWasClicked() && onClick)
            onClick(nebulaIndex);
    }

    std::function<void(int)> onClick;

private:
    int nebulaIndex;
    const NebulaPresets::NebulaCharacter& character;
    bool selected = false;
    bool isMouseOver = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NebulaListItem)
};

//==============================================================================
/**
 * Popup window containing the nebula list
 */
class NebulaSelectorPopup : public juce::Component
{
public:
    NebulaSelectorPopup(int currentSelection)
    {
        setOpaque(true);

        // Create list items for each nebula
        for (int i = 0; i < NebulaPresets::getNumPresets(); ++i)
        {
            auto item = std::make_unique<NebulaListItem>(i, NebulaPresets::getPreset(i), i == currentSelection);
            item->onClick = [this](int index)
            {
                if (onNebulaSelected)
                    onNebulaSelected(index);
            };
            addAndMakeVisible(*item);
            items.push_back(std::move(item));
        }

        // Calculate size
        int itemHeight = 50;
        int width = 380;
        int height = static_cast<int>(items.size()) * itemHeight + 20;
        setSize(width, height);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Dark space background
        g.setColour(juce::Colour(0xFF1A1A2E));
        g.fillRoundedRectangle(bounds, 8.0f);

        // Border
        g.setColour(juce::Colour(0xFF3A4A6A));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.5f);

        // Title
        g.setColour(juce::Colour(0xFF88AACC));
        g.setFont(juce::FontOptions().withHeight(12.0f).withStyle("Bold"));
        g.drawText("SELECT NEBULA", bounds.removeFromTop(24).reduced(10, 4), juce::Justification::centred);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromTop(24);  // Title area
        bounds.reduce(6, 6);

        int itemHeight = 50;
        for (auto& item : items)
        {
            item->setBounds(bounds.removeFromTop(itemHeight));
        }
    }

    std::function<void(int)> onNebulaSelected;

private:
    std::vector<std::unique_ptr<NebulaListItem>> items;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NebulaSelectorPopup)
};

//==============================================================================
/**
 * Button that displays current nebula and opens the selector popup
 */
class NebulaSelectorButton : public juce::Component
{
public:
    NebulaSelectorButton()
    {
        setOpaque(false);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background gradient
        juce::ColourGradient bgGrad(
            juce::Colour(0xFF2A3A5A), 0, 0,
            juce::Colour(0xFF1A2A4A), 0, bounds.getHeight(),
            false);
        g.setGradientFill(bgGrad);
        g.fillRoundedRectangle(bounds, 6.0f);

        // Border
        g.setColour(isMouseOver ? juce::Colour(0xFF6699FF) : juce::Colour(0xFF4A5A7A));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

        // Current nebula name
        const auto& nebula = NebulaPresets::getPreset(currentIndex);
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions().withHeight(13.0f).withStyle("Bold"));
        g.drawText(nebula.name, bounds.reduced(10, 2), juce::Justification::centredLeft);

        // Dropdown arrow
        g.setColour(juce::Colour(0xFF88AACC));
        juce::Path arrow;
        float arrowX = bounds.getRight() - 18;
        float arrowY = bounds.getCentreY() - 3;
        arrow.addTriangle(arrowX, arrowY, arrowX + 10, arrowY, arrowX + 5, arrowY + 6);
        g.fillPath(arrow);
    }

    void mouseEnter(const juce::MouseEvent&) override
    {
        isMouseOver = true;
        repaint();
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        isMouseOver = false;
        repaint();
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (e.mouseWasClicked())
            showPopup();
    }

    void showPopup()
    {
        auto popup = std::make_unique<NebulaSelectorPopup>(currentIndex);

        popup->onNebulaSelected = [this](int index)
        {
            currentIndex = index;
            if (onNebulaChanged)
                onNebulaChanged(index);
            repaint();

            // Close the popup
            if (auto* callout = juce::Component::getCurrentlyModalComponent())
                callout->exitModalState(0);
        };

        auto& cb = juce::CallOutBox::launchAsynchronously(
            std::move(popup),
            getScreenBounds(),
            nullptr);

        cb.setArrowSize(10);
    }

    void setCurrentIndex(int index)
    {
        if (index != currentIndex)
        {
            currentIndex = index;
            repaint();
        }
    }

    int getCurrentIndex() const { return currentIndex; }

    std::function<void(int)> onNebulaChanged;

private:
    int currentIndex = 0;
    bool isMouseOver = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NebulaSelectorButton)
};

} // namespace Cosmos
