cmake_minimum_required(VERSION 3.21)

project(MixingStudio VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Sql Multimedia Test)

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
    include/Model/WavExporter.h
    src/Model/WavExporter.cpp
    include/Model/WavDecoder.h
    src/Model/WavDecoder.cpp
    include/Model/AudioFileDecoder.h
    src/Model/AudioFileDecoder.cpp
)

set(MIXING_STUDIO_COMMON_SOURCES
    include/Common/MixerTypes.h
)

set(MIXING_STUDIO_APP_SOURCES
    include/App/MixingStudioApp.h
    src/App/MixingStudioApp.cpp
)

set(MIXING_STUDIO_VIEWMODEL_SOURCES
    include/ViewModel/TrackViewModel.h
    src/ViewModel/TrackViewModel.cpp
    include/ViewModel/RealMixerViewModel.h
    src/ViewModel/RealMixerViewModel.cpp
)

set(MIXING_STUDIO_QML_FILES
    src/View/Main.qml
    src/View/Theme.qml
    src/View/DarkButton.qml
    src/View/ToolIconButton.qml
    src/View/ClipLoopToggle.qml
    src/View/NumericValueField.qml
    src/View/LevelMeterBar.qml
    src/View/TimelineStrip.qml
    src/View/DetachablePanelHost.qml
    src/View/TransportBar.qml
    src/View/WaveformPanel.qml
    src/View/SpectrumPanel.qml
    src/View/ProjectsPanel.qml
    src/View/LibraryPanel.qml
    src/View/TrackMixerList.qml
)

# Shared QML module (app + UI automation test)
qt_add_library(MixingStudioQml STATIC)
qt_add_qml_module(MixingStudioQml
    URI MixingStudio
    VERSION 1.0
    QML_FILES ${MIXING_STUDIO_QML_FILES}
)
set_source_files_properties(src/View/Theme.qml PROPERTIES
    QT_QML_SINGLETON_TYPE TRUE
)

qt_add_executable(MixingStudio
    src/main.cpp
    ${MIXING_STUDIO_COMMON_SOURCES}
    ${MIXING_STUDIO_DSP_SOURCES}
    ${MIXING_STUDIO_MODEL_SOURCES}
    ${MIXING_STUDIO_APP_SOURCES}
    ${MIXING_STUDIO_VIEWMODEL_SOURCES}
)

set_target_properties(MixingStudio PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(MixingStudio
    PRIVATE
        MixingStudioQml
        MixingStudioQmlplugin
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Sql
        Qt6::Multimedia
)

target_include_directories(MixingStudio
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(APPLE)
    add_custom_command(TARGET MixingStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/samples"
                "$<TARGET_BUNDLE_CONTENT_DIR:MixingStudio>/Resources/samples"
        COMMENT "Copy sample project into the application bundle"
    )
else()
    add_custom_command(TARGET MixingStudio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/samples"
                "$<TARGET_FILE_DIR:MixingStudio>/samples"
        COMMENT "Copy sample project next to MixingStudio"
    )
endif()

install(TARGETS MixingStudio
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(NOT APPLE)
    install(DIRECTORY samples
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

qt_generate_deploy_qml_app_script(
    TARGET MixingStudio
    OUTPUT_SCRIPT MIXING_STUDIO_DEPLOY_SCRIPT
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${MIXING_STUDIO_DEPLOY_SCRIPT})

enable_testing()

add_test(NAME app_smoke COMMAND MixingStudio --smoke-test)
set_tests_properties(app_smoke PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;MIXING_STUDIO_FORCE_TIMER_CLOCK=1"
    TIMEOUT 10
)

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
        Qt6::Multimedia
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
        Qt6::Multimedia
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
        Qt6::Multimedia
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

add_executable(test_viewmodel_commands
    tests/test_viewmodel_commands.cpp
    ${MIXING_STUDIO_VIEWMODEL_SOURCES}
    ${MIXING_STUDIO_MODEL_SOURCES}
    ${MIXING_STUDIO_DSP_SOURCES}
)

set_target_properties(test_viewmodel_commands PROPERTIES AUTOMOC ON)

target_include_directories(test_viewmodel_commands
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_viewmodel_commands
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Sql
        Qt6::Multimedia
)

add_test(NAME viewmodel_commands COMMAND test_viewmodel_commands)

set_tests_properties(viewmodel_commands PROPERTIES
    ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
)

add_executable(test_wav_export
    tests/test_wav_export.cpp
    ${MIXING_STUDIO_MODEL_SOURCES}
    ${MIXING_STUDIO_DSP_SOURCES}
)

set_target_properties(test_wav_export PROPERTIES AUTOMOC ON)

target_include_directories(test_wav_export
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_wav_export
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Multimedia
)

add_test(NAME wav_export COMMAND test_wav_export)

set_tests_properties(wav_export PROPERTIES
    ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
)

add_executable(test_wav_decoder
    tests/test_wav_decoder.cpp
    src/Model/WavDecoder.cpp
    src/Model/WavExporter.cpp
    src/Model/AudioFileDecoder.cpp
)

target_include_directories(test_wav_decoder
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_wav_decoder
    PRIVATE
        Qt6::Core
        Qt6::Multimedia
)

add_test(NAME wav_decoder COMMAND test_wav_decoder)

set_tests_properties(wav_decoder PROPERTIES
    ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
)

add_executable(test_automation
    tests/test_automation.cpp
)

target_include_directories(test_automation
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_automation
    PRIVATE
        Qt6::Core
)

add_test(NAME automation COMMAND test_automation)

# UI automation: open Main window, drive Demo/Play/Stop/Master/Seek via QTest mouse+keyboard
add_executable(test_ui_binder
    tests/test_ui_binder.cpp
    include/App/MixingStudioApp.h
    src/App/MixingStudioApp.cpp
    ${MIXING_STUDIO_VIEWMODEL_SOURCES}
    ${MIXING_STUDIO_MODEL_SOURCES}
    ${MIXING_STUDIO_DSP_SOURCES}
)

set_target_properties(test_ui_binder PROPERTIES AUTOMOC ON)

target_include_directories(test_ui_binder
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(test_ui_binder
    PRIVATE
        MixingStudioQml
        MixingStudioQmlplugin
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Sql
        Qt6::Multimedia
        Qt6::Test
)

target_compile_definitions(test_ui_binder
    PRIVATE
        MIXINGSTUDIO_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

add_test(NAME ui_binder COMMAND test_ui_binder)

set_tests_properties(ui_binder PROPERTIES
    ENVIRONMENT "PATH=$<TARGET_FILE_DIR:Qt6::Core>;$ENV{PATH}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    TIMEOUT 120
)
set_property(TEST ui_binder APPEND PROPERTY ENVIRONMENT "MIXINGSTUDIO_UI_HEADLESS=1")
set_property(TEST ui_binder APPEND PROPERTY ENVIRONMENT "MIXINGSTUDIO_UI_PLAY_MS=3000")

add_executable(test_audio_pipeline_e2e
    tests/test_audio_pipeline_e2e.cpp
    ${MIXING_STUDIO_MODEL_SOURCES}
    ${MIXING_STUDIO_DSP_SOURCES}
)

set_target_properties(test_audio_pipeline_e2e PROPERTIES AUTOMOC ON)

target_include_directories(test_audio_pipeline_e2e
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(test_audio_pipeline_e2e
    PRIVATE MIXING_STUDIO_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(test_audio_pipeline_e2e
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Multimedia
)

add_test(NAME audio_pipeline_e2e COMMAND test_audio_pipeline_e2e)
