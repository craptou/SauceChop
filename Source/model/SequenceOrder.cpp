#include "SequenceOrder.h"

#include <algorithm>
#include <numeric>

namespace saucechop
{
std::vector<int> makeIdentitySequence(const int sliceCount)
{
    if (sliceCount <= 0)
        return {};

    std::vector<int> order(static_cast<std::size_t>(sliceCount));
    std::iota(order.begin(), order.end(), 0);
    return order;
}

bool isValidSequence(const std::vector<int>& order, const int sliceCount)
{
    if (sliceCount <= 0 || static_cast<int>(order.size()) != sliceCount)
        return false;

    std::vector<bool> found(static_cast<std::size_t>(sliceCount), false);

    for (const auto slice : order)
    {
        if (slice < 0 || slice >= sliceCount || found[static_cast<std::size_t>(slice)])
            return false;

        found[static_cast<std::size_t>(slice)] = true;
    }

    return true;
}

std::vector<int> moveSequenceItem(const std::vector<int>& order,
                                  const int fromIndex,
                                  const int toIndex)
{
    if (fromIndex < 0 || fromIndex >= static_cast<int>(order.size()) || toIndex < 0
        || toIndex >= static_cast<int>(order.size()) || fromIndex == toIndex)
    {
        return order;
    }

    auto result = order;
    const auto movedSlice = result[static_cast<std::size_t>(fromIndex)];
    result.erase(result.begin() + fromIndex);
    result.insert(result.begin() + toIndex, movedSlice);
    return result;
}

juce::String serialiseSequence(const std::vector<int>& order)
{
    juce::StringArray values;

    for (const auto slice : order)
        values.add(juce::String{slice});

    return values.joinIntoString(",");
}

std::vector<int> parseSequence(const juce::String& text, const int sliceCount)
{
    juce::StringArray values;
    values.addTokens(text, ",", {});
    values.trim();
    values.removeEmptyStrings();

    std::vector<int> result;
    result.reserve(static_cast<std::size_t>(juce::jmax(0, sliceCount)));

    for (const auto& value : values)
    {
        if (!value.containsOnly("0123456789"))
            return {};

        result.push_back(value.getIntValue());
    }

    return isValidSequence(result, sliceCount) ? result : std::vector<int>{};
}
} // namespace saucechop
