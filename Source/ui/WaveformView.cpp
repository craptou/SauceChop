#include "WaveformView.h"

#include <algorithm>

namespace
{
const auto panelColour = juce::Colour::fromRGB(28, 32, 39);
const auto borderColour = juce::Colour::fromRGB(61, 67, 78);
const auto accentColour = juce::Colour::fromRGB(255, 116, 69);
const auto secondaryTextColour = juce::Colour::fromRGB(157, 164, 177);
} // namespace

void WaveformView::setSample(std::shared_ptr<const saucechop::SourceSample> newSample)
{
    if (sample == newSample)
        return;

    sample = std::move(newSample);
    repaint();
}

void WaveformView::setSliceCount(const int newSliceCount)
{
    const auto sanitisedCount = juce::jlimit(1, 64, newSliceCount);

    if (sliceCount == sanitisedCount)
        return;

    sliceCount = sanitisedCount;
    repaint();
}

void WaveformView::setDropTargetActive(const bool shouldBeActive)
{
    if (dropTargetActive == shouldBeActive)
        return;

    dropTargetActive = shouldBeActive;
    repaint();
}

void WaveformView::paint(juce::Graphics& graphics)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    graphics.setColour(panelColour);
    graphics.fillRoundedRectangle(bounds, 12.0f);

    graphics.setColour(dropTargetActive ? accentColour : borderColour);
    graphics.drawRoundedRectangle(bounds, 12.0f, dropTargetActive ? 2.0f : 1.0f);

    bounds = bounds.reduced(18.0f);

    if (sample == nullptr || sample->waveformPeaks.empty())
        drawPlaceholder(graphics, bounds);
    else
        drawWaveform(graphics, bounds);

    drawSliceGrid(graphics, bounds);
}

void WaveformView::drawPlaceholder(juce::Graphics& graphics,
                                   const juce::Rectangle<float> bounds) const
{
    graphics.setColour(dropTargetActive ? juce::Colours::white : secondaryTextColour);
    graphics.setFont(16.0f);
    graphics.drawFittedText(dropTargetActive ? "Release to load this sample"
                                             : "Drop a WAV, AIFF or MP3 here",
                            bounds.toNearestInt(),
                            juce::Justification::centred,
                            1);
}

void WaveformView::drawWaveform(juce::Graphics& graphics,
                                const juce::Rectangle<float> bounds) const
{
    const auto centreY = bounds.getCentreY();
    const auto halfHeight = bounds.getHeight() * 0.45f;
    const auto width = std::max(1, static_cast<int>(bounds.getWidth()));
    const auto peakCount = static_cast<int>(sample->waveformPeaks.size());

    graphics.setColour(juce::Colours::white.withAlpha(0.08f));
    graphics.drawHorizontalLine(static_cast<int>(centreY), bounds.getX(), bounds.getRight());

    graphics.setColour(accentColour.withAlpha(0.92f));

    for (int x = 0; x < width; ++x)
    {
        const auto peakIndex = juce::jlimit(
            0, peakCount - 1, static_cast<int>((static_cast<std::int64_t>(x) * peakCount) / width));
        const auto& peak = sample->waveformPeaks[static_cast<std::size_t>(peakIndex)];
        const auto minimum = juce::jlimit(-1.0f, 1.0f, peak.minimum);
        const auto maximum = juce::jlimit(-1.0f, 1.0f, peak.maximum);
        const auto top = centreY - maximum * halfHeight;
        const auto bottom = centreY - minimum * halfHeight;
        const auto drawX = bounds.getX() + static_cast<float>(x);
        graphics.drawVerticalLine(static_cast<int>(drawX), top, bottom);
    }
}

void WaveformView::drawSliceGrid(juce::Graphics& graphics,
                                 const juce::Rectangle<float> bounds) const
{
    graphics.setColour(juce::Colours::white.withAlpha(sample != nullptr ? 0.16f : 0.07f));

    for (int slice = 1; slice < sliceCount; ++slice)
    {
        const auto x = bounds.getX()
            + bounds.getWidth() * static_cast<float>(slice) / static_cast<float>(sliceCount);
        graphics.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());
    }
}
