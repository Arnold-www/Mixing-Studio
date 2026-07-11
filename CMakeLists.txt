cmake_minimum_required(VERSION 3.21)

project(MixingStudio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Qml Quick QuickControls2)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(MixingStudio
    src/main.cpp
    include/DSP/DspProcessor.h
    src/DSP/DspProcessor.cpp
    include/Model/AudioEngine.h
    src/Model/AudioEngine.cpp
    include/ViewModel/TrackViewModel.h
    src/ViewModel/TrackViewModel.cpp
    include/ViewModel/MixerViewModel.h
    src/ViewModel/MixerViewModel.cpp
)

set_target_properties(MixingStudio PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

qt_add_qml_module(MixingStudio
    URI MixingStudio
    VERSION 1.0
    QML_FILES
        src/View/Main.qml
)

target_link_libraries(MixingStudio
    PRIVATE
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
)

target_include_directories(MixingStudio
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()

add_executable(test_dsp_processor
    tests/test_dsp_processor.cpp
    src/DSP/DspProcessor.cpp
)

target_include_directories(test_dsp_processor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_test(NAME dsp_processor COMMAND test_dsp_processor)
