#include "PluginProcessor.h"

#include "Parameters.h"
#include "PluginEditor.h"

namespace
{
constexpr auto stateTreeName = "SauceChopState";
} // namespace

SauceChopAudioProcessor::SauceChopAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput(
          "Output", juce::AudioChannelSet::stereo(), true)),
      parameterState(*this, nullptr, stateTreeName, createParameterLayout())
{
}

SauceChopAudioProcessor::~SauceChopAudioProcessor() = default;

void SauceChopAudioProcessor::prepareToPlay(const double sampleRate,
                                             const int maximumExpectedSamplesPerBlock)
{
    juce::ignoreUnused(sampleRate, maximumExpectedSamplesPerBlock);
}

void SauceChopAudioProcessor::releaseResources()
{
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

    // M1 is intentionally silent. The sample playback engine is introduced in M2/M3.
    buffer.clear();
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
    return 0.0;
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

            restoredState.removeProperty(saucechop::stateProperties::samplePath, nullptr);
            restoredState.removeProperty(saucechop::stateProperties::sampleFileSize, nullptr);
            restoredState.removeProperty(saucechop::stateProperties::sampleModifiedTime, nullptr);
            parameterState.replaceState(restoredState);
            sourceSample.store(nullptr);
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
    currentLoadState.store(SampleLoadState::loading);
    setLoadMessage("Loading " + file.getFileName() + "…");
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
    return sourceSample.load();
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
    sourceSample.store(loadedSample);
    currentLoadState.store(SampleLoadState::ready);
    setLoadMessage("Sample ready");
    setSampleReference({loadedSample->sourceFile.getFullPathName(),
                        loadedSample->fileSizeBytes,
                        loadedSample->modificationTime.toMilliseconds()});

    sendChangeMessage();
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

    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SauceChopAudioProcessor();
}
