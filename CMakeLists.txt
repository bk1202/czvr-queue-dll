cmake_minimum_required(VERSION 3.15)
project(CZVR_ReqQueue LANGUAGES CXX)

# EuroScope is a 32-bit application - plugins MUST be built x86, not x64.
# In Visual Studio, make sure your active configuration platform is Win32/x86.
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING "You are not building 32-bit (x86). EuroScope will refuse to load a 64-bit plugin DLL.")
endif()

# Point this at the folder containing EuroScopePlugIn.h and EuroScope.lib.
# See README.md for where to get these - they are NOT bundled with a normal
# EuroScope install as of 3.2+.
set(EUROSCOPE_SDK_DIR "" CACHE PATH "Folder containing EuroScopePlugIn.h and EuroScope.lib")

if(EUROSCOPE_SDK_DIR STREQUAL "")
    message(FATAL_ERROR "Set -DEUROSCOPE_SDK_DIR=<path> to the folder with EuroScopePlugIn.h and EuroScope.lib")
endif()

add_library(CZVR_ReqQueue SHARED
    src/PluginMain.cpp
    src/ReqQueuePlugin.cpp
    src/ReqQueuePlugin.h
)

target_include_directories(CZVR_ReqQueue PRIVATE ${EUROSCOPE_SDK_DIR})
target_link_directories(CZVR_ReqQueue PRIVATE ${EUROSCOPE_SDK_DIR})
target_link_libraries(CZVR_ReqQueue PRIVATE EuroScope.lib)

# Output a .dll with a clean name EuroScope's plugin loader will show.
set_target_properties(CZVR_ReqQueue PROPERTIES
    OUTPUT_NAME "CZVR_ReqQueue"
    SUFFIX ".dll"
)

target_compile_features(CZVR_ReqQueue PRIVATE cxx_std_14)
