#include "../Source/audio/MidiVoiceEngine.h"

#include <juce_core/juce_core.h>

#include <cmath>
#include <memory>

namespace
{
std::unique_ptr<saucechop::SourceSample> makeSlicedSample(const int frames = 400,
                                                          const double sampleRate = 1000.0)
{
    auto sample = std::make_unique<saucechop::SourceSample>();
    sample->audio.setSize(1, frames);

    for (int frame = 0; frame < frames; ++frame)
        sample->audio.setSample(0, frame,
                                0.2f * static_cast<float>(frame / (frames / 4) + 1));

    sample->originalSampleRateHz = sampleRate;
    sample->frameCount = frames;
    sample->channelCount = 1;
    return sample;
}

class MidiVoiceEngineTests final : public juce::UnitTest
{
public:
    MidiVoiceEngineTests()
        : UnitTest("MIDI voice engine", "audio")
    {
    }

    void runTest() override
    {
        beginTest("A note maps from the base note and honours its block offset");
        {
            auto sample = makeSlicedSample();
            saucechop::MidiVoiceEngine engine;
            engine.prepare(1000.0);
            engine.setSource(sample.get());
            engine.setSliceCount(4);

            juce::MidiBuffer midi;
            midi.addEvent(juce::MidiMessage::noteOn(1, 62, 1.0f), 7);
            juce::AudioBuffer<float> output(2, 32);
            output.clear();
            engine.processMidi(output, midi, 60, 1.0f);

            expectWithinAbsoluteError(output.getMagnitude(0, 0, 7), 0.0f, 0.000001f);
            expect(output.getSample(0, 12) > 0.4f);
            expectWithinAbsoluteError(output.getSample(0, 12),
                                      output.getSample(1, 12),
                                      0.000001f);
            expectEquals(engine.mostRecentActiveSlice(), 2);
        }

        beginTest("Velocity scales the voice level");
        {
            auto sample = makeSlicedSample();
            saucechop::MidiVoiceEngine quiet;
            saucechop::MidiVoiceEngine loud;
            quiet.prepare(1000.0);
            loud.prepare(1000.0);
            quiet.setSource(sample.get());
            loud.setSource(sample.get());
            quiet.setSliceCount(4);
            loud.setSliceCount(4);

            juce::MidiBuffer quietMidi;
            juce::MidiBuffer loudMidi;
            quietMidi.addEvent(juce::MidiMessage::noteOn(1, 60, juce::uint8{32}), 0);
            loudMidi.addEvent(juce::MidiMessage::noteOn(1, 60, juce::uint8{127}), 0);
            juce::AudioBuffer<float> quietOutput(1, 16);
            juce::AudioBuffer<float> loudOutput(1, 16);
            quietOutput.clear();
            loudOutput.clear();
            quiet.processMidi(quietOutput, quietMidi, 60, 1.0f);
            loud.processMidi(loudOutput, loudMidi, 60, 1.0f);

            expectWithinAbsoluteError(quietOutput.getSample(0, 8) * (127.0f / 32.0f),
                                      loudOutput.getSample(0, 8),
                                      0.0001f);
        }

        beginTest("One Shot ignores Note Off");
        {
            auto sample = makeSlicedSample();
            saucechop::MidiVoiceEngine engine;
            engine.prepare(1000.0);
            engine.setSource(sample.get());
            engine.setSliceCount(4);
            engine.setPlayMode(saucechop::MidiVoiceEngine::PlayMode::oneShot);

            juce::MidiBuffer midi;
            midi.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0);
            midi.addEvent(juce::MidiMessage::noteOff(1, 60), 10);
            juce::AudioBuffer<float> output(1, 32);
            output.clear();
            engine.processMidi(output, midi, 60, 1.0f);

            expect(output.getSample(0, 20) > 0.1f);
            expectEquals(engine.activeVoiceCount(), 1);
        }

        beginTest("Gate releases a voice after Note Off");
        {
            auto sample = makeSlicedSample();
            saucechop::MidiVoiceEngine engine;
            engine.prepare(1000.0);
            engine.setSource(sample.get());
            engine.setSliceCount(4);
            engine.setPlayMode(saucechop::MidiVoiceEngine::PlayMode::gate);

            juce::MidiBuffer midi;
            midi.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0);
            midi.addEvent(juce::MidiMessage::noteOff(1, 60), 10);
            juce::AudioBuffer<float> output(1, 32);
            output.clear();
            engine.processMidi(output, midi, 60, 1.0f);

            expect(output.getSample(0, 11) > 0.0f);
            expectWithinAbsoluteError(output.getSample(0, 20), 0.0f, 0.000001f);
            expectEquals(engine.activeVoiceCount(), 0);
        }

        beginTest("The fixed pool steals deterministically and stays bounded");
        {
            auto sample = makeSlicedSample(3200, 1000.0);
            saucechop::MidiVoiceEngine engine;
            engine.prepare(1000.0);
            engine.setSource(sample.get());
            engine.setSliceCount(32);

            juce::MidiBuffer midi;

            for (int note = 60; note < 77; ++note)
                midi.addEvent(juce::MidiMessage::noteOn(1, note, 1.0f), 0);

            juce::AudioBuffer<float> output(2, 16);
            output.clear();
            engine.processMidi(output, midi, 60, 1.0f);

            expectEquals(engine.activeVoiceCount(), saucechop::MidiVoiceEngine::maximumVoices);
            expectEquals(engine.mostRecentActiveSlice(), 16);

            for (int frame = 0; frame < output.getNumSamples(); ++frame)
                expect(std::isfinite(output.getSample(0, frame)));
        }
    }
};

MidiVoiceEngineTests midiVoiceEngineTests;
} // namespace
