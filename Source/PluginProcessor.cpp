#include "PluginProcessor.h"

#include "Parameters.h"
#include "PluginEditor.h"
#include "model/SequenceOrder.h"

#include <array>
#include <limits>

namespace
{
constexpr auto stateTreeName = "SauceChopState";
constexpr std::array sliceCounts{4, 8, 16, 32};
} // namespace

SauceChopAudioProcessor::SauceChopAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput(
          "Output", juce::AudioChannelSet::stereo(), true)),
      parameterState(*this, nullptr, stateTreeName, createParameterLayout())
{
    outputGainParameter = parameterState.getRawParameterValue(
        saucechop::parameters::outputGain);
    sliceCountParameter = parameterState.getRawParameterValue(
        saucechop::parameters::sliceCount);
    previewPlayParameter = parameterState.getRawParameterValue(
        saucechop::parameters::previewPlay);
    jassert(outputGainParameter != nullptr && sliceCountParameter != nullptr
            && previewPlayParameter != nullptr);

    currentSequenceOrder = saucechop::makeIdentitySequence(16);
    publishSequenceOrderLocked();
}

SauceChopAudioProcessor::~SauceChopAudioProcessor() = default;

void SauceChopAudioProcessor::prepareToPlay(const double sampleRate,
                                             const int maximumExpectedSamplesPerBlock)
{
    playbackEngine.prepare(sampleRate, maximumExpectedSamplesPerBlock);
    consumedPlaybackCommandSequence = playbackCommandSequence.load();
    consumedSequenceRevision = std::numeric_limits<std::uint64_t>::max();
    audioSliceCount = 0;
    realtimeSampleHazard.store(nullptr);
    audioIsPlaying.store(false);
    audioPreviewPlayState = false;
    playbackProgress.store(0.0f);
    playbackSequenceProgress.store(0.0f);
    playbackSlice.store(-1);
    playbackSequenceStep.store(-1);
}

void SauceChopAudioProcessor::releaseResources()
{
    playbackEngine.setSource(nullptr);
    realtimeSampleHazard.store(nullptr);
    audioIsPlaying.store(false);
    playbackSequenceProgress.store(0.0f);
    playbackSequenceStep.store(-1);
}

bool SauceChopAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet().isDisabled()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SauceChopAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    const saucechop::SourceSample* publishedSample = nullptr;

    // The hazard-pointer handshake keeps the immutable sample alive while this
    // block is rendered without locks or shared_ptr destruction on the audio thread.
    do
    {
        publishedSample = realtimeSourceSample.load();
        realtimeSampleHazard.store(publishedSample);
    } while (publishedSample != realtimeSourceSample.load());

    playbackEngine.setSource(publishedSample);

    const auto sliceChoice = sliceCountParameter != nullptr
        ? juce::jlimit(0,
                       static_cast<int>(sliceCounts.size()) - 1,
                       juce::roundToInt(sliceCountParameter->load()))
        : 2;
    const auto requestedSliceCount = sliceCounts[static_cast<std::size_t>(sliceChoice)];
    const auto sliceCountChanged = requestedSliceCount != audioSliceCount;

    if (sliceCountChanged)
    {
        audioSliceCount = requestedSliceCount;
        playbackEngine.setSliceCount(audioSliceCount);
    }

    const auto sequenceRevisionBefore = realtimeSequenceRevision.load();

    if ((sequenceRevisionBefore & 1U) == 0U
        && (sequenceRevisionBefore != consumedSequenceRevision || sliceCountChanged))
    {
        std::array<int, 32> sequenceSnapshot{};
        const auto publishedCount = realtimeSequenceCount.load();

        for (int index = 0; index < publishedCount && index < audioSliceCount; ++index)
        {
            sequenceSnapshot[static_cast<std::size_t>(index)] =
                realtimeSequenceOrder[static_cast<std::size_t>(index)].load();
        }

        const auto sequenceRevisionAfter = realtimeSequenceRevision.load();

        if (sequenceRevisionBefore == sequenceRevisionAfter
            && publishedCount == audioSliceCount)
        {
            playbackEngine.setSequenceOrder(sequenceSnapshot.data(), publishedCount);
            consumedSequenceRevision = sequenceRevisionAfter;
        }
    }

    const auto parameterRequestsPlayback = previewPlayParameter != nullptr
        && previewPlayParameter->load() >= 0.5f;

    if (parameterRequestsPlayback != audioPreviewPlayState)
    {
        audioPreviewPlayState = parameterRequestsPlayback;
        playbackRequested.store(parameterRequestsPlayback);

        if (parameterRequestsPlayback)
            playbackEngine.start();
        else
            playbackEngine.requestStop();
    }

    PlaybackCommand command = PlaybackCommand::none;
    std::uint64_t commandSequence = 0;
    std::uint64_t verifiedSequence = 0;

    do
    {
        commandSequence = playbackCommandSequence.load();
        command = pendingPlaybackCommand.load();
        verifiedSequence = playbackCommandSequence.load();
    } while (commandSequence != verifiedSequence);

    if (commandSequence != consumedPlaybackCommandSequence)
    {
        consumedPlaybackCommandSequence = commandSequence;

        if (command == PlaybackCommand::playFromStart)
            playbackEngine.start();
        else if (command == PlaybackCommand::stop)
            playbackEngine.requestStop();
    }

    const auto outputGainDb = outputGainParameter != nullptr ? outputGainParameter->load() : 0.0f;
    playbackEngine.process(buffer, juce::Decibels::decibelsToGain(outputGainDb));

    const auto playing = playbackEngine.isPlaying();
    audioIsPlaying.store(playing);
    playbackProgress.store(static_cast<float>(playbackEngine.progress()));
    playbackSequenceProgress.store(
        static_cast<float>(playbackEngine.sequenceProgressValue()));
    playbackSlice.store(playbackEngine.currentSlice());
    playbackSequenceStep.store(playbackEngine.currentSequenceStep());

    if (!playing && playbackRequested.load())
        playbackRequested.store(false);
}

juce::AudioProcessorEditor* SauceChopAudioProcessor::createEditor()
{
    return new SauceChopAudioProcessorEditor(*this);
}

bool SauceChopAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String SauceChopAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SauceChopAudioProcessor::acceptsMidi() const
{
    return true;
}

bool SauceChopAudioProcessor::producesMidi() const
{
    return false;
}

bool SauceChopAudioProcessor::isMidiEffect() const
{
    return false;
}

double SauceChopAudioProcessor::getTailLengthSeconds() const
{
    // The plug-in can start generating from its own UI without MIDI or input.
    // VST3 maps an infinite tail to an always-processable generator.
    return std::numeric_limits<double>::infinity();
}

int SauceChopAudioProcessor::getNumPrograms()
{
    return 1;
}

int SauceChopAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SauceChopAudioProcessor::setCurrentProgram(const int index)
{
    juce::ignoreUnused(index);
}

const juce::String SauceChopAudioProcessor::getProgramName(const int index)
{
    juce::ignoreUnused(index);
    return {};
}

void SauceChopAudioProcessor::changeProgramName(const int index,
                                                const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void SauceChopAudioProcessor::getStateInformation(juce::MemoryBlock& destinationData)
{
    auto state = parameterState.copyState();
    const auto reference = sampleReferenceSnapshot();
    state.setProperty(saucechop::stateProperties::samplePath, reference.path, nullptr);
    state.setProperty(
        saucechop::stateProperties::sampleFileSize, reference.fileSizeBytes, nullptr);
    state.setProperty(saucechop::stateProperties::sampleModifiedTime,
                      reference.modifiedTimeMilliseconds,
                      nullptr);
    state.setProperty(saucechop::stateProperties::sequenceOrder,
                      saucechop::serialiseSequence(sequenceOrderSnapshot()),
                      nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destinationData);
}

void SauceChopAudioProcessor::setStateInformation(const void* data, const int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName(parameterState.state.getType()))
        {
            auto restoredState = juce::ValueTree::fromXml(*xml);
            SampleReference restoredReference;
            restoredReference.path = restoredState
                                         .getProperty(saucechop::stateProperties::samplePath)
                                         .toString();
            restoredReference.fileSizeBytes = static_cast<std::int64_t>(
                restoredState.getProperty(saucechop::stateProperties::sampleFileSize));
            restoredReference.modifiedTimeMilliseconds = static_cast<std::int64_t>(
                restoredState.getProperty(saucechop::stateProperties::sampleModifiedTime));
            const auto restoredSequenceText = restoredState
                                                  .getProperty(
                                                      saucechop::stateProperties::sequenceOrder)
                                                  .toString();

            restoredState.removeProperty(saucechop::stateProperties::samplePath, nullptr);
            restoredState.removeProperty(saucechop::stateProperties::sampleFileSize, nullptr);
            restoredState.removeProperty(saucechop::stateProperties::sampleModifiedTime, nullptr);
            restoredState.removeProperty(saucechop::stateProperties::sequenceOrder, nullptr);
            parameterState.replaceState(restoredState);

            if (auto* previewParameter = parameterState.getParameter(
                    saucechop::parameters::previewPlay))
            {
                previewParameter->setValue(0.0f);
            }

            const auto restoredSliceChoice = sliceCountParameter != nullptr
                ? juce::jlimit(0,
                               static_cast<int>(sliceCounts.size()) - 1,
                               juce::roundToInt(sliceCountParameter->load()))
                : 2;
            const auto restoredSliceCount =
                sliceCounts[static_cast<std::size_t>(restoredSliceChoice)];
            auto restoredSequence =
                saucechop::parseSequence(restoredSequenceText, restoredSliceCount);

            if (restoredSequence.empty())
                restoredSequence = saucechop::makeIdentitySequence(restoredSliceCount);

            setSequenceOrder(std::move(restoredSequence), false);
            publishSourceSample(nullptr);
            setSampleReference(restoredReference);

            if (restoredReference.path.isNotEmpty())
            {
                loadSampleAsync(juce::File{restoredReference.path});
            }
            else
            {
                currentLoadState.store(SampleLoadState::empty);
                setLoadMessage({});
                sendChangeMessage();
            }
        }
    }
}

void SauceChopAudioProcessor::loadSampleAsync(const juce::File& file)
{
    stopPlayback();
    currentLoadState.store(SampleLoadState::loading);
    setLoadMessage("Loading " + file.getFileName() + "...");
    sendChangeMessage();

    sampleLoader.load(file,
                      [this](saucechop::SampleLoadResult result)
                      {
                          handleSampleLoadResult(std::move(result));
                      });
}

std::shared_ptr<const saucechop::SourceSample>
SauceChopAudioProcessor::sourceSampleSnapshot() const noexcept
{
    const juce::ScopedLock lock(sampleOwnershipLock);
    return currentSourceSample;
}

void SauceChopAudioProcessor::startPlayback()
{
    if (realtimeSourceSample.load() == nullptr)
        return;

    setPreviewPlayParameter(true);
    playbackRequested.store(true);
    pendingPlaybackCommand.store(PlaybackCommand::playFromStart);
    playbackCommandSequence.fetch_add(1);
}

void SauceChopAudioProcessor::stopPlayback()
{
    setPreviewPlayParameter(false);
    playbackRequested.store(false);
    pendingPlaybackCommand.store(PlaybackCommand::stop);
    playbackCommandSequence.fetch_add(1);
}

void SauceChopAudioProcessor::synchronisePlaybackControl()
{
    if (!playbackRequested.load() && !audioIsPlaying.load()
        && previewPlayParameter != nullptr && previewPlayParameter->load() >= 0.5f)
    {
        setPreviewPlayParameter(false);
    }
}

void SauceChopAudioProcessor::setSequenceSliceCount(const int sliceCount)
{
    if (sliceCount != 4 && sliceCount != 8 && sliceCount != 16 && sliceCount != 32)
        return;

    const auto currentOrder = sequenceOrderSnapshot();

    if (static_cast<int>(currentOrder.size()) == sliceCount)
        return;

    stopPlayback();
    setSequenceOrder(saucechop::makeIdentitySequence(sliceCount), true);
}

void SauceChopAudioProcessor::moveSequenceStep(const int fromIndex, const int toIndex)
{
    const auto currentOrder = sequenceOrderSnapshot();
    const auto movedOrder = saucechop::moveSequenceItem(currentOrder, fromIndex, toIndex);

    if (movedOrder != currentOrder)
        setSequenceOrder(movedOrder, true);
}

void SauceChopAudioProcessor::resetSequenceOrder()
{
    const auto currentOrder = sequenceOrderSnapshot();
    const auto identity = saucechop::makeIdentitySequence(
        static_cast<int>(currentOrder.size()));

    if (currentOrder != identity)
        setSequenceOrder(identity, true);
}

std::vector<int> SauceChopAudioProcessor::sequenceOrderSnapshot() const
{
    const juce::ScopedLock lock(sequenceOrderLock);
    return currentSequenceOrder;
}

juce::String SauceChopAudioProcessor::sampleLoadMessage() const
{
    const juce::ScopedLock lock(loadMessageLock);
    return currentLoadMessage;
}

void SauceChopAudioProcessor::setLoadMessage(juce::String message)
{
    const juce::ScopedLock lock(loadMessageLock);
    currentLoadMessage = std::move(message);
}

void SauceChopAudioProcessor::setSampleReference(SampleReference reference)
{
    const juce::ScopedLock lock(sampleReferenceLock);
    currentSampleReference = std::move(reference);
}

SauceChopAudioProcessor::SampleReference
SauceChopAudioProcessor::sampleReferenceSnapshot() const
{
    const juce::ScopedLock lock(sampleReferenceLock);
    return currentSampleReference;
}

void SauceChopAudioProcessor::handleSampleLoadResult(saucechop::SampleLoadResult result)
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

    if (!result.succeeded())
    {
        currentLoadState.store(SampleLoadState::error);
        setLoadMessage(result.errorMessage.isNotEmpty()
                           ? std::move(result.errorMessage)
                           : juce::String{"The audio file could not be loaded."});
        sendChangeMessage();
        return;
    }

    const auto loadedSample = std::move(result.sample);
    publishSourceSample(loadedSample);
    currentLoadState.store(SampleLoadState::ready);
    setLoadMessage("Sample ready");
    setSampleReference({loadedSample->sourceFile.getFullPathName(),
                        loadedSample->fileSizeBytes,
                        loadedSample->modificationTime.toMilliseconds()});

    sendChangeMessage();
}

void SauceChopAudioProcessor::publishSourceSample(
    std::shared_ptr<const saucechop::SourceSample> sample)
{
    const juce::ScopedLock lock(sampleOwnershipLock);

    if (currentSourceSample != nullptr)
        retiredSourceSamples.push_back(std::move(currentSourceSample));

    currentSourceSample = std::move(sample);
    realtimeSourceSample.store(currentSourceSample.get());
    playbackProgress.store(0.0f);
    playbackSequenceProgress.store(0.0f);
    playbackSlice.store(-1);
    playbackSequenceStep.store(-1);
    reclaimRetiredSamples();
}

void SauceChopAudioProcessor::reclaimRetiredSamples()
{
    const auto* protectedSample = realtimeSampleHazard.load();
    std::erase_if(retiredSourceSamples,
                  [protectedSample](const auto& sample)
                  {
                      return sample.get() != protectedSample;
                  });
}

void SauceChopAudioProcessor::setSequenceOrder(std::vector<int> order,
                                               const bool notifyListeners)
{
    const auto count = static_cast<int>(order.size());

    if (count > static_cast<int>(realtimeSequenceOrder.size())
        || !saucechop::isValidSequence(order, count))
    {
        return;
    }

    {
        const juce::ScopedLock lock(sequenceOrderLock);
        currentSequenceOrder = std::move(order);
        publishSequenceOrderLocked();
    }

    if (notifyListeners)
        sendChangeMessage();
}

void SauceChopAudioProcessor::publishSequenceOrderLocked()
{
    realtimeSequenceRevision.fetch_add(1);
    realtimeSequenceCount.store(static_cast<int>(currentSequenceOrder.size()));

    for (std::size_t index = 0; index < currentSequenceOrder.size(); ++index)
        realtimeSequenceOrder[index].store(currentSequenceOrder[index]);

    realtimeSequenceRevision.fetch_add(1);
}

void SauceChopAudioProcessor::setPreviewPlayParameter(const bool shouldPlay)
{
    const auto alreadyAtTarget = previewPlayParameter != nullptr
        && (previewPlayParameter->load() >= 0.5f) == shouldPlay;

    if (alreadyAtTarget && !shouldPlay)
        return;

    if (auto* parameter = parameterState.getParameter(saucechop::parameters::previewPlay))
    {
        const auto normalisedValue = shouldPlay ? 1.0f : 0.0f;

        if (!alreadyAtTarget)
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(normalisedValue);
            parameter->endChangeGesture();
        }
    }

    updateHostDisplay(juce::AudioProcessorListener::ChangeDetails{}
                          .withNonParameterStateChanged(true));
}

juce::AudioProcessorValueTreeState::ParameterLayout
SauceChopAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{saucechop::parameters::outputGain, 1},
        "Output Gain",
        juce::NormalisableRange<float>{-60.0f, 6.0f, 0.1f},
        0.0f,
        juce::AudioParameterFloatAttributes{}.withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{saucechop::parameters::sliceCount, 1},
        "Slice Count",
        juce::StringArray{"4", "8", "16", "32"},
        2));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{saucechop::parameters::midiBaseNote, 1},
        "MIDI Base Note",
        0,
        127,
        60));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{saucechop::parameters::previewPlay, 1},
        "Preview Play",
        false));

    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SauceChopAudioProcessor();
}
