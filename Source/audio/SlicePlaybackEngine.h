#pragma once

#include "../files/SourceSample.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

namespace saucechop
{
class SlicePlaybackEngine final
{
public:
    SlicePlaybackEngine() noexcept;

    void prepare(double newHostSampleRate, int maximumExpectedSamplesPerBlock) noexcept;
    void setSource(const SourceSample* newSource) noexcept;
    void setSliceCount(int newSliceCount) noexcept;
    void setSequenceOrder(const int* newOrder, int orderSize) noexcept;

    void start() noexcept;
    void requestStop() noexcept;
    void stopImmediately() noexcept;
    void process(juce::AudioBuffer<float>& output, float outputGainLinear) noexcept;

    [[nodiscard]] bool isPlaying() const noexcept { return playing; }
    [[nodiscard]] double progress() const noexcept { return playbackProgress; }
    [[nodiscard]] double sequenceProgressValue() const noexcept { return sequenceProgress; }
    [[nodiscard]] int currentSlice() const noexcept { return activeSlice; }
    [[nodiscard]] int currentSequenceStep() const noexcept { return sequenceStep; }
    [[nodiscard]] const SourceSample* sourceSample() const noexcept { return source; }

private:
    [[nodiscard]] int sliceStartFrame(int slice, int totalFrames) const noexcept;
    [[nodiscard]] int sliceEndFrame(int slice, int totalFrames) const noexcept;
    void resetIdentityOrder() noexcept;

    const SourceSample* source = nullptr;
    double hostSampleRate = 44100.0;
    double sourcePosition = 0.0;
    double playbackProgress = 0.0;
    double sequenceProgress = 0.0;
    int sliceCount = 16;
    int activeSlice = -1;
    int sequenceStep = -1;
    std::array<int, 32> sequenceOrder{};
    bool playing = false;
    bool stopping = false;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> playbackEnvelope;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGain;
};
} // namespace saucechop
