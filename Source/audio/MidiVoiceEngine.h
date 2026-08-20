#pragma once

#include "../files/SourceSample.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <cstdint>

namespace saucechop
{
class MidiVoiceEngine final
{
public:
    enum class PlayMode
    {
        oneShot,
        gate
    };

    static constexpr int maximumVoices = 16;

    void prepare(double newHostSampleRate) noexcept;
    void setSource(const SourceSample* newSource) noexcept;
    void setSliceCount(int newSliceCount) noexcept;
    void setPlayMode(PlayMode newMode) noexcept { playMode = newMode; }

    void processMidi(juce::AudioBuffer<float>& output,
                     const juce::MidiBuffer& midiMessages,
                     int midiBaseNote,
                     float outputGainLinear) noexcept;

    void stopAll(bool allowRelease) noexcept;

    [[nodiscard]] int activeVoiceCount() const noexcept;
    [[nodiscard]] int mostRecentActiveSlice() const noexcept;

private:
    struct Voice
    {
        double sourcePosition = 0.0;
        std::uint64_t age = 0;
        float velocity = 0.0f;
        float envelope = 0.0f;
        std::array<float, 2> lastOutput{};
        std::array<float, 2> stolenTail{};
        float stolenTailEnvelope = 0.0f;
        int midiNote = -1;
        int slice = -1;
        bool active = false;
        bool releasing = false;
    };

    void renderRange(juce::AudioBuffer<float>& output,
                     int startSample,
                     int numSamples) noexcept;
    void noteOn(int midiNote, int slice, float velocity) noexcept;
    void noteOff(int midiNote) noexcept;
    [[nodiscard]] Voice& chooseVoice() noexcept;
    [[nodiscard]] int sliceStartFrame(int slice, int totalFrames) const noexcept;
    [[nodiscard]] int sliceEndFrame(int slice, int totalFrames) const noexcept;

    const SourceSample* source = nullptr;
    double hostSampleRate = 44100.0;
    int sliceCount = 16;
    PlayMode playMode = PlayMode::oneShot;
    std::array<Voice, maximumVoices> voices{};
    std::uint64_t nextVoiceAge = 1;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGain;
};
} // namespace saucechop
