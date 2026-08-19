#include "SampleFileReader.h"

#include "../model/WaveformPeaks.h"

#include <juce_audio_formats/juce_audio_formats.h>

#include <limits>
#include <new>

namespace saucechop
{
namespace
{
constexpr auto maximumDurationSeconds = 10.0 * 60.0;
constexpr auto maximumDecodedBytes = std::int64_t{1024} * 1024 * 1024;
constexpr auto waveformBinCount = 2048;

bool hasSupportedExtension(const juce::File& file)
{
    const auto extension = file.getFileExtension().toLowerCase();
    return extension == ".wav" || extension == ".aif" || extension == ".aiff";
}
} // namespace

SampleLoadResult decodeSampleFile(const juce::File& file,
                                  SampleLoadCancellationCheck shouldCancel)
{
    const auto isCancelled = [&shouldCancel]
    {
        return shouldCancel && shouldCancel();
    };

    if (isCancelled())
        return {};

    if (!file.existsAsFile())
        return {nullptr, "The selected audio file does not exist."};

    if (!hasSupportedExtension(file))
        return {nullptr, "Unsupported audio format. Use WAV or AIFF."};

    try
    {
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

        if (reader == nullptr)
            return {nullptr, "The audio file could not be decoded."};

        if (reader->numChannels == 0 || reader->numChannels > 2)
            return {nullptr, "Only mono and stereo audio files are supported."};

        if (reader->sampleRate <= 0.0 || reader->lengthInSamples <= 0)
            return {nullptr, "The audio file contains no readable samples."};

        if (reader->lengthInSamples > std::numeric_limits<int>::max())
            return {nullptr, "This audio file is too long to load safely."};

        const auto duration = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;

        if (duration > maximumDurationSeconds)
            return {nullptr, "This audio file exceeds the 10 minute limit."};

        const auto decodedBytes = static_cast<long double>(reader->lengthInSamples)
            * static_cast<long double>(reader->numChannels) * sizeof(float);

        if (decodedBytes > static_cast<long double>(maximumDecodedBytes))
            return {nullptr, "This audio file exceeds the 1 GiB decoded memory limit."};

        if (isCancelled())
            return {};

        auto source = std::make_shared<SourceSample>();
        source->sourceFile = file;
        source->originalSampleRateHz = reader->sampleRate;
        source->frameCount = reader->lengthInSamples;
        source->channelCount = static_cast<int>(reader->numChannels);
        source->fileSizeBytes = file.getSize();
        source->modificationTime = file.getLastModificationTime();
        source->audio.setSize(source->channelCount,
                              static_cast<int>(source->frameCount),
                              false,
                              true,
                              false);

        if (!reader->read(&source->audio,
                          0,
                          static_cast<int>(source->frameCount),
                          0,
                          true,
                          true))
        {
            return {nullptr, "The audio file could not be read completely."};
        }

        if (isCancelled())
            return {};

        source->waveformPeaks = createWaveformPeaks(source->audio, waveformBinCount);
        return {std::move(source), {}};
    }
    catch (const std::bad_alloc&)
    {
        return {nullptr, "There is not enough memory to load this audio file."};
    }
    catch (...)
    {
        return {nullptr, "An unexpected error occurred while loading the audio file."};
    }
}
} // namespace saucechop
