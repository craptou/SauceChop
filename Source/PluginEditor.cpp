#include "PluginEditor.h"

#include <cmath>

namespace
{
const auto backgroundColour = juce::Colour::fromRGB(17, 19, 24);
const auto panelColour = juce::Colour::fromRGB(28, 32, 39);
const auto accentColour = juce::Colour::fromRGB(255, 116, 69);
const auto secondaryTextColour = juce::Colour::fromRGB(157, 164, 177);
} // namespace

SauceChopAudioProcessorEditor::SauceChopAudioProcessorEditor(
    SauceChopAudioProcessor& audioProcessor)
    : AudioProcessorEditor(&audioProcessor), processor(audioProcessor)
{
    titleLabel.setText("SauceChop", juce::dontSendNotification);
    titleLabel.setFont(juce::Font{juce::FontOptions{32.0f, juce::Font::bold}});
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    statusLabel.setText("VST3 shell ready  •  Sample engine comes next",
                        juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centredRight);
    statusLabel.setColour(juce::Label::textColourId, secondaryTextColour);
    addAndMakeVisible(statusLabel);

    outputGainLabel.setText("OUTPUT", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, secondaryTextColour);
    addAndMakeVisible(outputGainLabel);

    outputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 24);
    outputGainSlider.setTextValueSuffix(" dB");
    outputGainSlider.setColour(juce::Slider::rotarySliderFillColourId, accentColour);
    outputGainSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    outputGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    outputGainSlider.setColour(juce::Slider::textBoxOutlineColourId,
                               juce::Colours::transparentBlack);
    addAndMakeVisible(outputGainSlider);

    outputGainAttachment = std::make_unique<SliderAttachment>(
        processor.parameters(), "outputGain", outputGainSlider);

    setResizable(true, true);
    setResizeLimits(640, 420, 1400, 900);
    setSize(900, 560);
}

void SauceChopAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(backgroundColour);

    auto panel = waveformBounds.toFloat();
    graphics.setColour(panelColour);
    graphics.fillRoundedRectangle(panel, 12.0f);

    const auto centreY = panel.getCentreY();
    graphics.setColour(juce::Colours::white.withAlpha(0.08f));
    graphics.drawHorizontalLine(static_cast<int>(centreY), panel.getX(), panel.getRight());

    for (int slice = 1; slice < 16; ++slice)
    {
        const auto x = panel.getX() + panel.getWidth() * static_cast<float>(slice) / 16.0f;
        graphics.drawVerticalLine(static_cast<int>(x), panel.getY() + 12.0f,
                                  panel.getBottom() - 12.0f);
    }

    juce::Path placeholderWaveform;
    constexpr int pointCount = 160;

    for (int point = 0; point < pointCount; ++point)
    {
        const auto normalisedX = static_cast<float>(point) / static_cast<float>(pointCount - 1);
        const auto envelope = std::sin(juce::MathConstants<float>::pi * normalisedX);
        const auto signal = std::sin(normalisedX * juce::MathConstants<float>::twoPi * 7.0f)
            + 0.35f * std::sin(normalisedX * juce::MathConstants<float>::twoPi * 19.0f);
        const auto x = panel.getX() + normalisedX * panel.getWidth();
        const auto y = centreY - signal * envelope * panel.getHeight() * 0.24f;

        if (point == 0)
            placeholderWaveform.startNewSubPath(x, y);
        else
            placeholderWaveform.lineTo(x, y);
    }

    graphics.setColour(accentColour.withAlpha(0.85f));
    graphics.strokePath(placeholderWaveform, juce::PathStrokeType{2.0f});

    graphics.setColour(secondaryTextColour);
    graphics.setFont(15.0f);
    graphics.drawFittedText("Drop a WAV or AIFF here — available in milestone M2",
                            waveformBounds.reduced(24), juce::Justification::centred, 1);
}

void SauceChopAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(28);
    auto header = area.removeFromTop(48);

    titleLabel.setBounds(header.removeFromLeft(260));
    statusLabel.setBounds(header);

    area.removeFromTop(18);
    auto footer = area.removeFromBottom(145);
    area.removeFromBottom(18);
    waveformBounds = area;

    auto gainArea = footer.withSizeKeepingCentre(120, 140);
    outputGainLabel.setBounds(gainArea.removeFromTop(24));
    outputGainSlider.setBounds(gainArea);
}
