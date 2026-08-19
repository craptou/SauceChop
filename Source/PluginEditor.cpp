#include "PluginEditor.h"

#include "Parameters.h"

#include <algorithm>

namespace
{
const auto backgroundColour = juce::Colour::fromRGB(17, 19, 24);
const auto panelColour = juce::Colour::fromRGB(28, 32, 39);
const auto accentColour = juce::Colour::fromRGB(255, 116, 69);
const auto secondaryTextColour = juce::Colour::fromRGB(157, 164, 177);

juce::String formatDuration(const double seconds)
{
    const auto wholeSeconds = juce::roundToInt(seconds);
    return juce::String{wholeSeconds / 60} + ":"
        + juce::String{wholeSeconds % 60}.paddedLeft('0', 2);
}
} // namespace

SauceChopAudioProcessorEditor::SauceChopAudioProcessorEditor(
    SauceChopAudioProcessor& audioProcessor)
    : AudioProcessorEditor(&audioProcessor), processor(audioProcessor)
{
    titleLabel.setText("SauceChop", juce::dontSendNotification);
    titleLabel.setFont(juce::Font{juce::FontOptions{32.0f, juce::Font::bold}});
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    statusLabel.setJustificationType(juce::Justification::centredRight);
    statusLabel.setColour(juce::Label::textColourId, secondaryTextColour);
    addAndMakeVisible(statusLabel);

    sampleInfoLabel.setColour(juce::Label::textColourId, secondaryTextColour);
    sampleInfoLabel.setFont(juce::Font{juce::FontOptions{14.0f}});
    addAndMakeVisible(sampleInfoLabel);

    loadSampleButton.setColour(juce::TextButton::buttonColourId, accentColour);
    loadSampleButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loadSampleButton.onClick = [this]
    {
        chooseSample();
    };
    addAndMakeVisible(loadSampleButton);

    outputGainLabel.setText("OUTPUT", juce::dontSendNotification);
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, secondaryTextColour);
    addAndMakeVisible(outputGainLabel);

    outputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 24);
    outputGainSlider.setTextValueSuffix(" dB");
    outputGainSlider.setColour(juce::Slider::rotarySliderFillColourId, accentColour);
    outputGainSlider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
    outputGainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    outputGainSlider.setColour(juce::Slider::textBoxOutlineColourId,
                               juce::Colours::transparentBlack);
    addAndMakeVisible(outputGainSlider);

    sliceCountLabel.setText("SLICES", juce::dontSendNotification);
    sliceCountLabel.setJustificationType(juce::Justification::centred);
    sliceCountLabel.setColour(juce::Label::textColourId, secondaryTextColour);
    addAndMakeVisible(sliceCountLabel);

    sliceCountBox.addItemList({"4", "8", "16", "32"}, 1);
    sliceCountBox.setJustificationType(juce::Justification::centred);
    sliceCountBox.onChange = [this]
    {
        updateSliceCount();
    };
    addAndMakeVisible(sliceCountBox);

    addAndMakeVisible(waveformView);

    outputGainAttachment = std::make_unique<SliderAttachment>(
        processor.parameters(), saucechop::parameters::outputGain, outputGainSlider);
    sliceCountAttachment = std::make_unique<ComboBoxAttachment>(
        processor.parameters(), saucechop::parameters::sliceCount, sliceCountBox);

    processor.addChangeListener(this);
    refreshSampleState();
    updateSliceCount();

    setResizable(true, true);
    setResizeLimits(680, 460, 1400, 900);
    setSize(900, 560);
}

SauceChopAudioProcessorEditor::~SauceChopAudioProcessorEditor()
{
    processor.removeChangeListener(this);
    fileChooser.reset();
}

void SauceChopAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(backgroundColour);

    auto footer = getLocalBounds().reduced(28).removeFromBottom(136).toFloat();
    graphics.setColour(panelColour);
    graphics.fillRoundedRectangle(footer, 12.0f);
}

void SauceChopAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(28);
    auto header = area.removeFromTop(42);
    titleLabel.setBounds(header.removeFromLeft(250));
    loadSampleButton.setBounds(header.removeFromRight(124).reduced(0, 3));
    header.removeFromRight(12);
    statusLabel.setBounds(header);

    area.removeFromTop(8);
    sampleInfoLabel.setBounds(area.removeFromTop(24));
    area.removeFromTop(8);

    auto footer = area.removeFromBottom(136);
    area.removeFromBottom(16);
    waveformView.setBounds(area);

    auto controls = footer.withSizeKeepingCentre(300, footer.getHeight());
    auto gainArea = controls.removeFromLeft(140).reduced(10, 6);
    outputGainLabel.setBounds(gainArea.removeFromTop(22));
    outputGainSlider.setBounds(gainArea);

    controls.removeFromLeft(20);
    auto sliceArea = controls.reduced(10, 28);
    sliceCountLabel.setBounds(sliceArea.removeFromTop(24));
    sliceArea.removeFromTop(8);
    sliceCountBox.setBounds(sliceArea.removeFromTop(32));
}

void SauceChopAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &processor)
        refreshSampleState();
}

bool SauceChopAudioProcessorEditor::isInterestedInFileDrag(const juce::StringArray& files)
{
    return std::any_of(files.begin(), files.end(), isSupportedAudioFile);
}

void SauceChopAudioProcessorEditor::fileDragEnter(const juce::StringArray& files,
                                                  const int x,
                                                  const int y)
{
    juce::ignoreUnused(x, y);
    waveformView.setDropTargetActive(isInterestedInFileDrag(files));
}

void SauceChopAudioProcessorEditor::fileDragExit(const juce::StringArray& files)
{
    juce::ignoreUnused(files);
    waveformView.setDropTargetActive(false);
}

void SauceChopAudioProcessorEditor::filesDropped(const juce::StringArray& files,
                                                 const int x,
                                                 const int y)
{
    juce::ignoreUnused(x, y);
    waveformView.setDropTargetActive(false);

    const auto match = std::find_if(files.begin(), files.end(), isSupportedAudioFile);

    if (match != files.end())
        processor.loadSampleAsync(juce::File{*match});
}

void SauceChopAudioProcessorEditor::chooseSample()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Choose a sample", juce::File{}, "*.wav;*.aif;*.aiff;*.mp3");

    const auto safeThis = juce::Component::SafePointer<SauceChopAudioProcessorEditor>{this};
    fileChooser->launchAsync(juce::FileBrowserComponent::openMode
                                 | juce::FileBrowserComponent::canSelectFiles,
                             [safeThis](const juce::FileChooser& chooser)
                             {
                                 if (safeThis == nullptr)
                                     return;

                                 const auto selectedFile = chooser.getResult();

                                 if (selectedFile.existsAsFile())
                                     safeThis->processor.loadSampleAsync(selectedFile);
                             });
}

void SauceChopAudioProcessorEditor::refreshSampleState()
{
    const auto sample = processor.sourceSampleSnapshot();
    waveformView.setSample(sample);

    switch (processor.sampleLoadState())
    {
        case SauceChopAudioProcessor::SampleLoadState::empty:
            statusLabel.setText("Ready for a sample", juce::dontSendNotification);
            sampleInfoLabel.setText("WAV, AIFF or MP3 • mono or stereo • maximum 10 minutes",
                                    juce::dontSendNotification);
            break;

        case SauceChopAudioProcessor::SampleLoadState::loading:
            statusLabel.setText(processor.sampleLoadMessage(), juce::dontSendNotification);
            break;

        case SauceChopAudioProcessor::SampleLoadState::ready:
            statusLabel.setText("Sample ready", juce::dontSendNotification);

            if (sample != nullptr)
            {
                const auto channelText = sample->channelCount == 1 ? "mono" : "stereo";
                const auto sampleRateKhz = sample->originalSampleRateHz / 1000.0;
                sampleInfoLabel.setText(sample->sourceFile.getFileName() + "  •  "
                                            + formatDuration(sample->durationSeconds()) + "  •  "
                                            + juce::String{sampleRateKhz, 1} + " kHz  •  "
                                            + channelText,
                                        juce::dontSendNotification);
            }
            break;

        case SauceChopAudioProcessor::SampleLoadState::error:
            statusLabel.setText("Load failed", juce::dontSendNotification);
            sampleInfoLabel.setText(processor.sampleLoadMessage(), juce::dontSendNotification);
            break;
    }

    loadSampleButton.setEnabled(processor.sampleLoadState()
                                != SauceChopAudioProcessor::SampleLoadState::loading);
}

void SauceChopAudioProcessorEditor::updateSliceCount()
{
    const auto selectedText = sliceCountBox.getText();
    waveformView.setSliceCount(selectedText.isNotEmpty() ? selectedText.getIntValue() : 16);
}

bool SauceChopAudioProcessorEditor::isSupportedAudioFile(const juce::String& path)
{
    const auto extension = juce::File{path}.getFileExtension().toLowerCase();
    return extension == ".wav" || extension == ".aif" || extension == ".aiff"
        || extension == ".mp3";
}
