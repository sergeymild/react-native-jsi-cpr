cmake_minimum_required(VERSION 3.10)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-DFOLLY_NO_CONFIG=1 -DFOLLY_HAVE_CLOCK_GETTIME=1 -DFOLLY_HAVE_MEMRCHR=1 -DFOLLY_USE_LIBCPP=1 -DFOLLY_MOBILE=1 -DON_ANDROID -DONANDROID")

set (PACKAGE_NAME "react-native-jsi-cpr")
set (BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set (CPPHTTPLIB_ZLIB_SUPPORT 0)

# Consume shared libraries and headers from prefabs
find_package(fbjni REQUIRED CONFIG)
find_package(ReactAndroid REQUIRED CONFIG)


file(GLOB cpr_library "../cpp/cpr/*.cpp")

add_library(
        ${PACKAGE_NAME}
        SHARED
        src/main/cpp/HttpHelper.cpp

        ../cpp/JsiHttp.cpp
        ../cpp/JsiUtils.cpp
        ../cpp/Logger.cpp

        ${cpr_library}
)


target_include_directories(
        ${PACKAGE_NAME}
        PRIVATE
        "${REACT_NATIVE_DIR}/ReactAndroid/src/main/jni/react/turbomodule"
        "${REACT_NATIVE_DIR}/ReactCommon"
        "${REACT_NATIVE_DIR}/ReactCommon/callinvoker"
        "src/main/cpp"
        "../cpp"
)

include_directories(
        "../cpp/include"
)


find_library(
        LOG_LIB
        log
)

find_library(z-lib z)
find_package(curl REQUIRED CONFIG)

target_link_libraries(
        ${PACKAGE_NAME}
        ${LOG_LIB}
        ReactAndroid::jsi
        ReactAndroid::reactnativejni
        fbjni::fbjni
        curl::curl
        ${z-lib}
        android
)
