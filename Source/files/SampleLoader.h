#pragma once

#include "SourceSample.h"

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

#include <atomic>
#include <functional>
#include <memory>
#include <optional>

namespace saucechop
{
class SampleLoader final : private juce::AsyncUpdater
{
public:
    using Callback = std::function<void(SampleLoadResult)>;

    SampleLoader();
    ~SampleLoader() override;

    void load(const juce::File& file, Callback callback);

private:
    class LoadJob;

    struct PendingCompletion
    {
        std::uint64_t requestId = 0;
        Callback callback;
        SampleLoadResult result;
    };

    void publish(std::uint64_t requestId, Callback callback, SampleLoadResult result);
    void handleAsyncUpdate() override;

    juce::ThreadPool threadPool{1};
    juce::CriticalSection completionLock;
    std::optional<PendingCompletion> pendingCompletion;
    std::atomic<std::uint64_t> latestRequestId{0};
    std::atomic<bool> shuttingDown{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleLoader)
};
} // namespace saucechop
