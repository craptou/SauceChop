#pragma once

#include "files/SampleLoader.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <atomic>
#include <memory>

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

    juce::AudioProcessorValueTreeState parameterState;
    std::atomic<std::shared_ptr<const saucechop::SourceSample>> sourceSample;
    std::atomic<SampleLoadState> currentLoadState{SampleLoadState::empty};
    mutable juce::CriticalSection loadMessageLock;
    juce::String currentLoadMessage;
    mutable juce::CriticalSection sampleReferenceLock;
    SampleReference currentSampleReference;
    saucechop::SampleLoader sampleLoader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SauceChopAudioProcessor)
};
