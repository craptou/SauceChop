#include "../Source/files/SampleFileReader.h"

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_core/juce_core.h>

namespace
{
class TemporaryTestFile final
{
public:
    explicit TemporaryTestFile(const juce::String& extension)
        : file(juce::File::getSpecialLocation(juce::File::tempDirectory)
                   .getNonexistentChildFile("SauceChop-test", extension, false))
    {
    }

    ~TemporaryTestFile()
    {
        file.deleteFile();
    }

    juce::File file;
};

class SampleFileReaderTests final : public juce::UnitTest
{
public:
    SampleFileReaderTests()
        : UnitTest("Sample file decoding", "files")
    {
    }

    void runTest() override
    {
        beginTest("A mono WAV is decoded with metadata and waveform peaks");
        TemporaryTestFile temporaryWav{".wav"};
        juce::AudioBuffer<float> expectedAudio{1, 128};

        for (int sample = 0; sample < expectedAudio.getNumSamples(); ++sample)
            expectedAudio.setSample(0, sample, static_cast<float>(sample) / 127.0f * 2.0f - 1.0f);

        juce::WavAudioFormat wavFormat;
        std::unique_ptr<juce::OutputStream> output(temporaryWav.file.createOutputStream());
        const auto options = juce::AudioFormatWriterOptions{}
                                 .withSampleRate(48'000.0)
                                 .withNumChannels(1)
                                 .withBitsPerSample(16);
        auto writer = wavFormat.createWriterFor(output, options);
        expect(writer != nullptr);

        if (writer != nullptr)
            expect(writer->writeFromAudioSampleBuffer(expectedAudio, 0, expectedAudio.getNumSamples()));

        writer.reset();
        output.reset();

        const auto result = saucechop::decodeSampleFile(temporaryWav.file);
        expect(result.succeeded(), result.errorMessage);

        if (result.sample != nullptr)
        {
            expectEquals(result.sample->channelCount, 1);
            expectEquals(result.sample->frameCount, std::int64_t{128});
            expectWithinAbsoluteError(result.sample->originalSampleRateHz, 48'000.0, 0.01);
            expectEquals(result.sample->audio.getNumSamples(), 128);
            expect(!result.sample->waveformPeaks.empty());
            expect(result.sample->fileSizeBytes > 0);
        }

        beginTest("A stereo AIFF is decoded");
        TemporaryTestFile temporaryAiff{".aiff"};
        juce::AudioBuffer<float> stereoAudio{2, 64};
        stereoAudio.clear();
        stereoAudio.setSample(0, 8, 0.75f);
        stereoAudio.setSample(1, 24, -0.5f);

        juce::AiffAudioFormat aiffFormat;
        std::unique_ptr<juce::OutputStream> aiffOutput(temporaryAiff.file.createOutputStream());
        const auto aiffOptions = juce::AudioFormatWriterOptions{}
                                     .withSampleRate(44'100.0)
                                     .withNumChannels(2)
                                     .withBitsPerSample(16);
        auto aiffWriter = aiffFormat.createWriterFor(aiffOutput, aiffOptions);
        expect(aiffWriter != nullptr);

        if (aiffWriter != nullptr)
            expect(aiffWriter->writeFromAudioSampleBuffer(stereoAudio, 0, 64));

        aiffWriter.reset();
        aiffOutput.reset();

        const auto aiffResult = saucechop::decodeSampleFile(temporaryAiff.file);
        expect(aiffResult.succeeded(), aiffResult.errorMessage);

        if (aiffResult.sample != nullptr)
        {
            expectEquals(aiffResult.sample->channelCount, 2);
            expectEquals(aiffResult.sample->frameCount, std::int64_t{64});
            expectWithinAbsoluteError(aiffResult.sample->originalSampleRateHz, 44'100.0, 0.01);
        }

        beginTest("An MP3 is decoded through the native platform codec");
        const auto mp3File = juce::File{SAUCECHOP_TEST_MP3_PATH};
        expect(mp3File.existsAsFile());
        const auto mp3Result = saucechop::decodeSampleFile(mp3File);
        expect(mp3Result.succeeded(), mp3Result.errorMessage);

        if (mp3Result.sample != nullptr)
        {
            expect(mp3Result.sample->channelCount == 1 || mp3Result.sample->channelCount == 2);
            expect(mp3Result.sample->frameCount > 0);
            expect(mp3Result.sample->originalSampleRateHz > 0.0);
            expect(!mp3Result.sample->waveformPeaks.empty());
        }

        beginTest("Unsupported extensions fail without decoding");
        TemporaryTestFile temporaryText{".txt"};
        expect(temporaryText.file.replaceWithText("not audio"));
        const auto unsupported = saucechop::decodeSampleFile(temporaryText.file);
        expect(!unsupported.succeeded());
        expect(unsupported.errorMessage.containsIgnoreCase("unsupported"));

        beginTest("Cancellation stops before file access");
        const auto cancelled = saucechop::decodeSampleFile(
            juce::File{"file-that-does-not-exist.wav"},
            []
            {
                return true;
            });
        expect(!cancelled.succeeded());
        expect(cancelled.errorMessage.isEmpty());
    }
};

SampleFileReaderTests sampleFileReaderTests;
} // namespace
