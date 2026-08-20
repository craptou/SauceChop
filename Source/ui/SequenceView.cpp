#include "SequenceView.h"

#include "../model/SequenceOrder.h"

#include <algorithm>

namespace
{
const auto panelColour = juce::Colour::fromRGB(28, 32, 39);
const auto cellColour = juce::Colour::fromRGB(48, 54, 64);
const auto accentColour = juce::Colour::fromRGB(255, 116, 69);
const auto activeColour = juce::Colour::fromRGB(87, 201, 151);
} // namespace

void SequenceView::setOrder(std::vector<int> newOrder)
{
    const auto selectedSlice = selectedIndex >= 0
            && selectedIndex < static_cast<int>(order.size())
        ? order[static_cast<std::size_t>(selectedIndex)]
        : -1;

    order = std::move(newOrder);
    const auto match = std::find(order.begin(), order.end(), selectedSlice);
    selectedIndex = match != order.end()
        ? static_cast<int>(std::distance(order.begin(), match))
        : -1;
    repaint();
}

void SequenceView::setActiveStep(const int newActiveStep)
{
    const auto sanitisedStep = newActiveStep >= 0
            && newActiveStep < static_cast<int>(order.size())
        ? newActiveStep
        : -1;

    if (activeStep == sanitisedStep)
        return;

    activeStep = sanitisedStep;
    repaint();
}

void SequenceView::paint(juce::Graphics& graphics)
{
    const auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    graphics.setColour(panelColour);
    graphics.fillRoundedRectangle(bounds, 8.0f);

    if (order.empty())
        return;

    for (int index = 0; index < static_cast<int>(order.size()); ++index)
    {
        const auto cell = cellBounds(index);
        const auto isActive = index == activeStep;
        const auto isSelected = index == selectedIndex;

        graphics.setColour(isActive ? activeColour.withAlpha(0.72f) : cellColour);
        graphics.fillRoundedRectangle(cell, 4.0f);

        if (isSelected)
        {
            graphics.setColour(accentColour);
            graphics.drawRoundedRectangle(cell.reduced(0.5f), 4.0f, 2.0f);
        }

        graphics.setColour(juce::Colours::white.withAlpha(isActive ? 1.0f : 0.86f));
        graphics.setFont(juce::jlimit(9.0f, 14.0f, cell.getWidth() * 0.42f));
        graphics.drawText(juce::String{order[static_cast<std::size_t>(index)] + 1}
                              .paddedLeft('0', 2),
                          cell.toNearestInt(),
                          juce::Justification::centred,
                          false);
    }

    if (dragging && dragTargetIndex >= 0)
    {
        graphics.setColour(accentColour.withAlpha(0.9f));
        graphics.drawRoundedRectangle(cellBounds(dragTargetIndex).expanded(1.0f),
                                      4.0f,
                                      2.0f);
    }
}

void SequenceView::mouseDown(const juce::MouseEvent& event)
{
    selectedIndex = indexAt(event.position.x);
    dragSourceIndex = selectedIndex;
    dragTargetIndex = selectedIndex;
    dragging = false;

    if (selectedIndex >= 0 && onSelectionChanged)
        onSelectionChanged(order[static_cast<std::size_t>(selectedIndex)]);

    repaint();
}

void SequenceView::mouseDrag(const juce::MouseEvent& event)
{
    if (dragSourceIndex < 0 || event.getDistanceFromDragStart() < 4)
        return;

    dragging = true;
    dragTargetIndex = indexAt(event.position.x);
    repaint();
}

void SequenceView::mouseUp(const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);

    if (dragging && dragSourceIndex >= 0 && dragTargetIndex >= 0
        && dragSourceIndex != dragTargetIndex)
    {
        const auto fromIndex = dragSourceIndex;
        const auto toIndex = dragTargetIndex;
        order = saucechop::moveSequenceItem(order, fromIndex, toIndex);
        selectedIndex = toIndex;

        if (onOrderChanged)
            onOrderChanged(fromIndex, toIndex);

        if (onSelectionChanged)
            onSelectionChanged(order[static_cast<std::size_t>(selectedIndex)]);
    }

    dragging = false;
    dragSourceIndex = -1;
    dragTargetIndex = -1;
    repaint();
}

int SequenceView::indexAt(const float x) const noexcept
{
    if (order.empty() || x < 0.0f || x > static_cast<float>(getWidth()))
        return -1;

    return juce::jlimit(0,
                        static_cast<int>(order.size()) - 1,
                        static_cast<int>(x * static_cast<float>(order.size())
                                         / juce::jmax(1.0f, static_cast<float>(getWidth()))));
}

juce::Rectangle<float> SequenceView::cellBounds(const int index) const noexcept
{
    const auto count = juce::jmax(1, static_cast<int>(order.size()));
    const auto width = static_cast<float>(getWidth()) / static_cast<float>(count);
    return {static_cast<float>(index) * width + 2.0f,
            5.0f,
            juce::jmax(1.0f, width - 4.0f),
            juce::jmax(1.0f, static_cast<float>(getHeight()) - 10.0f)};
}
