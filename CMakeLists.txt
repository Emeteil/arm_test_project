cmake_minimum_required(VERSION 3.16)
project(arm_project LANGUAGES CXX)

option(ARMP_BUILD_GUI "Собирать графический интерфейс (GLFW + ImGui)" ON)
option(ARMP_FORCE_NEON "Принудительно включить -mfpu=neon на ARM32" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(ARMP_IS_ARM FALSE)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "(aarch64|arm64|armv7|armv8|arm)")
    set(ARMP_IS_ARM TRUE)
    message(STATUS "[arm_project] Целевая архитектура: ARM (${CMAKE_SYSTEM_PROCESSOR})")
else()
    message(STATUS "[arm_project] Целевая архитектура: ${CMAKE_SYSTEM_PROCESSOR} (NEON-функции будут заменены на скалярные)")
endif()

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
    add_compile_options($<$<CONFIG:Release>:-O3>)
endif()

if(ARMP_IS_ARM AND NOT MSVC)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
        if(ARMP_FORCE_NEON)
            add_compile_options(-mfpu=neon -mfloat-abi=hard)
        endif()
    endif()
endif()

add_subdirectory(third_party)

add_library(armp_core STATIC
    src/core/Platform.cpp
    src/core/algo/SignalEnergy.cpp
    src/core/algo/SignalEnergyScalar.cpp
    src/core/bench/Benchmark.cpp
)
target_include_directories(armp_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_features(armp_core PUBLIC cxx_std_17)

if(NOT MSVC)
    set_source_files_properties(
        src/core/algo/SignalEnergyScalar.cpp
        PROPERTIES COMPILE_OPTIONS "-fno-tree-vectorize"
    )
endif()

add_library(armp_cli STATIC src/cli/CliApp.cpp)
target_include_directories(armp_cli PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(armp_cli PUBLIC armp_core)
if(ARMP_TUI_AVAILABLE)
    target_link_libraries(armp_cli PUBLIC armp_ftxui)
    target_compile_definitions(armp_cli PUBLIC ARMP_HAVE_TUI=1)
endif()

set(_armp_gui_target "")
if(ARMP_BUILD_GUI AND ARMP_GUI_AVAILABLE)
    add_library(armp_gui STATIC
        src/gui/App.cpp
        src/gui/Theme.cpp
        src/gui/widgets/ParticleField.cpp
        src/gui/widgets/BenchmarkView.cpp
        src/gui/widgets/ChartView.cpp
        src/gui/widgets/DemoView.cpp
    )
    target_include_directories(armp_gui PUBLIC ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(armp_gui PUBLIC armp_core armp_imgui)
    target_compile_definitions(armp_gui PUBLIC ARMP_HAVE_GUI=1)
    set(_armp_gui_target armp_gui)
endif()

add_executable(arm_project src/app/main.cpp)
target_link_libraries(arm_project PRIVATE armp_cli ${_armp_gui_target})
if(_armp_gui_target)
    target_compile_definitions(arm_project PRIVATE ARMP_HAVE_GUI=1)
endif()

find_package(Threads REQUIRED)
target_link_libraries(armp_core PUBLIC Threads::Threads)

set_target_properties(arm_project PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

message(STATUS "[arm_project] GUI: ${ARMP_GUI_AVAILABLE}, type: ${CMAKE_BUILD_TYPE}")