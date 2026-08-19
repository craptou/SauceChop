#include "SampleLoader.h"

#include "SampleFileReader.h"

#include <utility>

namespace saucechop
{
class SampleLoader::LoadJob final : public juce::ThreadPoolJob
{
public:
    LoadJob(SampleLoader& loaderToUse,
            juce::File fileToLoad,
            const std::uint64_t requestToUse,
            Callback callbackToUse)
        : ThreadPoolJob("Load sample"),
          loader(loaderToUse),
          file(std::move(fileToLoad)),
          requestId(requestToUse),
          callback(std::move(callbackToUse))
    {
    }

    JobStatus runJob() override
    {
        auto result = decodeSampleFile(file,
                                       [this]
                                       {
                                           return shouldExit();
                                       });

        if (!shouldExit())
            loader.publish(requestId, std::move(callback), std::move(result));

        return jobHasFinished;
    }

private:
    SampleLoader& loader;
    juce::File file;
    std::uint64_t requestId = 0;
    Callback callback;
};

SampleLoader::SampleLoader() = default;

SampleLoader::~SampleLoader()
{
    shuttingDown.store(true);
    latestRequestId.fetch_add(1);
    threadPool.removeAllJobs(true, -1);
    cancelPendingUpdate();

    const juce::ScopedLock lock(completionLock);
    pendingCompletion.reset();
}

void SampleLoader::load(const juce::File& file, Callback callback)
{
    const auto requestId = latestRequestId.fetch_add(1) + 1;
    threadPool.removeAllJobs(true, 0);
    threadPool.addJob(new LoadJob(*this, file, requestId, std::move(callback)), true);
}

void SampleLoader::publish(const std::uint64_t requestId,
                           Callback callback,
                           SampleLoadResult result)
{
    if (shuttingDown.load() || requestId != latestRequestId.load())
        return;

    {
        const juce::ScopedLock lock(completionLock);
        pendingCompletion = PendingCompletion{requestId, std::move(callback), std::move(result)};
    }

    triggerAsyncUpdate();
}

void SampleLoader::handleAsyncUpdate()
{
    std::optional<PendingCompletion> completion;

    {
        const juce::ScopedLock lock(completionLock);
        completion = std::move(pendingCompletion);
        pendingCompletion.reset();
    }

    if (completion.has_value() && !shuttingDown.load()
        && completion->requestId == latestRequestId.load() && completion->callback)
    {
        completion->callback(std::move(completion->result));
    }
}
} // namespace saucechop
