#include "MidiVoiceEngine.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace saucechop
{
namespace
{
constexpr double attackSeconds = 0.003;
constexpr double releaseSeconds = 0.005;
constexpr double sliceFadeSeconds = 0.003;
constexpr double gainSmoothingSeconds = 0.02;
}

void MidiVoiceEngine::prepare(const double newHostSampleRate) noexcept
{
    hostSampleRate = newHostSampleRate > 0.0 ? newHostSampleRate : 44100.0;
    outputGain.reset(hostSampleRate, gainSmoothingSeconds);
    outputGain.setCurrentAndTargetValue(1.0f);
    stopAll(false);
}

void MidiVoiceEngine::setSource(const SourceSample* const newSource) noexcept
{
    if (source == newSource)
        return;

    source = newSource;
    stopAll(false);
}

void MidiVoiceEngine::setSliceCount(const int newSliceCount) noexcept
{
    const auto sanitisedCount = juce::jlimit(1, 32, newSliceCount);

    if (sliceCount == sanitisedCount)
        return;

    sliceCount = sanitisedCount;
    stopAll(false);
}

void MidiVoiceEngine::processMidi(juce::AudioBuffer<float>& output,
                                  const juce::MidiBuffer& midiMessages,
                                  const int midiBaseNote,
                                  const float outputGainLinear) noexcept
{
    outputGain.setTargetValue(juce::jmax(0.0f, outputGainLinear));
    const auto baseNote = juce::jlimit(0, 127, midiBaseNote);
    auto cursor = 0;

    for (const auto metadata : midiMessages)
    {
        const auto eventPosition = juce::jlimit(0, output.getNumSamples(),
                                                metadata.samplePosition);

        if (eventPosition > cursor)
            renderRange(output, cursor, eventPosition - cursor);

        const auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            const auto note = message.getNoteNumber();
            const auto slice = note - baseNote;

            if (slice >= 0 && slice < sliceCount)
                noteOn(note, slice, message.getFloatVelocity());
        }
        else if (message.isNoteOff())
        {
            noteOff(message.getNoteNumber());
        }
        else if (message.isAllSoundOff())
        {
            stopAll(false);
        }
        else if (message.isAllNotesOff())
        {
            stopAll(true);
        }

        cursor = juce::jmax(cursor, eventPosition);
    }

    if (cursor < output.getNumSamples())
        renderRange(output, cursor, output.getNumSamples() - cursor);
}

void MidiVoiceEngine::stopAll(const bool allowRelease) noexcept
{
    for (auto& voice : voices)
    {
        if (allowRelease && voice.active)
            voice.releasing = true;
        else
            voice = {};
    }
}

int MidiVoiceEngine::activeVoiceCount() const noexcept
{
    return static_cast<int>(std::count_if(voices.begin(), voices.end(),
                                          [](const Voice& voice)
                                          {
                                              return voice.active;
                                          }));
}

int MidiVoiceEngine::mostRecentActiveSlice() const noexcept
{
    const Voice* mostRecent = nullptr;

    for (const auto& voice : voices)
        if (voice.active && (mostRecent == nullptr || voice.age > mostRecent->age))
            mostRecent = &voice;

    return mostRecent != nullptr ? mostRecent->slice : -1;
}

void MidiVoiceEngine::renderRange(juce::AudioBuffer<float>& output,
                                  const int startSample,
                                  const int numSamples) noexcept
{
    if (numSamples <= 0)
        return;

    if (source == nullptr || source->audio.getNumSamples() <= 0
        || source->audio.getNumChannels() <= 0 || source->originalSampleRateHz <= 0.0
        || output.getNumChannels() <= 0)
    {
        outputGain.skip(numSamples);
        return;
    }

    const auto totalFrames = source->audio.getNumSamples();
    const auto sourceChannels = source->audio.getNumChannels();
    const auto outputChannels = output.getNumChannels();
    const auto sourceFramesPerOutputFrame = source->originalSampleRateHz / hostSampleRate;
    const auto fadeFrames = juce::jmax(1.0, source->originalSampleRateHz * sliceFadeSeconds);
    const auto attackStep = static_cast<float>(1.0 / juce::jmax(1.0,
                                                               hostSampleRate * attackSeconds));
    const auto releaseStep = static_cast<float>(1.0 / juce::jmax(1.0,
                                                                hostSampleRate * releaseSeconds));

    for (int frameOffset = 0; frameOffset < numSamples; ++frameOffset)
    {
        const auto globalGain = outputGain.getNextValue();

        for (auto& voice : voices)
        {
            if (!voice.active)
                continue;

            if (voice.stolenTailEnvelope > 0.0f)
            {
                for (int outputChannel = 0; outputChannel < outputChannels; ++outputChannel)
                {
                    const auto tailChannel = juce::jmin(outputChannel, 1);
                    output.addSample(outputChannel,
                                     startSample + frameOffset,
                                     voice.stolenTail[static_cast<std::size_t>(tailChannel)]
                                         * voice.stolenTailEnvelope * globalGain);
                }

                voice.stolenTailEnvelope = juce::jmax(
                    0.0f, voice.stolenTailEnvelope - releaseStep);
            }

            const auto sliceStart = sliceStartFrame(voice.slice, totalFrames);
            const auto sliceEnd = sliceEndFrame(voice.slice, totalFrames);

            if (sliceEnd <= sliceStart || voice.sourcePosition >= sliceEnd)
            {
                voice = {};
                continue;
            }

            if (voice.releasing)
                voice.envelope = juce::jmax(0.0f, voice.envelope - releaseStep);
            else
                voice.envelope = juce::jmin(1.0f, voice.envelope + attackStep);

            if (voice.envelope <= 0.0f && voice.releasing)
            {
                voice = {};
                continue;
            }

            const auto firstFrame = juce::jlimit(0, totalFrames - 1,
                                                 static_cast<int>(voice.sourcePosition));
            const auto secondFrame = juce::jmin(firstFrame + 1, sliceEnd - 1);
            const auto fraction = static_cast<float>(voice.sourcePosition - firstFrame);
            const auto framesFromStart = voice.sourcePosition - sliceStart;
            const auto framesRemaining = sliceEnd - voice.sourcePosition;
            const auto boundaryGain = static_cast<float>(juce::jlimit(
                0.0, 1.0, std::min(framesFromStart, framesRemaining) / fadeFrames));
            const auto voiceGain = globalGain * voice.velocity * voice.envelope * boundaryGain;

            for (int outputChannel = 0; outputChannel < outputChannels; ++outputChannel)
            {
                const auto sourceChannel = juce::jmin(outputChannel, sourceChannels - 1);
                const auto* sourceData = source->audio.getReadPointer(sourceChannel);
                const auto firstValue = sourceData[firstFrame];
                const auto value = firstValue
                    + fraction * (sourceData[secondFrame] - firstValue);
                voice.lastOutput[static_cast<std::size_t>(juce::jmin(outputChannel, 1))] =
                    value * voice.velocity * voice.envelope * boundaryGain;
                output.addSample(outputChannel, startSample + frameOffset,
                                 value * voiceGain);
            }

            voice.sourcePosition += sourceFramesPerOutputFrame;

            if (voice.sourcePosition >= sliceEnd)
                voice = {};
        }
    }
}

void MidiVoiceEngine::noteOn(const int midiNote,
                             const int slice,
                             const float velocity) noexcept
{
    if (source == nullptr || velocity <= 0.0f)
        return;

    auto& voice = chooseVoice();
    const auto stolenTail = voice.lastOutput;
    const auto wasStolen = voice.active;
    voice = {};
    voice.stolenTail = stolenTail;
    voice.stolenTailEnvelope = wasStolen ? 1.0f : 0.0f;
    voice.sourcePosition = sliceStartFrame(slice, source->audio.getNumSamples());
    voice.age = nextVoiceAge++;
    voice.velocity = juce::jlimit(0.0f, 1.0f, velocity);
    voice.midiNote = midiNote;
    voice.slice = slice;
    voice.active = true;
}

void MidiVoiceEngine::noteOff(const int midiNote) noexcept
{
    if (playMode == PlayMode::oneShot)
        return;

    Voice* oldestMatch = nullptr;

    for (auto& voice : voices)
        if (voice.active && !voice.releasing && voice.midiNote == midiNote
            && (oldestMatch == nullptr || voice.age < oldestMatch->age))
            oldestMatch = &voice;

    if (oldestMatch != nullptr)
        oldestMatch->releasing = true;
}

MidiVoiceEngine::Voice& MidiVoiceEngine::chooseVoice() noexcept
{
    const auto freeVoice = std::find_if(voices.begin(), voices.end(),
                                        [](const Voice& voice)
                                        {
                                            return !voice.active;
                                        });

    if (freeVoice != voices.end())
        return *freeVoice;

    auto selected = voices.begin();

    for (auto candidate = voices.begin() + 1; candidate != voices.end(); ++candidate)
    {
        if (candidate->releasing != selected->releasing)
        {
            if (candidate->releasing)
                selected = candidate;

            continue;
        }

        const auto candidateLevel = candidate->envelope * candidate->velocity;
        const auto selectedLevel = selected->envelope * selected->velocity;

        if (candidateLevel < selectedLevel
            || (candidateLevel == selectedLevel && candidate->age < selected->age))
            selected = candidate;
    }

    return *selected;
}

int MidiVoiceEngine::sliceStartFrame(const int slice, const int totalFrames) const noexcept
{
    return static_cast<int>((static_cast<std::int64_t>(slice) * totalFrames) / sliceCount);
}

int MidiVoiceEngine::sliceEndFrame(const int slice, const int totalFrames) const noexcept
{
    return static_cast<int>((static_cast<std::int64_t>(slice + 1) * totalFrames) / sliceCount);
}
} // namespace saucechop
