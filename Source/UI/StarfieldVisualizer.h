#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "CosmosLookAndFeel.h"
#include "BinaryData.h"
#include <array>
#include <random>

namespace Cosmos
{

//==============================================================================
/**
 * Animated Starfield Visualizer
 *
 * Displays an animated star field that reacts to reverb parameters:
 * - Star density/brightness responds to reverb decay envelope
 * - Star speed responds to modulation chaos
 * - Color shifts during fairing separation
 * - Nebula background images based on selected preset
 */
class StarfieldVisualizer : public juce::Component,
                            public juce::Timer
{
public:
    static constexpr int MaxStars = 100;

    //==========================================================================
    struct Star
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;       // Depth (for parallax)
        float speed = 1.0f;
        float brightness = 1.0f;
        float size = 1.0f;
    };

    //==========================================================================
    StarfieldVisualizer()
    {
        setOpaque(false);
        initializeStars();
        loadNebulaImages();
        startTimerHz(60);
    }

    ~StarfieldVisualizer() override
    {
        stopTimer();
    }

    //==========================================================================
    // Update visualizer state from processor data
    void setDecayEnvelope(float envelope)
    {
        decayEnvelope = juce::jlimit(0.0f, 1.0f, envelope);
    }

    void setModulationChaos(float chaos)
    {
        modulationChaos = juce::jlimit(0.0f, 1.0f, chaos);
    }

    void setFairingSeparationActive(bool active)
    {
        fairingActive = active;
    }

    void setFairingSeparationIntensity(float intensity)
    {
        fairingIntensity = juce::jlimit(0.0f, 1.0f, intensity);
    }

    void setNebulaIndex(int index)
    {
        if (index != currentNebulaIndex)
        {
            currentNebulaIndex = index;
            repaint();
        }
    }

    //==========================================================================
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Check if we have a nebula image to display (index > 0, as 0 is Manual)
        if (currentNebulaIndex > 0 && currentNebulaIndex <= static_cast<int>(nebulaImages.size()))
        {
            const auto& nebulaImage = nebulaImages[static_cast<size_t>(currentNebulaIndex - 1)];
            if (nebulaImage.isValid())
            {
                // Draw nebula image as background, scaled to fill
                g.drawImage(nebulaImage, bounds, juce::RectanglePlacement::centred | juce::RectanglePlacement::fillDestination);

                // Add a semi-transparent dark overlay for better UI readability
                g.setColour(juce::Colours::black.withAlpha(0.4f));
                g.fillRect(bounds);
            }
            else
            {
                // Fallback to gradient if image not loaded
                drawDefaultBackground(g, bounds);
            }
        }
        else
        {
            // Manual mode or no image - use default space gradient
            drawDefaultBackground(g, bounds);
        }

        // Draw stars
        float centerX = bounds.getCentreX();
        float centerY = bounds.getCentreY();

        for (const auto& star : stars)
        {
            // Project 3D position to 2D
            float scale = 1.0f / (star.z + 0.5f);
            float screenX = centerX + (star.x - 0.5f) * bounds.getWidth() * scale * 2.0f;
            float screenY = centerY + (star.y - 0.5f) * bounds.getHeight() * scale * 2.0f;

            // Skip stars outside bounds
            if (screenX < 0 || screenX > bounds.getWidth() ||
                screenY < 0 || screenY > bounds.getHeight())
                continue;

            // Calculate star properties
            float brightness = star.brightness * (1.0f - star.z) * (0.5f + decayEnvelope * 0.5f);
            float size = star.size * scale * (1.0f + decayEnvelope * 0.5f);

            // Color based on fairing state
            juce::Colour starColor = CosmosLookAndFeel::Colors::starWhite;
            if (fairingActive)
            {
                // Shift towards cyan during fairing separation
                starColor = starColor.interpolatedWith(
                    CosmosLookAndFeel::Colors::fairingCyan, fairingIntensity * 0.7f);
            }
            else if (modulationChaos > 0.5f)
            {
                // Subtle violet tint at high chaos
                starColor = starColor.interpolatedWith(
                    CosmosLookAndFeel::Colors::chaosViolet, (modulationChaos - 0.5f) * 0.3f);
            }

            // Draw star glow
            if (brightness > 0.3f && size > 1.5f)
            {
                g.setColour(starColor.withAlpha(brightness * 0.2f));
                g.fillEllipse(screenX - size * 2, screenY - size * 2, size * 4, size * 4);
            }

            // Draw star core
            g.setColour(starColor.withAlpha(brightness));
            g.fillEllipse(screenX - size / 2, screenY - size / 2, size, size);
        }

        // Draw subtle vignette
        juce::ColourGradient vignette(
            juce::Colours::transparentBlack, centerX, centerY,
            juce::Colours::black.withAlpha(0.4f), 0, 0, true);
        g.setGradientFill(vignette);
        g.fillRect(bounds);

        // Draw fairing separation flash effect
        if (fairingIntensity > 0.1f)
        {
            g.setColour(CosmosLookAndFeel::Colors::fairingCyan.withAlpha(fairingIntensity * 0.15f));
            g.fillRect(bounds);

            // Radial burst from center
            float burstRadius = bounds.getWidth() * fairingIntensity * 0.5f;
            juce::ColourGradient burst(
                CosmosLookAndFeel::Colors::fairingCyan.withAlpha(fairingIntensity * 0.3f),
                centerX, centerY,
                juce::Colours::transparentBlack,
                centerX + burstRadius, centerY, true);
            g.setGradientFill(burst);
            g.fillRect(bounds);
        }
    }

    //==========================================================================
    void timerCallback() override
    {
        updateStars();
        repaint();
    }

private:
    //==========================================================================
    void drawDefaultBackground(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        juce::ColourGradient bgGradient(
            CosmosLookAndFeel::Colors::deepSpace, bounds.getCentreX(), 0,
            CosmosLookAndFeel::Colors::darkBlue, bounds.getCentreX(), bounds.getHeight(),
            false);
        g.setGradientFill(bgGradient);
        g.fillRect(bounds);
    }

    void loadNebulaImages()
    {
        // Load nebula images in order matching preset indices 1-11
        // Index 0 is Manual (no image)
        nebulaImages.clear();

        // 1: Pillars of Creation
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_pillars_jpg, BinaryData::nebula_pillars_jpgSize));

        // 2: Crab Nebula
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_crab_jpg, BinaryData::nebula_crab_jpgSize));

        // 3: Orion Nebula
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_orion_jpg, BinaryData::nebula_orion_jpgSize));

        // 4: Helix Nebula
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_helix_jpg, BinaryData::nebula_helix_jpgSize));

        // 5: Horsehead Nebula
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_horsehead_jpg, BinaryData::nebula_horsehead_jpgSize));

        // 6: Ring Nebula
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_ring_jpg, BinaryData::nebula_ring_jpgSize));

        // 7: Carina Nebula
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_carina_jpg, BinaryData::nebula_carina_jpgSize));

        // 8: Lagoon Nebula
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_lagoon_jpg, BinaryData::nebula_lagoon_jpgSize));

        // 9: Veil Nebula
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_veil_jpg, BinaryData::nebula_veil_jpgSize));

        // 10: Cat's Eye Nebula
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_catseye_png, BinaryData::nebula_catseye_pngSize));

        // 11: Tarantula Nebula
        nebulaImages.push_back(juce::ImageCache::getFromMemory(
            BinaryData::nebula_tarantula_jpg, BinaryData::nebula_tarantula_jpgSize));
    }

    void initializeStars()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        for (auto& star : stars)
        {
            resetStar(star, dist(gen));
        }
    }

    void resetStar(Star& star, float initialZ = 0.0f)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        star.x = dist(gen);
        star.y = dist(gen);
        star.z = initialZ;
        star.speed = 0.002f + dist(gen) * 0.008f;
        star.brightness = 0.3f + dist(gen) * 0.7f;
        star.size = 1.0f + dist(gen) * 3.0f;
    }

    void updateStars()
    {
        // Speed modifier based on chaos and fairing
        float speedMod = 1.0f + modulationChaos * 2.0f;
        if (fairingActive)
            speedMod *= (1.0f + fairingIntensity * 3.0f);

        for (auto& star : stars)
        {
            // Move star towards viewer (decreasing z)
            star.z -= star.speed * speedMod * 0.016f; // Assuming ~60fps

            // Reset star when it passes the viewer
            if (star.z < 0.0f)
            {
                resetStar(star, 1.0f);
            }

            // Add slight wobble based on chaos
            if (modulationChaos > 0.3f)
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

                float wobble = (modulationChaos - 0.3f) * 0.001f;
                star.x += dist(gen) * wobble;
                star.y += dist(gen) * wobble;

                // Keep in bounds
                star.x = juce::jlimit(0.0f, 1.0f, star.x);
                star.y = juce::jlimit(0.0f, 1.0f, star.y);
            }
        }
    }

    //==========================================================================
    std::array<Star, MaxStars> stars;
    std::vector<juce::Image> nebulaImages;
    int currentNebulaIndex = 0;

    float decayEnvelope = 0.0f;
    float modulationChaos = 0.0f;
    bool fairingActive = false;
    float fairingIntensity = 0.0f;
};

} // namespace Cosmos
