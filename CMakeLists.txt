cmake_minimum_required(VERSION 3.10)

project(ImperialEngine2 LANGUAGES CXX)

add_library(imperial_engine SHARED  Source/Engine.cpp
                                    Source/Layers.cpp
                                    Source/Debug.cpp
                                    Source/Queue.cpp
                                    Source/Log.h)

set_target_properties(imperial_engine PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

set_target_properties(imperial_engine PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${CMAKE_ANDROID_ARCH_ABI}"
)

# Vulkan
find_package(Vulkan REQUIRED)
target_link_libraries(imperial_engine PUBLIC Vulkan::Vulkan)

# Volk
add_subdirectory("External/Volk")
target_include_directories(imperial_engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/External/Volk")
target_link_libraries(imperial_engine PUBLIC volk)

if(ANDROID)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_ANDROID_KHR)
else()
    message("Must add support for this platform!")
endif()




if(ANDROID)
    target_link_libraries(imperial_engine PUBLIC vulkan)
endif()

# If generating for Visual Studio, disable building (only generate for editing)
if (MSVC)
    set_target_properties(imperial_engine PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        EXCLUDE_FROM_DEFAULT_BUILD TRUE
    )
endif()