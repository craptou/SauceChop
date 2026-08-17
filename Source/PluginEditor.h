#pragma once

#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

class SauceChopAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit SauceChopAudioProcessorEditor(SauceChopAudioProcessor& processor);
    ~SauceChopAudioProcessorEditor() override = default;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    SauceChopAudioProcessor& processor;

    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label outputGainLabel;
    juce::Slider outputGainSlider;
    juce::Rectangle<int> waveformBounds;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> outputGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SauceChopAudioProcessorEditor)
};
