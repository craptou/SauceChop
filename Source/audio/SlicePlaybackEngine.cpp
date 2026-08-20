#include "SlicePlaybackEngine.h"

#include <algorithm>

namespace saucechop
{
namespace
{
constexpr double playbackFadeSeconds = 0.005;
constexpr double sliceFadeSeconds = 0.003;
constexpr double gainSmoothingSeconds = 0.02;
}

SlicePlaybackEngine::SlicePlaybackEngine() noexcept
{
    resetIdentityOrder();
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
    const auto sanitisedCount = juce::jlimit(1, static_cast<int>(sequenceOrder.size()),
                                             newSliceCount);

    if (sliceCount == sanitisedCount)
        return;

    sliceCount = sanitisedCount;
    resetIdentityOrder();
    stopImmediately();
}

void SlicePlaybackEngine::setSequenceOrder(const int* const newOrder,
                                           const int orderSize) noexcept
{
    if (newOrder == nullptr || orderSize != sliceCount)
        return;

    std::array<bool, 32> found{};

    for (int index = 0; index < orderSize; ++index)
    {
        const auto slice = newOrder[index];

        if (slice < 0 || slice >= sliceCount || found[static_cast<std::size_t>(slice)])
            return;

        found[static_cast<std::size_t>(slice)] = true;
    }

    for (int index = 0; index < orderSize; ++index)
        sequenceOrder[static_cast<std::size_t>(index)] = newOrder[index];

    if (playing)
    {
        const auto* begin = sequenceOrder.data();
        const auto* end = begin + sliceCount;
        const auto* match = std::find(begin, end, activeSlice);
        sequenceStep = match != end ? static_cast<int>(std::distance(begin, match)) : 0;
    }
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
    sequenceProgress = 0.0;
    sequenceStep = 0;
    activeSlice = sequenceOrder.front();
    sourcePosition = sliceStartFrame(activeSlice, source->audio.getNumSamples());
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
    sequenceProgress = 0.0;
    activeSlice = -1;
    sequenceStep = -1;
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
    const auto sliceFadeFrames = juce::jmax(1.0, source->originalSampleRateHz * sliceFadeSeconds);

    for (int outputFrame = 0; outputFrame < output.getNumSamples(); ++outputFrame)
    {
        while (sequenceStep >= 0 && sequenceStep < sliceCount)
        {
            activeSlice = sequenceOrder[static_cast<std::size_t>(sequenceStep)];
            const auto startFrame = sliceStartFrame(activeSlice, totalFrames);
            const auto endFrame = sliceEndFrame(activeSlice, totalFrames);

            if (endFrame > startFrame && sourcePosition < static_cast<double>(endFrame))
                break;

            const auto overshoot = endFrame > startFrame
                ? juce::jmax(0.0, sourcePosition - static_cast<double>(endFrame))
                : 0.0;
            ++sequenceStep;

            if (sequenceStep < sliceCount)
            {
                activeSlice = sequenceOrder[static_cast<std::size_t>(sequenceStep)];
                sourcePosition = sliceStartFrame(activeSlice, totalFrames) + overshoot;
            }
        }

        if (sequenceStep < 0 || sequenceStep >= sliceCount)
        {
            playing = false;
            stopping = false;
            sequenceProgress = 1.0;
            playbackEnvelope.setCurrentAndTargetValue(0.0f);
            break;
        }

        const auto sliceStart = sliceStartFrame(activeSlice, totalFrames);
        const auto sliceEnd = sliceEndFrame(activeSlice, totalFrames);

        const auto firstFrame = juce::jlimit(0, totalFrames - 1,
                                             static_cast<int>(sourcePosition));
        const auto secondFrame = juce::jmin(firstFrame + 1, sliceEnd - 1);
        const auto fraction = static_cast<float>(sourcePosition - firstFrame);
        const auto framesFromStart = sourcePosition - static_cast<double>(sliceStart);
        const auto framesRemaining = static_cast<double>(sliceEnd) - sourcePosition;
        const auto sliceGain = static_cast<float>(juce::jlimit(
            0.0, 1.0, std::min(framesFromStart, framesRemaining) / sliceFadeFrames));
        const auto gain = playbackEnvelope.getNextValue() * outputGain.getNextValue()
            * sliceGain;

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
        const auto localSliceProgress = juce::jlimit(
            0.0,
            1.0,
            (sourcePosition - sliceStart) / static_cast<double>(sliceEnd - sliceStart));
        sequenceProgress = (static_cast<double>(sequenceStep) + localSliceProgress)
            / static_cast<double>(sliceCount);

        if (sourcePosition >= static_cast<double>(sliceEnd))
        {
            const auto overshoot = sourcePosition - static_cast<double>(sliceEnd);
            ++sequenceStep;

            if (sequenceStep >= sliceCount)
            {
                playing = false;
                stopping = false;
                playbackProgress = static_cast<double>(sliceEnd) / totalFrames;
                sequenceProgress = 1.0;
                playbackEnvelope.setCurrentAndTargetValue(0.0f);
                break;
            }

            activeSlice = sequenceOrder[static_cast<std::size_t>(sequenceStep)];
            sourcePosition = sliceStartFrame(activeSlice, totalFrames) + overshoot;
            playbackProgress = juce::jlimit(
                0.0, 1.0, sourcePosition / static_cast<double>(totalFrames));
            sequenceProgress = static_cast<double>(sequenceStep) / sliceCount;
        }

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

int SlicePlaybackEngine::sliceStartFrame(const int slice, const int totalFrames) const noexcept
{
    return static_cast<int>((static_cast<std::int64_t>(slice) * totalFrames) / sliceCount);
}

int SlicePlaybackEngine::sliceEndFrame(const int slice, const int totalFrames) const noexcept
{
    return static_cast<int>((static_cast<std::int64_t>(slice + 1) * totalFrames) / sliceCount);
}

void SlicePlaybackEngine::resetIdentityOrder() noexcept
{
    for (std::size_t index = 0; index < sequenceOrder.size(); ++index)
        sequenceOrder[index] = static_cast<int>(index);
}

} // namespace saucechop
