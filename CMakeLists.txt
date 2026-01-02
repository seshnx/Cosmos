cmake_minimum_required(VERSION 3.22)

project(Cosmos VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# AAX SDK Path
set(AAX_SDK_PATH "C:/aax-sdk-2-9-0" CACHE PATH "Path to AAX SDK")

# JUCE setup via FetchContent
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# Set AAX SDK path
juce_set_aax_sdk_path(${AAX_SDK_PATH})

# Plugin definition
juce_add_plugin(Cosmos
    COMPANY_NAME "SeshNx"
    PLUGIN_MANUFACTURER_CODE Sesh
    PLUGIN_CODE Cosm
    FORMATS VST3 AU Standalone AAX
    PRODUCT_NAME "SeshNx Cosmos"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

# Source files
target_sources(Cosmos
    PRIVATE
        # Core
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp

        # DSP
        Source/DSP/AllpassFilter.cpp
        Source/DSP/CombFilter.cpp
        Source/DSP/DiffusionNetwork.cpp
        Source/DSP/ModulationEngine.cpp
        Source/DSP/AlgorithmicReverb.cpp
        Source/DSP/FairingSeparation.cpp

        # UI
        Source/UI/CosmosLookAndFeel.cpp
        Source/UI/StarfieldVisualizer.cpp
        Source/UI/EngineKnob.cpp
        Source/UI/DecayCurveDisplay.cpp

        # Utils
        Source/Utils/Parameters.cpp
)

# Include directories
target_include_directories(Cosmos
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/DSP
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/UI
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Utils
)

# Binary resources (logo and nebula images)
juce_add_binary_data(Cosmos_BinaryData
    SOURCES
        Resources/company_logo.png
        Resources/nebula_pillars.jpg
        Resources/nebula_crab.jpg
        Resources/nebula_orion.jpg
        Resources/nebula_helix.jpg
        Resources/nebula_horsehead.jpg
        Resources/nebula_ring.jpg
        Resources/nebula_carina.jpg
        Resources/nebula_lagoon.jpg
        Resources/nebula_veil.jpg
        Resources/nebula_catseye.png
        Resources/nebula_tarantula.jpg
    HEADER_NAME BinaryData.h
)

# Compiler definitions
target_compile_definitions(Cosmos
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link JUCE modules
target_link_libraries(Cosmos
    PRIVATE
        Cosmos_BinaryData
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
