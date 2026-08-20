#pragma once

#include "../files/SourceSample.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

class WaveformView final : public juce::Component
{
public:
    void setSample(std::shared_ptr<const saucechop::SourceSample> newSample);
    void setSliceCount(int newSliceCount);
    void setDropTargetActive(bool shouldBeActive);
    void setPlaybackPosition(float newPosition, bool isActive);

    void paint(juce::Graphics& graphics) override;

private:
    void drawPlaceholder(juce::Graphics& graphics, juce::Rectangle<float> bounds) const;
    void drawWaveform(juce::Graphics& graphics, juce::Rectangle<float> bounds) const;
    void drawSliceGrid(juce::Graphics& graphics, juce::Rectangle<float> bounds) const;
    void drawPlayhead(juce::Graphics& graphics, juce::Rectangle<float> bounds) const;

    std::shared_ptr<const saucechop::SourceSample> sample;
    int sliceCount = 16;
    bool dropTargetActive = false;
    float playbackPosition = 0.0f;
    bool playbackActive = false;
};
