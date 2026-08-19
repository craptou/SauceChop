#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <vector>

namespace saucechop
{
struct WaveformPeak
{
    float minimum = 0.0f;
    float maximum = 0.0f;
};

[[nodiscard]] std::vector<WaveformPeak>
createWaveformPeaks(const juce::AudioBuffer<float>& audio, int maximumBinCount);
} // namespace saucechop
