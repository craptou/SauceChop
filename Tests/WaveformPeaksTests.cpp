#include "../Source/model/WaveformPeaks.h"

#include <juce_core/juce_core.h>

#include <algorithm>
#include <array>

namespace
{
class WaveformPeaksTests final : public juce::UnitTest
{
public:
    WaveformPeaksTests()
        : UnitTest("Waveform peak cache", "model")
    {
    }

    void runTest() override
    {
        beginTest("Empty inputs produce no peaks");
        juce::AudioBuffer<float> empty;
        expect(saucechop::createWaveformPeaks(empty, 64).empty());

        juce::AudioBuffer<float> audio{1, 8};
        const std::array values{-1.0f, -0.5f, 0.0f, 0.5f, 1.0f, 0.25f, -0.25f, 0.0f};
        std::copy(values.begin(), values.end(), audio.getWritePointer(0));

        beginTest("Mono samples are reduced into exact min/max bins");
        const auto peaks = saucechop::createWaveformPeaks(audio, 4);
        expectEquals(static_cast<int>(peaks.size()), 4);

        if (peaks.size() == 4)
        {
            expectWithinAbsoluteError(peaks[0].minimum, -1.0f, 0.0001f);
            expectWithinAbsoluteError(peaks[0].maximum, -0.5f, 0.0001f);
            expectWithinAbsoluteError(peaks[1].minimum, 0.0f, 0.0001f);
            expectWithinAbsoluteError(peaks[1].maximum, 0.5f, 0.0001f);
            expectWithinAbsoluteError(peaks[2].minimum, 0.25f, 0.0001f);
            expectWithinAbsoluteError(peaks[2].maximum, 1.0f, 0.0001f);
            expectWithinAbsoluteError(peaks[3].minimum, -0.25f, 0.0001f);
            expectWithinAbsoluteError(peaks[3].maximum, 0.0f, 0.0001f);
        }

        beginTest("Stereo channels are merged for display");
        juce::AudioBuffer<float> stereo{2, 2};
        stereo.setSample(0, 0, 0.2f);
        stereo.setSample(0, 1, 0.3f);
        stereo.setSample(1, 0, -0.7f);
        stereo.setSample(1, 1, 0.8f);
        const auto stereoPeaks = saucechop::createWaveformPeaks(stereo, 1);

        expectEquals(static_cast<int>(stereoPeaks.size()), 1);

        if (!stereoPeaks.empty())
        {
            expectWithinAbsoluteError(stereoPeaks[0].minimum, -0.7f, 0.0001f);
            expectWithinAbsoluteError(stereoPeaks[0].maximum, 0.8f, 0.0001f);
        }
    }
};

WaveformPeaksTests waveformPeaksTests;
} // namespace
