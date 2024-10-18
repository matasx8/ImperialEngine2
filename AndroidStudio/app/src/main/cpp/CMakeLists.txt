cmake_minimum_required(VERSION 3.22.1)

project(UGKTests)
set(BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../")

function(map_build_type INPUT OUTPUT)
    if(${INPUT} STREQUAL "Release" OR ${INPUT} STREQUAL "RelWithDebInfo")
        set(${OUTPUT} "release" PARENT_SCOPE)
    elseif(${INPUT} STREQUAL "Debug")
        set(${OUTPUT} "debug" PARENT_SCOPE)
    else()
        set(${OUTPUT} ${INPUT} PARENT_SCOPE)
    endif()
endfunction()

function(map_architecture INPUT OUTPUT)
    if(${INPUT} STREQUAL "aarch64")
        set(${OUTPUT} "arm64-v8a" PARENT_SCOPE)
    elseif(${INPUT} STREQUAL "armv7-a")
        set(${OUTPUT} "arm32-v7a" PARENT_SCOPE)
    else()
        set(${OUTPUT} ${INPUT} PARENT_SCOPE)
    endif()
endfunction()

#map_build_type("${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)
map_architecture("${CMAKE_SYSTEM_PROCESSOR}" SYSTEM_ARCH)

add_library(native-activity SHARED main.cpp)

add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC
        ${ANDROID_NDK}/sources/android/native_app_glue)

add_library(imperial_engine SHARED IMPORTED)
set_target_properties(imperial_engine PROPERTIES
        IMPORTED_LOCATION ${BASE_DIR}/build/${CMAKE_BUILD_TYPE}/${SYSTEM_ARCH}/libimperial_engine.so
        IMPORTED_LINK_INTERFACE_LANGUAGES CXX)

find_library(log-lib log)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

target_link_libraries(native-activity
        android
        native_app_glue
        imperial_engine
        ${log-lib})
