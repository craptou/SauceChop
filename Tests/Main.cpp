#include <juce_core/juce_core.h>

#include <iostream>

int main()
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure(false);
    runner.runAllTests();

    int failureCount = 0;

    for (int index = 0; index < runner.getNumResults(); ++index)
    {
        if (const auto* result = runner.getResult(index))
        {
            failureCount += result->failures;

            for (const auto& message : result->messages)
                std::cout << result->unitTestName << " / " << result->subcategoryName
                          << ": " << message << '\n';
        }
    }

    return failureCount == 0 ? 0 : 1;
}
