# ============================================================================ #
#                       Application name and options                           #
# ============================================================================ #

# Configure build environment
include(../../cmake/BuildEnvironment.cmake)

# OpenGL dependency
set(OpenGL_GL_PREFERENCE GLVND)
if (NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
endif()
include_directories(${OPENGL_INCLUDE_DIRS})

# Target name
set(TARGET imgui_utils)

# Print status message
message(STATUS "App ${TARGET}")

# ============================================================================ #
#                       Sources                                                #
# ============================================================================ #

# Source files
file(GLOB_RECURSE SOURCES_H_CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )

# ============================================================================ #
#                       Create Library                                         #
# ============================================================================ #

babylon_add_library(${TARGET} STATIC ${SOURCES_H_CPP} $<TARGET_OBJECTS:ImGuiColorTextEdit>)

target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_include_directories(${TARGET} PRIVATE SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../../external)

# Libraries
target_link_libraries(${TARGET}
    PUBLIC
    imgui
    BabylonCpp
)
if (NOT EMSCRIPTEN)
    target_link_libraries(${TARGET} PUBLIC ${OPENGL_LIBRARIES})
endif()

if (UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
    target_link_libraries(${TARGET} PUBLIC X11)
endif()
