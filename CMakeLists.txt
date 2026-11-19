cmake_minimum_required(VERSION 3.28)
project(gui_ex_03_a)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- wxWidgets ---------------------------------------------------------------
# Use an installed wxWidgets if available (e.g. apt install libwxgtk3.2-dev),
# otherwise download and build it automatically (first build takes a while).
# For the MinGW cross-build, a prebuilt wxWidgets install is used (see
# WX_MINGW_ROOT in cmake/mingw-w64-x86_64.cmake); it ships a CMake config.
if (WIN32)
    find_package(wxWidgets 3.2 CONFIG QUIET COMPONENTS core base)
else ()
    find_package(wxWidgets 3.2 QUIET COMPONENTS core base)
endif ()

if (TARGET wx::core)
    message(STATUS "Using prebuilt wxWidgets from ${wxWidgets_DIR}")
    set(WX_LIBS wx::core wx::base)
elseif (wxWidgets_FOUND)
    include(${wxWidgets_USE_FILE})
    set(WX_LIBS ${wxWidgets_LIBRARIES})
else ()
    message(STATUS "wxWidgets not found - fetching it from GitHub")
    include(FetchContent)
    set(wxBUILD_SHARED OFF CACHE BOOL "" FORCE)   # link statically
    set(wxBUILD_SAMPLES OFF CACHE STRING "" FORCE)
    set(wxBUILD_TESTS OFF CACHE STRING "" FORCE)
    set(wxBUILD_DEMOS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        wxWidgets
        GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
        GIT_TAG        v3.2.6
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(wxWidgets)
    set(WX_LIBS wx::core wx::base)
endif ()

# --- Executable --------------------------------------------------------------
# Put the built executable into <project>/exe/<platform>
#   Linux   -> exe/linux
#   Windows -> exe/windows  (native Windows build or MinGW cross-compile)
if (WIN32)
    set(EXE_PLATFORM windows)
elseif (APPLE)
    set(EXE_PLATFORM macos)
else ()
    set(EXE_PLATFORM linux)
endif ()
set(EXE_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/exe/${EXE_PLATFORM})

add_executable(gui_ex_03_a main.cpp SubWin.cpp SubWin.h)
target_link_libraries(gui_ex_03_a PRIVATE ${WX_LIBS})

# MinGW: link the GCC runtime statically so the .exe runs on Windows
# without needing extra DLLs (libstdc++, libgcc, libwinpthread)
if (MINGW)
    target_link_options(gui_ex_03_a PRIVATE -static -static-libgcc -static-libstdc++)
endif ()

set_target_properties(gui_ex_03_a PROPERTIES
    WIN32_EXECUTABLE TRUE            # no console window on Windows
    RUNTIME_OUTPUT_DIRECTORY         ${EXE_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${EXE_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXE_OUTPUT_DIR}
)
