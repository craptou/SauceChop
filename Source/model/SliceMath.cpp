#include "SliceMath.h"

namespace saucechop
{
std::vector<SliceRange> createEqualSlices(const std::int64_t totalFrames,
                                          const int sliceCount)
{
    if (totalFrames <= 0 || sliceCount <= 0)
        return {};

    std::vector<SliceRange> slices;
    slices.reserve(static_cast<std::size_t>(sliceCount));

    const auto count = static_cast<std::int64_t>(sliceCount);
    const auto quotient = totalFrames / count;
    const auto remainder = totalFrames % count;

    const auto boundary = [quotient, remainder, count](const std::int64_t index)
    {
        return index * quotient + (index * remainder) / count;
    };

    for (std::int64_t index = 0; index < count; ++index)
        slices.push_back({boundary(index), boundary(index + 1)});

    return slices;
}
} // namespace saucechop
