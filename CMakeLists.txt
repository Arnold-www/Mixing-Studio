cmake_minimum_required(VERSION 3.21)

project(MixingStudio VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Sql)

qt_standard_project_setup(REQUIRES 6.5)

set(MIXING_STUDIO_DSP_SOURCES
    include/DSP/DspProcessor.h
    src/DSP/DspProcessor.cpp
    include/DSP/DspAnalysis.h
    src/DSP/DspAnalysis.cpp
)

set(MIXING_STUDIO_MODEL_SOURCES
    include/Model/AudioTrack.h
    include/Model/AudioEngine.h
    src/Model/AudioEngine.cpp
    src/Model/AudioEngine_Playback.cpp
    src/Model/AudioEngine_Tracks.cpp
    src/Model/AudioEngine_Analysis.cpp
    src/Model/AudioEngine_Persistence.cpp
    include/Model/ProjectStore.h
    src/Model/ProjectStore.cpp
    include/Model/AssetLibrary.h
    src/Model/AssetLibrary.cpp
)

set(MIXING_STUDIO_COMMON_SOURCES
    include/Common/ICommandBase.h
    include/Common/MixerTypes.h
)

set(MIXING_STUDIO_APP_SOURCES
    include/App/MixingStudioApp.h
    src/App/MixingStudioApp.cpp
)

set(MIXING_STUDIO_COMMAND_SOURCES
    include/Command/ICommand.h
    include/Command/PlaybackCommands.h
    src/Command/PlaybackCommands.cpp
    include/Command/ProjectCommands.h
    src/Command/ProjectCommands.cpp
    include/Command/TrackDspCommands.h
    src/Command/TrackDspCommands.cpp
    include/Command/MixerCommands.h
)

set(MIXING_STUDIO_VIEWMODEL_SOURCES
    include/ViewModel/ITrackViewModel.h
    include/ViewModel/IMixerViewModel.h
    include/ViewModel/TrackViewModel.h
    src/ViewModel/TrackViewModel.cpp
    include/ViewModel/RealMixerViewModel.h
    src/ViewModel/RealMixerViewModel.cpp
)

qt_add_executable(MixingStudio
    src/main.cpp
    ${MIXING_STUDIO_COMMON_SOURCES}
    ${MIXING_STUDIO_DSP_SOURCES}
    ${MIXING_STUDIO_MODEL_SOURCES}
    ${MIXING_STUDIO_APP_SOURCES}
    ${MIXING_STUDIO_COMMAND_SOURCES}
    ${MIXING_STUDIO_VIEWMODEL_SOURCES}
)

set_target_properties(MixingStudio PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

qt_add_qml_module(MixingStudio
    URI MixingStudio
    VERSION 1.0
    QML_FILES
        src/View/Main.qml
        src/View/TransportBar.qml
        src/View/WaveformPanel.qml
        src/View/SpectrumPanel.qml
        src/View/LibraryPanel.qml
        src/View/TrackMixerList.qml
)

target_link_libraries(MixingStudio
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Sql
)

target_include_directories(MixingStudio
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()

add_executable(test_dsp_processor
    tests/test_dsp_processor.cpp
    src/DSP/DspProcessor.cpp
    src/DSP/DspAnalysis.cpp
)

target_include_directories(test_dsp_processor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_test(NAME dsp_processor COMMAND test_dsp_processor)

add_executable(test_audio_engine
    tests/test_audio_engine.cpp
    ${MIXING_STUDIO_MODEL_SOURCES}
    ${MIXING_STUDIO_DSP_SOURCES}
)

set_target_properties(test_audio_engine PROPERTIES AUTOMOC ON)

target_include_directories(test_audio_engine
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_audio_engine
    PRIVATE
        Qt6::Core
        Qt6::Sql
)

add_test(NAME audio_engine COMMAND test_audio_engine)

set_tests_properties(audio_engine PROPERTIES
    ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
)

add_executable(test_project_store
    tests/test_project_store.cpp
    ${MIXING_STUDIO_MODEL_SOURCES}
    ${MIXING_STUDIO_DSP_SOURCES}
)

set_target_properties(test_project_store PROPERTIES AUTOMOC ON)

target_include_directories(test_project_store
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_project_store
    PRIVATE
        Qt6::Core
        Qt6::Sql
)

add_test(NAME project_store COMMAND test_project_store)

set_tests_properties(project_store PROPERTIES
    ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
)

add_executable(test_asset_library
    tests/test_asset_library.cpp
    src/Model/AssetLibrary.cpp
)

target_include_directories(test_asset_library
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_asset_library
    PRIVATE
        Qt6::Core
        Qt6::Sql
)

add_test(NAME asset_library COMMAND test_asset_library)

set_tests_properties(asset_library PROPERTIES
    ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
)

add_executable(test_common_types
    tests/test_common_types.cpp
)

target_include_directories(test_common_types
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_common_types
    PRIVATE
        Qt6::Core
)

add_test(NAME common_types COMMAND test_common_types)

set_tests_properties(common_types PROPERTIES
    ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
)

add_executable(test_commands
    tests/test_commands.cpp
    ${MIXING_STUDIO_COMMAND_SOURCES}
    ${MIXING_STUDIO_MODEL_SOURCES}
    ${MIXING_STUDIO_DSP_SOURCES}
)

set_target_properties(test_commands PROPERTIES AUTOMOC ON)

target_include_directories(test_commands
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_commands
    PRIVATE
        Qt6::Core
        Qt6::Sql
)

add_test(NAME commands COMMAND test_commands)

set_tests_properties(commands PROPERTIES
    ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
)
