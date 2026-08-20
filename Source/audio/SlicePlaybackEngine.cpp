#include "SlicePlaybackEngine.h"

#include <algorithm>
#include <cmath>

namespace saucechop
{
namespace
{
constexpr double playbackFadeSeconds = 0.005;
constexpr double gainSmoothingSeconds = 0.02;
}

void SlicePlaybackEngine::prepare(const double newHostSampleRate,
                                  const int maximumExpectedSamplesPerBlock) noexcept
{
    juce::ignoreUnused(maximumExpectedSamplesPerBlock);
    hostSampleRate = newHostSampleRate > 0.0 ? newHostSampleRate : 44100.0;
    playbackEnvelope.reset(hostSampleRate, playbackFadeSeconds);
    playbackEnvelope.setCurrentAndTargetValue(0.0f);
    outputGain.reset(hostSampleRate, gainSmoothingSeconds);
    outputGain.setCurrentAndTargetValue(1.0f);
    stopImmediately();
}

void SlicePlaybackEngine::setSource(const SourceSample* const newSource) noexcept
{
    if (source == newSource)
        return;

    source = newSource;
    stopImmediately();
}

void SlicePlaybackEngine::setSliceCount(const int newSliceCount) noexcept
{
    sliceCount = juce::jlimit(1, 64, newSliceCount);

    if (source != nullptr)
        activeSlice = sliceForPosition(sourcePosition, source->audio.getNumSamples());
}

void SlicePlaybackEngine::start() noexcept
{
    if (source == nullptr || source->audio.getNumSamples() <= 0
        || source->originalSampleRateHz <= 0.0)
    {
        stopImmediately();
        return;
    }

    sourcePosition = 0.0;
    playbackProgress = 0.0;
    activeSlice = 0;
    playing = true;
    stopping = false;
    playbackEnvelope.setCurrentAndTargetValue(0.0f);
    playbackEnvelope.setTargetValue(1.0f);
}

void SlicePlaybackEngine::requestStop() noexcept
{
    if (!playing)
        return;

    stopping = true;
    playbackEnvelope.setTargetValue(0.0f);
}

void SlicePlaybackEngine::stopImmediately() noexcept
{
    sourcePosition = 0.0;
    playbackProgress = 0.0;
    activeSlice = -1;
    playing = false;
    stopping = false;
    playbackEnvelope.setCurrentAndTargetValue(0.0f);
}

void SlicePlaybackEngine::process(juce::AudioBuffer<float>& output,
                                  const float outputGainLinear) noexcept
{
    output.clear();
    outputGain.setTargetValue(juce::jmax(0.0f, outputGainLinear));

    if (!playing || source == nullptr)
        return;

    const auto totalFrames = source->audio.getNumSamples();
    const auto sourceChannels = source->audio.getNumChannels();
    const auto outputChannels = output.getNumChannels();

    if (totalFrames <= 0 || sourceChannels <= 0 || outputChannels <= 0
        || source->originalSampleRateHz <= 0.0)
    {
        stopImmediately();
        return;
    }

    const auto sourceFramesPerOutputFrame = source->originalSampleRateHz / hostSampleRate;
    const auto tailFadeFrames = juce::jmax(1.0, source->originalSampleRateHz * playbackFadeSeconds);

    for (int outputFrame = 0; outputFrame < output.getNumSamples(); ++outputFrame)
    {
        if (sourcePosition >= static_cast<double>(totalFrames))
        {
            playing = false;
            stopping = false;
            activeSlice = sliceCount - 1;
            playbackProgress = 1.0;
            playbackEnvelope.setCurrentAndTargetValue(0.0f);
            break;
        }

        const auto firstFrame = juce::jlimit(0, totalFrames - 1,
                                             static_cast<int>(sourcePosition));
        const auto secondFrame = juce::jmin(firstFrame + 1, totalFrames - 1);
        const auto fraction = static_cast<float>(sourcePosition - firstFrame);
        const auto framesRemaining = static_cast<double>(totalFrames) - sourcePosition;
        const auto naturalTailGain = static_cast<float>(
            juce::jlimit(0.0, 1.0, framesRemaining / tailFadeFrames));
        const auto gain = playbackEnvelope.getNextValue() * outputGain.getNextValue()
            * naturalTailGain;

        for (int outputChannel = 0; outputChannel < outputChannels; ++outputChannel)
        {
            const auto sourceChannel = juce::jmin(outputChannel, sourceChannels - 1);
            const auto* sourceData = source->audio.getReadPointer(sourceChannel);
            const auto firstValue = sourceData[firstFrame];
            const auto sampleValue = firstValue
                + fraction * (sourceData[secondFrame] - firstValue);
            output.setSample(outputChannel, outputFrame, sampleValue * gain);
        }

        sourcePosition += sourceFramesPerOutputFrame;
        playbackProgress = juce::jlimit(
            0.0, 1.0, sourcePosition / static_cast<double>(totalFrames));
        activeSlice = sliceForPosition(sourcePosition, totalFrames);

        if (stopping && !playbackEnvelope.isSmoothing()
            && playbackEnvelope.getCurrentValue() <= 0.0f)
        {
            playing = false;
            stopping = false;
            activeSlice = -1;
            break;
        }
    }
}

int SlicePlaybackEngine::sliceForPosition(const double position,
                                          const int totalFrames) const noexcept
{
    if (totalFrames <= 0)
        return -1;

    const auto frame = static_cast<std::int64_t>(juce::jlimit(
        0.0, static_cast<double>(totalFrames - 1), std::floor(position)));
    const auto numerator = (frame + 1) * static_cast<std::int64_t>(sliceCount) - 1;
    return juce::jlimit(0,
                        sliceCount - 1,
                        static_cast<int>(numerator / totalFrames));
}
} // namespace saucechop
