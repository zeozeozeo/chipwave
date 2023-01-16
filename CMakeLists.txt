cmake_minimum_required(VERSION 3.5)

project(chipwave LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find SDL2
find_package(SDL2 REQUIRED)
include_directories(chipwave ${SDL2_INCLUDE_DIRS})

if(RASPBIAN AND RPI_HAS_NEON)
    add_compile_options(-mfpu=neon)
endif()

include_directories(include)

# compile imgui as a static library
set(IMGUI_PATH ${CMAKE_CURRENT_LIST_DIR}/include/imgui)
add_library(ImGui STATIC)

target_sources(ImGui PRIVATE
    ${IMGUI_PATH}/imgui_demo.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl2.cpp
    ${IMGUI_PATH}/backends/imgui_impl_sdl.cpp
)

target_include_directories(ImGui
    PUBLIC ${IMGUI_PATH}
    PUBLIC ${IMGUI_PATH}/backends
    PUBLIC ${IMGUI_PATH}/include/SDL2
)


# link GLES/OpenGL
if(USE_GLES OR RASPBIAN)
    target_compile_definitions(ImGui PUBLIC -DIMGUI_IMPL_OPENGL_ES2)

    if(RASPBIAN AND RPI_USE_BROADCOM_DRIVER)
        target_link_directories(ImGui PUBLIC /opt/vc/lib)
        target_link_libraries(ImGui PUBLIC brcmGLESv2 brcmEGL)
    else()
        target_link_libraries(ImGui PUBLIC GLESv2)
    endif()
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(ImGui PUBLIC ${OPENGL_LIBRARIES})
endif()

add_executable(chipwave
    src/main.cpp
    src/mixer.cpp
    src/mixer.h
    src/node.cpp
    src/node.h
    src/envelope.cpp
    src/envelope.h
    src/voice.cpp
    src/voice.h
)

target_link_libraries(chipwave ${SDL2_LIBRARIES} ImGui)

install(TARGETS chipwave
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
