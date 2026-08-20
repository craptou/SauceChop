#pragma once

#include "PluginProcessor.h"
#include "ui/SequenceView.h"
#include "ui/WaveformView.h"

#include <juce_gui_basics/juce_gui_basics.h>

class SauceChopAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                           private juce::ChangeListener,
                                           private juce::FileDragAndDropTarget,
                                           private juce::Timer
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
    void timerCallback() override;

    void chooseSample();
    void refreshSampleState();
    void updateSliceCount();
    void togglePlayback();
    void refreshSequenceOrder();
    [[nodiscard]] static bool isSupportedAudioFile(const juce::String& path);

    SauceChopAudioProcessor& processor;

    juce::Label titleLabel;
    juce::Label statusLabel;
    juce::Label sampleInfoLabel;
    juce::Label outputGainLabel;
    juce::Label sliceCountLabel;
    juce::Label midiBaseNoteLabel;
    juce::Label midiPlayModeLabel;
    juce::Label sequenceLabel;
    juce::Slider outputGainSlider;
    juce::Slider midiBaseNoteSlider;
    juce::ComboBox sliceCountBox;
    juce::ComboBox midiPlayModeBox;
    juce::TextButton loadSampleButton{"Load sample"};
    juce::TextButton playbackButton{"Play"};
    juce::TextButton resetOrderButton{"Reset order"};
    WaveformView waveformView;
    SequenceView sequenceView;
    std::unique_ptr<juce::FileChooser> fileChooser;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAttachment> outputGainAttachment;
    std::unique_ptr<SliderAttachment> midiBaseNoteAttachment;
    std::unique_ptr<ComboBoxAttachment> sliceCountAttachment;
    std::unique_ptr<ComboBoxAttachment> midiPlayModeAttachment;
    double playbackRequestStartMilliseconds = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SauceChopAudioProcessorEditor)
};
