cmake_minimum_required(VERSION 3.10)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-DFOLLY_NO_CONFIG=1 -DFOLLY_HAVE_CLOCK_GETTIME=1 -DFOLLY_HAVE_MEMRCHR=1 -DFOLLY_USE_LIBCPP=1 -DFOLLY_MOBILE=1 -DON_ANDROID -DONANDROID")

set (PACKAGE_NAME "react-native-jsi-cpr")
set (BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set (CPPHTTPLIB_ZLIB_SUPPORT 0)
set (RN_SO_DIR ${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/jni/first-party/react/jni)

add_library(
        ${PACKAGE_NAME}
        SHARED
        src/main/cpp/HttpHelper.cpp
        ../cpp/JsiHttp.cpp
        ../cpp/JsiUtils.cpp
        ../cpp/Logger.cpp

        ../cpp/cpr/accept_encoding.cpp
        ../cpp/cpr/async.cpp
        ../cpp/cpr/auth.cpp
        ../cpp/cpr/bearer.cpp
        ../cpp/cpr/cert_info.cpp
        ../cpp/cpr/cookies.cpp
        ../cpp/cpr/cprtypes.cpp
        ../cpp/cpr/curl_container.cpp
        ../cpp/cpr/curlholder.cpp
        ../cpp/cpr/curlmultiholder.cpp
        ../cpp/cpr/error.cpp
        ../cpp/cpr/file.cpp
        ../cpp/cpr/interceptor.cpp
        ../cpp/cpr/multipart.cpp
        ../cpp/cpr/multiperform.cpp
        ../cpp/cpr/parameters.cpp
        ../cpp/cpr/payload.cpp
        ../cpp/cpr/proxies.cpp
        ../cpp/cpr/proxyauth.cpp
        ../cpp/cpr/redirect.cpp
        ../cpp/cpr/response.cpp
        ../cpp/cpr/session.cpp
        ../cpp/cpr/ssl_ctx.cpp
        ../cpp/cpr/threadpool.cpp
        ../cpp/cpr/timeout.cpp
        ../cpp/cpr/unix_socket.cpp
        ../cpp/cpr/util.cpp
)

# includes

file (GLOB LIBFBJNI_INCLUDE_DIR "${BUILD_DIR}/fbjni-*-headers.jar/")

target_include_directories(
        ${PACKAGE_NAME}
        PRIVATE
        # --- fbjni ---
        "${LIBFBJNI_INCLUDE_DIR}"
#        # --- Third Party (required by RN) ---
#        "${BUILD_DIR}/third-party-ndk/boost"
#        "${BUILD_DIR}/third-party-ndk/double-conversion"
#        "${BUILD_DIR}/third-party-ndk/folly"
#        "${BUILD_DIR}/third-party-ndk/glog"
        # --- React Native ---
        "${NODE_MODULES_DIR}/react-native/React"
        "${NODE_MODULES_DIR}/react-native/React/Base"
        "${NODE_MODULES_DIR}/react-native/ReactCommon/jsi"
#                "${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/jni"
                "${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/java/com/facebook/react/turbomodule/core/jni"
                "${NODE_MODULES_DIR}/react-native/ReactCommon"
                "${NODE_MODULES_DIR}/react-native/ReactCommon/callinvoker"
#        "${NODE_MODULES_DIR}/hermes-engine/android/include/"
        "src/main/cpp"
        "../cpp"
)

include_directories(
        "../cpp/include"
)

# find libraries

file (GLOB LIBRN_DIR "${BUILD_DIR}/react-native-0*/jni/${ANDROID_ABI}")

find_library(
        FBJNI_LIB
        fbjni
        PATHS ${LIBRN_DIR}
        NO_CMAKE_FIND_ROOT_PATH
)
if(${REACT_NATIVE_VERSION} LESS 69)
    find_library(
            FOLLY_JSON_LIB
            folly_json
            PATHS ${LIBRN_DIR}
            NO_CMAKE_FIND_ROOT_PATH
    )
else()
    find_library(
            FOLLY_JSON_LIB
            folly_runtime
            PATHS ${LIBRN_DIR}
            NO_CMAKE_FIND_ROOT_PATH
    )
endif()

find_library(
        REACT_NATIVE_JNI_LIB
        reactnativejni
        PATHS ${LIBRN_DIR}
        NO_CMAKE_FIND_ROOT_PATH
)

# RN 0.66 distributes libjsi.so, can be used instead of compiling jsi.cpp manually.
find_library(
        JSI_LIB
        jsi
        PATHS ${LIBRN_DIR}
        NO_CMAKE_FIND_ROOT_PATH
)


find_library(
        LOG_LIB
        log
)
find_library(z-lib z)

find_package(curl REQUIRED CONFIG)

message(WARNING "JSI-WS linking: CMAKE_CURRENT_SOURCE_DIR=${ANDROID_ABI}")
target_link_libraries(
        ${PACKAGE_NAME}
        ${LOG_LIB}
        ${JSI_LIB}
        ${FBJNI_LIB}
        ${REACT_NATIVE_JNI_LIB}
        curl::curl
        ${z-lib}
        android
)
