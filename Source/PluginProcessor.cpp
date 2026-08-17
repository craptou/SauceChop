#include "PluginProcessor.h"

#include "PluginEditor.h"

namespace
{
constexpr auto stateTreeName = "SauceChopState";
constexpr auto outputGainParameterId = "outputGain";
constexpr auto sliceCountParameterId = "sliceCount";
constexpr auto midiBaseNoteParameterId = "midiBaseNote";
} // namespace

SauceChopAudioProcessor::SauceChopAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput(
          "Output", juce::AudioChannelSet::stereo(), true)),
      parameterState(*this, nullptr, stateTreeName, createParameterLayout())
{
}

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
    if (auto xml = parameterState.copyState().createXml())
        copyXmlToBinary(*xml, destinationData);
}

void SauceChopAudioProcessor::setStateInformation(const void* data, const int sizeInBytes)
{
    if (const auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        if (xml->hasTagName(parameterState.state.getType()))
            parameterState.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout
SauceChopAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{outputGainParameterId, 1},
        "Output Gain",
        juce::NormalisableRange<float>{-60.0f, 6.0f, 0.1f},
        0.0f,
        juce::AudioParameterFloatAttributes{}.withLabel("dB")));

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{sliceCountParameterId, 1},
        "Slice Count",
        juce::StringArray{"4", "8", "16", "32"},
        2));

    layout.add(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID{midiBaseNoteParameterId, 1}, "MIDI Base Note", 0, 127, 60));

    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SauceChopAudioProcessor();
}
