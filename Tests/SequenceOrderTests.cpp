#include "../Source/model/SequenceOrder.h"

#include <juce_core/juce_core.h>

namespace
{
class SequenceOrderTests final : public juce::UnitTest
{
public:
    SequenceOrderTests()
        : UnitTest("Sequence order", "model")
    {
    }

    void runTest() override
    {
        beginTest("Identity order contains every slice once");
        const auto identity = saucechop::makeIdentitySequence(4);
        expect(identity == std::vector<int>({0, 1, 2, 3}));
        expect(saucechop::isValidSequence(identity, 4));

        beginTest("Moving an item preserves a valid permutation");
        const auto moved = saucechop::moveSequenceItem(identity, 1, 3);
        expect(moved == std::vector<int>({0, 2, 3, 1}));
        expect(saucechop::isValidSequence(moved, 4));

        beginTest("Invalid moves leave the order unchanged");
        expect(saucechop::moveSequenceItem(identity, -1, 2) == identity);
        expect(saucechop::moveSequenceItem(identity, 0, 9) == identity);

        beginTest("Duplicate, missing and out-of-range slices are rejected");
        expect(!saucechop::isValidSequence({0, 1, 1, 3}, 4));
        expect(!saucechop::isValidSequence({0, 1, 2}, 4));
        expect(!saucechop::isValidSequence({0, 1, 2, 4}, 4));

        beginTest("A valid order survives state serialisation");
        const auto encoded = saucechop::serialiseSequence(moved);
        expectEquals(encoded, juce::String{"0,2,3,1"});
        expect(saucechop::parseSequence(encoded, 4) == moved);
        expect(saucechop::parseSequence("0,2,nope,1", 4).empty());
    }
};

SequenceOrderTests sequenceOrderTests;
} // namespace
