#pragma once

#include <juce_core/juce_core.h>

#include <vector>

namespace saucechop
{
[[nodiscard]] std::vector<int> makeIdentitySequence(int sliceCount);
[[nodiscard]] bool isValidSequence(const std::vector<int>& order, int sliceCount);
[[nodiscard]] std::vector<int> moveSequenceItem(const std::vector<int>& order,
                                                int fromIndex,
                                                int toIndex);
[[nodiscard]] juce::String serialiseSequence(const std::vector<int>& order);
[[nodiscard]] std::vector<int> parseSequence(const juce::String& text, int sliceCount);
} // namespace saucechop
