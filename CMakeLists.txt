# Change this to your project name
set(PROJECT_NAME "Schwring")

# cmake version to be used
cmake_minimum_required(VERSION 3.20)

# project name
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# adding cmake module
add_subdirectory(tests)
add_subdirectory(libs/mx)
add_subdirectory(libs/tracktion_engine/modules/juce ./cmake_build_juce)
add_subdirectory(libs/tracktion_engine/modules ./cmake_build_tracktion)


#  adding juce modules
juce_add_module(src/hello_world_module)

# target
#add_executable(schwrrring main.cpp)


juce_add_gui_app(${PROJECT_NAME}
        ENABLE_EXPORTS true
        PRODUCT_NAME "EasyEars"
        )
target_sources(${PROJECT_NAME}
        PRIVATE
        main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        JUCE_UNIT_TESTS=1
        JUCE_USE_CURL=0d
        JUCE_WEB_BROWSER=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_PLUGINHOST_AU=0
        JUCE_PLUGINHOST_VST3=0
        JUCE_MODAL_LOOPS_PERMITTED=1
        )

# include
target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/tracktion_engine/
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

# external libs
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        # GuiAppData            # If we'd created a binary data target, we'd link to it here
        juce::juce_gui_extra
        PUBLIC
        #  todo: move as a dependency along with the other ones to the juce module.
        mx
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
        tracktion::tracktion_engine
        hello_world_module
        )
juce_generate_juce_header(${PROJECT_NAME})

#target_link_directories()