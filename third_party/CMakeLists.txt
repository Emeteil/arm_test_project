# third_party/CMakeLists.txt
# Сборка зависимостей: GLFW и Dear ImGui (опционально, при наличии).

set(ARMP_GUI_AVAILABLE FALSE PARENT_SCOPE)

set(_glfw_dir "${CMAKE_CURRENT_SOURCE_DIR}/glfw")
set(_imgui_dir "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(_ftxui_dir "${CMAKE_CURRENT_SOURCE_DIR}/ftxui")

if(EXISTS "${_glfw_dir}/CMakeLists.txt" AND EXISTS "${_imgui_dir}/imgui.h")
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
    add_subdirectory(${_glfw_dir} EXCLUDE_FROM_ALL)

    add_library(armp_imgui STATIC
        ${_imgui_dir}/imgui.cpp
        ${_imgui_dir}/imgui_draw.cpp
        ${_imgui_dir}/imgui_tables.cpp
        ${_imgui_dir}/imgui_widgets.cpp
        ${_imgui_dir}/imgui_demo.cpp
        ${_imgui_dir}/backends/imgui_impl_glfw.cpp
        ${_imgui_dir}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(armp_imgui PUBLIC
        ${_imgui_dir}
        ${_imgui_dir}/backends
    )
    target_link_libraries(armp_imgui PUBLIC glfw)
    target_compile_features(armp_imgui PUBLIC cxx_std_17)

    if(NOT WIN32 AND NOT APPLE)
        find_package(OpenGL QUIET)
        if(OpenGL_FOUND)
            target_link_libraries(armp_imgui PUBLIC OpenGL::GL)
        endif()
    elseif(WIN32)
        target_link_libraries(armp_imgui PUBLIC opengl32)
    elseif(APPLE)
        target_link_libraries(armp_imgui PUBLIC "-framework OpenGL")
    endif()

    set(ARMP_GUI_AVAILABLE TRUE PARENT_SCOPE)
    message(STATUS "[arm_project] GUI зависимости найдены, GUI будет собран.")
else()
    message(WARNING "[arm_project] third_party/glfw или third_party/imgui отсутствуют - GUI отключён, доступен только CLI.")
endif()

if(EXISTS "${_ftxui_dir}/CMakeLists.txt" AND EXISTS "${_ftxui_dir}/include/ftxui/component/component.hpp")
    set(FTXUI_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(FTXUI_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(FTXUI_QUIET          ON  CACHE BOOL "" FORCE)
    add_subdirectory(${_ftxui_dir} EXCLUDE_FROM_ALL)

    add_library(armp_ftxui INTERFACE)
    target_link_libraries(armp_ftxui INTERFACE screen dom component)
    target_include_directories(armp_ftxui INTERFACE ${_ftxui_dir}/include)
    target_compile_features(armp_ftxui INTERFACE cxx_std_17)

    set(ARMP_TUI_AVAILABLE TRUE PARENT_SCOPE)
    message(STATUS "[arm_project] FTXUI найден, TUI-интерфейс CLI будет собран.")
else()
    set(ARMP_TUI_AVAILABLE FALSE PARENT_SCOPE)
    message(WARNING "[arm_project] third_party/ftxui отсутствует - TUI-интерфейс CLI отключён.")
endif()
