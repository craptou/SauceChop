#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

class SequenceView final : public juce::Component
{
public:
    void setOrder(std::vector<int> newOrder);
    void setActiveStep(int newActiveStep);

    std::function<void(int sourceSlice)> onSelectionChanged;
    std::function<void(int fromIndex, int toIndex)> onOrderChanged;

    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

private:
    [[nodiscard]] int indexAt(float x) const noexcept;
    [[nodiscard]] juce::Rectangle<float> cellBounds(int index) const noexcept;

    std::vector<int> order;
    int selectedIndex = -1;
    int activeStep = -1;
    int dragSourceIndex = -1;
    int dragTargetIndex = -1;
    bool dragging = false;
};
