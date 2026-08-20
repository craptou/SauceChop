#pragma once

#include "audio/SlicePlaybackEngine.h"
#include "files/SampleLoader.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <array>
#include <cstdint>
#include <memory>
#include <vector>

class SauceChopAudioProcessor final : public juce::AudioProcessor,
                                     public juce::ChangeBroadcaster
{
public:
    enum class SampleLoadState
    {
        empty,
        loading,
        ready,
        error
    };

    SauceChopAudioProcessor();
    ~SauceChopAudioProcessor() override;

    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    [[nodiscard]] juce::AudioProcessorEditor* createEditor() override;
    [[nodiscard]] bool hasEditor() const override;

    [[nodiscard]] const juce::String getName() const override;
    [[nodiscard]] bool acceptsMidi() const override;
    [[nodiscard]] bool producesMidi() const override;
    [[nodiscard]] bool isMidiEffect() const override;
    [[nodiscard]] double getTailLengthSeconds() const override;

    [[nodiscard]] int getNumPrograms() override;
    [[nodiscard]] int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    [[nodiscard]] const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destinationData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void loadSampleAsync(const juce::File& file);
    void startPlayback();
    void stopPlayback();
    void synchronisePlaybackControl();
    void setSequenceSliceCount(int sliceCount);
    void moveSequenceStep(int fromIndex, int toIndex);
    void resetSequenceOrder();

    [[nodiscard]] std::vector<int> sequenceOrderSnapshot() const;

    [[nodiscard]] bool isPlaybackRequested() const noexcept
    {
        return playbackRequested.load();
    }

    [[nodiscard]] bool isPlaybackActive() const noexcept
    {
        return audioIsPlaying.load();
    }

    [[nodiscard]] float playbackPosition() const noexcept
    {
        return playbackProgress.load();
    }

    [[nodiscard]] float playbackSequencePosition() const noexcept
    {
        return playbackSequenceProgress.load();
    }

    [[nodiscard]] int currentPlaybackSlice() const noexcept
    {
        return playbackSlice.load();
    }

    [[nodiscard]] int currentPlaybackSequenceStep() const noexcept
    {
        return playbackSequenceStep.load();
    }

    [[nodiscard]] std::shared_ptr<const saucechop::SourceSample>
    sourceSampleSnapshot() const noexcept;

    [[nodiscard]] SampleLoadState sampleLoadState() const noexcept
    {
        return currentLoadState.load();
    }

    [[nodiscard]] juce::String sampleLoadMessage() const;

    [[nodiscard]] juce::AudioProcessorValueTreeState& parameters() noexcept
    {
        return parameterState;
    }

private:
    enum class PlaybackCommand
    {
        none,
        playFromStart,
        stop
    };

    struct SampleReference
    {
        juce::String path;
        std::int64_t fileSizeBytes = 0;
        std::int64_t modifiedTimeMilliseconds = 0;
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void setLoadMessage(juce::String message);
    void setSampleReference(SampleReference reference);
    [[nodiscard]] SampleReference sampleReferenceSnapshot() const;
    void handleSampleLoadResult(saucechop::SampleLoadResult result);
    void publishSourceSample(std::shared_ptr<const saucechop::SourceSample> sample);
    void reclaimRetiredSamples();
    void setSequenceOrder(std::vector<int> order, bool notifyListeners);
    void publishSequenceOrderLocked();
    void setPreviewPlayParameter(bool shouldPlay);

    juce::AudioProcessorValueTreeState parameterState;
    const std::atomic<float>* outputGainParameter = nullptr;
    const std::atomic<float>* sliceCountParameter = nullptr;
    const std::atomic<float>* previewPlayParameter = nullptr;

    std::atomic<const saucechop::SourceSample*> realtimeSourceSample{nullptr};
    std::atomic<const saucechop::SourceSample*> realtimeSampleHazard{nullptr};
    mutable juce::CriticalSection sampleOwnershipLock;
    std::shared_ptr<const saucechop::SourceSample> currentSourceSample;
    std::vector<std::shared_ptr<const saucechop::SourceSample>> retiredSourceSamples;

    saucechop::SlicePlaybackEngine playbackEngine;
    std::atomic<PlaybackCommand> pendingPlaybackCommand{PlaybackCommand::none};
    std::atomic<std::uint64_t> playbackCommandSequence{0};
    std::uint64_t consumedPlaybackCommandSequence = 0;
    std::atomic<bool> playbackRequested{false};
    std::atomic<bool> audioIsPlaying{false};
    bool audioPreviewPlayState = false;
    std::atomic<float> playbackProgress{0.0f};
    std::atomic<float> playbackSequenceProgress{0.0f};
    std::atomic<int> playbackSlice{-1};
    std::atomic<int> playbackSequenceStep{-1};

    mutable juce::CriticalSection sequenceOrderLock;
    std::vector<int> currentSequenceOrder;
    std::array<std::atomic<int>, 32> realtimeSequenceOrder{};
    std::atomic<int> realtimeSequenceCount{16};
    std::atomic<std::uint64_t> realtimeSequenceRevision{0};
    std::uint64_t consumedSequenceRevision = 0;
    int audioSliceCount = 0;

    std::atomic<SampleLoadState> currentLoadState{SampleLoadState::empty};
    mutable juce::CriticalSection loadMessageLock;
    juce::String currentLoadMessage;
    mutable juce::CriticalSection sampleReferenceLock;
    SampleReference currentSampleReference;
    saucechop::SampleLoader sampleLoader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SauceChopAudioProcessor)
};
