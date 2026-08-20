#include "../Source/audio/SlicePlaybackEngine.h"

#include <juce_core/juce_core.h>

#include <cmath>
#include <memory>

namespace
{
std::unique_ptr<saucechop::SourceSample> makeConstantSample(const int channels,
                                                            const int frames,
                                                            const double sampleRate,
                                                            const float value = 1.0f)
{
    auto sample = std::make_unique<saucechop::SourceSample>();
    sample->audio.setSize(channels, frames);
    sample->audio.clear();

    for (int channel = 0; channel < channels; ++channel)
        for (int frame = 0; frame < frames; ++frame)
            sample->audio.setSample(channel, frame, value);

    sample->originalSampleRateHz = sampleRate;
    sample->frameCount = frames;
    sample->channelCount = channels;
    return sample;
}

class SlicePlaybackEngineTests final : public juce::UnitTest
{
public:
    SlicePlaybackEngineTests()
        : UnitTest("Slice playback engine", "audio")
    {
    }

    void runTest() override
    {
        beginTest("Mono playback is duplicated to stereo and advances");
        {
            auto sample = makeConstantSample(1, 2048, 48000.0, 0.75f);
            saucechop::SlicePlaybackEngine engine;
            engine.prepare(48000.0, 128);
            engine.setSource(sample.get());
            engine.setSliceCount(4);
            engine.start();

            juce::AudioBuffer<float> output(2, 128);
            engine.process(output, 1.0f);

            expect(engine.isPlaying());
            expect(output.getMagnitude(0, 0, 128) > 0.1f);
            expectWithinAbsoluteError(output.getSample(0, 100),
                                      output.getSample(1, 100),
                                      0.000001f);
            expectWithinAbsoluteError(static_cast<float>(engine.progress()),
                                      128.0f / 2048.0f,
                                      0.0001f);
            expectEquals(engine.currentSlice(), 0);
        }

        beginTest("Source-rate conversion consumes the expected frames");
        {
            auto sample = makeConstantSample(2, 1000, 48000.0);
            saucechop::SlicePlaybackEngine engine;
            engine.prepare(24000.0, 100);
            engine.setSource(sample.get());
            engine.start();

            juce::AudioBuffer<float> output(2, 100);
            engine.process(output, 1.0f);

            expectWithinAbsoluteError(static_cast<float>(engine.progress()), 0.2f, 0.0001f);
        }

        beginTest("Playback conversion is correct at common host sample rates");
        {
            for (const auto hostRate : {44100.0, 48000.0, 96000.0})
            {
                auto sample = makeConstantSample(1, 48000, 48000.0);
                saucechop::SlicePlaybackEngine engine;
                engine.prepare(hostRate, 256);
                engine.setSource(sample.get());
                engine.start();

                juce::AudioBuffer<float> output(2, 256);
                engine.process(output, 1.0f);

                const auto expectedProgress = static_cast<float>(256.0 / hostRate);
                expectWithinAbsoluteError(static_cast<float>(engine.progress()),
                                          expectedProgress,
                                          0.0001f);
            }
        }

        beginTest("Playback reports the active equal slice");
        {
            auto sample = makeConstantSample(1, 400, 1000.0);
            saucechop::SlicePlaybackEngine engine;
            engine.prepare(1000.0, 110);
            engine.setSource(sample.get());
            engine.setSliceCount(4);
            engine.start();

            juce::AudioBuffer<float> output(2, 110);
            engine.process(output, 1.0f);
            expectEquals(engine.currentSlice(), 1);
        }

        beginTest("Uneven slice boundaries match the model convention");
        {
            auto sample = makeConstantSample(1, 10, 1000.0);
            saucechop::SlicePlaybackEngine engine;
            engine.prepare(1000.0, 2);
            engine.setSource(sample.get());
            engine.setSliceCount(4);
            engine.start();

            juce::AudioBuffer<float> output(2, 2);
            engine.process(output, 1.0f);
            expectEquals(engine.currentSlice(), 1);
        }

        beginTest("A reordered sequence jumps between source slices");
        {
            auto sample = makeConstantSample(1, 400, 1000.0);

            for (int frame = 0; frame < 400; ++frame)
                sample->audio.setSample(0, frame, 0.25f * static_cast<float>(frame / 100 + 1));

            constexpr int reordered[]{2, 0, 3, 1};
            saucechop::SlicePlaybackEngine engine;
            engine.prepare(1000.0, 100);
            engine.setSource(sample.get());
            engine.setSliceCount(4);
            engine.setSequenceOrder(reordered, 4);
            engine.start();

            juce::AudioBuffer<float> output(2, 10);
            engine.process(output, 1.0f);
            expectEquals(engine.currentSlice(), 2);
            expectEquals(engine.currentSequenceStep(), 0);
            expectWithinAbsoluteError(static_cast<float>(engine.progress()), 0.525f, 0.0001f);
            expect(output.getSample(0, 8) > 0.6f);

            output.setSize(2, 100, false, false, true);
            engine.process(output, 1.0f);
            expectEquals(engine.currentSlice(), 0);
            expectEquals(engine.currentSequenceStep(), 1);
            expectWithinAbsoluteError(static_cast<float>(engine.progress()), 0.025f, 0.0001f);
        }

        beginTest("Stop uses a short fade and reaches silence");
        {
            auto sample = makeConstantSample(1, 1000, 1000.0);
            saucechop::SlicePlaybackEngine engine;
            engine.prepare(1000.0, 32);
            engine.setSource(sample.get());
            engine.start();

            juce::AudioBuffer<float> output(2, 16);
            engine.process(output, 1.0f);
            engine.requestStop();
            engine.process(output, 1.0f);

            expect(!engine.isPlaying());
            expectWithinAbsoluteError(output.getSample(0, 15), 0.0f, 0.000001f);
        }

        beginTest("Natural completion stops and keeps progress at the end");
        {
            auto sample = makeConstantSample(1, 8, 1000.0);
            saucechop::SlicePlaybackEngine engine;
            engine.prepare(1000.0, 16);
            engine.setSource(sample.get());
            engine.start();

            juce::AudioBuffer<float> output(2, 16);
            engine.process(output, 1.0f);

            expect(!engine.isPlaying());
            expectWithinAbsoluteError(static_cast<float>(engine.progress()), 1.0f, 0.000001f);

            for (int frame = 0; frame < output.getNumSamples(); ++frame)
                expect(std::isfinite(output.getSample(0, frame)));
        }
    }
};

SlicePlaybackEngineTests slicePlaybackEngineTests;
} // namespace
