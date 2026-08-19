#pragma once

#include "SourceSample.h"

#include <functional>

namespace saucechop
{
using SampleLoadCancellationCheck = std::function<bool()>;

[[nodiscard]] SampleLoadResult
decodeSampleFile(const juce::File& file,
                 SampleLoadCancellationCheck shouldCancel = SampleLoadCancellationCheck{});
} // namespace saucechop
