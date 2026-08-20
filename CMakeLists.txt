cmake_minimum_required(VERSION 3.25)

project(SauceChop VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SAUCECHOP_BUILD_TESTS "Build SauceChop unit tests" ON)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE is missing. Run: git submodule update --init --recursive")
endif()

add_subdirectory(external/JUCE)

juce_add_plugin(SauceChop
    COMPANY_NAME "SauceChop"
    COMPANY_WEBSITE "https://github.com/craptou/SauceChop"
    BUNDLE_ID "com.craptou.saucechop"
    PLUGIN_MANUFACTURER_CODE Crpt
    PLUGIN_CODE SaCh
    FORMATS Standalone VST3
    PRODUCT_NAME "SauceChop"
    DESCRIPTION "Creative sample chopping instrument"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Sampler)

juce_generate_juce_header(SauceChop)

target_sources(SauceChop
    PRIVATE
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/Parameters.h
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/audio/SlicePlaybackEngine.cpp
        Source/audio/SlicePlaybackEngine.h
        Source/files/SampleLoader.cpp
        Source/files/SampleLoader.h
        Source/files/SampleFileReader.cpp
        Source/files/SampleFileReader.h
        Source/files/SourceSample.h
        Source/model/SliceMath.cpp
        Source/model/SliceMath.h
        Source/model/WaveformPeaks.cpp
        Source/model/WaveformPeaks.h
        Source/ui/WaveformView.cpp
        Source/ui/WaveformView.h)

target_compile_definitions(SauceChop
    PUBLIC
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(SauceChop
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_dsp
        juce::juce_events
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/Parameters.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/audio/SlicePlaybackEngine.cpp
    Source/audio/SlicePlaybackEngine.h
    Source/files/SampleLoader.cpp
    Source/files/SampleLoader.h
    Source/files/SampleFileReader.cpp
    Source/files/SampleFileReader.h
    Source/files/SourceSample.h
    Source/model/SliceMath.cpp
    Source/model/SliceMath.h
    Source/model/WaveformPeaks.cpp
    Source/model/WaveformPeaks.h
    Source/ui/WaveformView.cpp
    Source/ui/WaveformView.h)

if(SAUCECHOP_BUILD_TESTS)
    include(CTest)

    juce_add_console_app(SauceChopTests
        PRODUCT_NAME "SauceChop Tests")

    target_sources(SauceChopTests
        PRIVATE
            Tests/Main.cpp
            Tests/SlicePlaybackEngineTests.cpp
            Tests/SampleFileReaderTests.cpp
            Tests/SliceMathTests.cpp
            Tests/WaveformPeaksTests.cpp
            Source/files/SampleFileReader.cpp
            Source/files/SampleFileReader.h
            Source/files/SourceSample.h
            Source/audio/SlicePlaybackEngine.cpp
            Source/audio/SlicePlaybackEngine.h
            Source/model/SliceMath.cpp
            Source/model/SliceMath.h
            Source/model/WaveformPeaks.cpp
            Source/model/WaveformPeaks.h)

    target_link_libraries(SauceChopTests
        PRIVATE
            juce::juce_core
            juce::juce_audio_basics
            juce::juce_audio_formats
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_warning_flags)

    target_compile_definitions(SauceChopTests
        PRIVATE
            JUCE_USE_CURL=0
            SAUCECHOP_TEST_MP3_PATH="${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/examples/Assets/Notifications/sounds/served.mp3")

    add_test(NAME SauceChop.UnitTests COMMAND SauceChopTests)
endif()
