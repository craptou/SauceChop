#include "WaveformPeaks.h"

#include <algorithm>
#include <limits>

namespace saucechop
{
std::vector<WaveformPeak> createWaveformPeaks(const juce::AudioBuffer<float>& audio,
                                              const int maximumBinCount)
{
    const auto sampleCount = audio.getNumSamples();
    const auto channelCount = audio.getNumChannels();

    if (sampleCount <= 0 || channelCount <= 0 || maximumBinCount <= 0)
        return {};

    const auto binCount = std::min(sampleCount, maximumBinCount);
    std::vector<WaveformPeak> peaks;
    peaks.reserve(static_cast<std::size_t>(binCount));

    for (int bin = 0; bin < binCount; ++bin)
    {
        const auto startSample = static_cast<int>(
            (static_cast<std::int64_t>(bin) * sampleCount) / binCount);
        const auto endSample = static_cast<int>(
            (static_cast<std::int64_t>(bin + 1) * sampleCount) / binCount);

        auto minimum = std::numeric_limits<float>::max();
        auto maximum = std::numeric_limits<float>::lowest();

        for (int channel = 0; channel < channelCount; ++channel)
        {
            const auto* samples = audio.getReadPointer(channel);

            for (int sample = startSample; sample < endSample; ++sample)
            {
                minimum = std::min(minimum, samples[sample]);
                maximum = std::max(maximum, samples[sample]);
            }
        }

        peaks.push_back({minimum, maximum});
    }

    return peaks;
}
} // namespace saucechop
