#pragma once

#include "../model/WaveformPeaks.h"

#include <juce_core/juce_core.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace saucechop
{
struct SourceSample
{
    juce::AudioBuffer<float> audio;
    std::vector<WaveformPeak> waveformPeaks;
    juce::File sourceFile;
    double originalSampleRateHz = 0.0;
    std::int64_t frameCount = 0;
    int channelCount = 0;
    std::int64_t fileSizeBytes = 0;
    juce::Time modificationTime;

    [[nodiscard]] double durationSeconds() const noexcept
    {
        return originalSampleRateHz > 0.0
            ? static_cast<double>(frameCount) / originalSampleRateHz
            : 0.0;
    }
};

struct SampleLoadResult
{
    std::shared_ptr<const SourceSample> sample;
    juce::String errorMessage;

    [[nodiscard]] bool succeeded() const noexcept
    {
        return sample != nullptr;
    }
};
} // namespace saucechop
