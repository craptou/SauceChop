#pragma once

#include "PluginProcessor.h"
#include "ui/WaveformView.h"

#include <juce_gui_basics/juce_gui_basics.h>

class SauceChopAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                           private juce::ChangeListener,
                                           private juce::FileDragAndDropTarget
{
public:
    explicit SauceChopAudioProcessorEditor(SauceChopAudioProcessor& processor);
    ~SauceChopAudioProcessorEditor() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void chooseSample();
    void refreshSampleState();
    void updateSliceCount();
    [[nodiscard]] static bool isSupportedAudioFile(const juce::String& path);

    SauceChopAudioProcessor& processor;

    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label sampleInfoLabel;
    juce::Label outputGainLabel;
    juce::Label sliceCountLabel;
    juce::Slider outputGainSlider;
    juce::ComboBox sliceCountBox;
    juce::TextButton loadSampleButton{"Load sample"};
    WaveformView waveformView;
    std::unique_ptr<juce::FileChooser> fileChooser;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAttachment> outputGainAttachment;
    std::unique_ptr<ComboBoxAttachment> sliceCountAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SauceChopAudioProcessorEditor)
};
