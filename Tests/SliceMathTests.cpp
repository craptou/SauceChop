#include "../Source/model/SliceMath.h"

#include <juce_core/juce_core.h>

namespace
{
class SliceMathTests final : public juce::UnitTest
{
public:
    SliceMathTests()
        : UnitTest("Equal slice boundaries", "model")
    {
    }

    void runTest() override
    {
        beginTest("Invalid inputs produce no slices");
        expect(saucechop::createEqualSlices(0, 16).empty());
        expect(saucechop::createEqualSlices(100, 0).empty());
        expect(saucechop::createEqualSlices(-1, 4).empty());

        beginTest("Ten frames are covered by four contiguous slices");
        const auto slices = saucechop::createEqualSlices(10, 4);

        expectEquals(static_cast<int>(slices.size()), 4);

        if (slices.size() == 4)
        {
            expect(slices[0] == saucechop::SliceRange{0, 2});
            expect(slices[1] == saucechop::SliceRange{2, 5});
            expect(slices[2] == saucechop::SliceRange{5, 7});
            expect(slices[3] == saucechop::SliceRange{7, 10});
        }

        beginTest("All supported slice counts cover the exact source range");

        for (const auto sliceCount : {4, 8, 16, 32})
        {
            const auto ranges = saucechop::createEqualSlices(48'001, sliceCount);
            expectEquals(static_cast<int>(ranges.size()), sliceCount);

            if (ranges.empty())
                continue;

            expectEquals(ranges.front().startSample, std::int64_t{0});
            expectEquals(ranges.back().endSample, std::int64_t{48'001});

            for (std::size_t index = 0; index < ranges.size(); ++index)
            {
                expect(ranges[index].startSample <= ranges[index].endSample);

                if (index > 0)
                    expectEquals(ranges[index - 1].endSample, ranges[index].startSample);
            }
        }

        beginTest("Boundary calculation does not overflow for long sources");
        constexpr auto longSource = std::int64_t{9'000'000'000'000'000};
        const auto longRanges = saucechop::createEqualSlices(longSource, 32);

        expect(!longRanges.empty());

        if (!longRanges.empty())
            expectEquals(longRanges.back().endSample, longSource);
    }
};

SliceMathTests sliceMathTests;
} // namespace
