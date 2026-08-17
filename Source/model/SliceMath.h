#pragma once

#include <compare>
#include <cstdint>
#include <vector>

namespace saucechop
{
struct SliceRange
{
    std::int64_t startSample = 0;
    std::int64_t endSample = 0;

    [[nodiscard]] std::int64_t length() const noexcept
    {
        return endSample - startSample;
    }

    auto operator<=>(const SliceRange&) const = default;
};

[[nodiscard]] std::vector<SliceRange> createEqualSlices(std::int64_t totalFrames,
                                                         int sliceCount);
} // namespace saucechop
