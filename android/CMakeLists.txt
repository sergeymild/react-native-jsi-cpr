cmake_minimum_required(VERSION 3.10)

set (CMAKE_VERBOSE_MAKEFILE ON)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_FLAGS "-DFOLLY_NO_CONFIG=1 -DFOLLY_HAVE_CLOCK_GETTIME=1 -DFOLLY_HAVE_MEMRCHR=1 -DFOLLY_USE_LIBCPP=1 -DFOLLY_MOBILE=1 -DON_ANDROID -DONANDROID -DFOR_HERMES=${FOR_HERMES}")

set (PACKAGE_NAME "react-native-jsi-cpr")
set (BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set (CPPHTTPLIB_ZLIB_SUPPORT 0)
set (RN_SO_DIR ${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/jni/first-party/react/jni)

if(${REACT_NATIVE_VERSION} LESS 66)
        set (
                INCLUDE_JSI_CPP
                "${NODE_MODULES_DIR}/react-native/ReactCommon/jsi/jsi/jsi.cpp"
        )
        set (
                INCLUDE_JSIDYNAMIC_CPP
                "${NODE_MODULES_DIR}/react-native/ReactCommon/jsi/jsi/JSIDynamic.cpp"
        )
endif()

add_library(
        ${PACKAGE_NAME}
        SHARED
        src/main/cpp/HttpHelper.cpp
        ../cpp/ThreadPool.cpp
        ../cpp/JsiHttp.cpp
        ../cpp/JsiUtils.cpp
        ../cpp/Logger.cpp

        ../cpp/cpr/auth.cpp
        ../cpp/cpr/bearer.cpp
        ../cpp/cpr/cookies.cpp
        ../cpp/cpr/cprtypes.cpp
        ../cpp/cpr/curl_container.cpp
        ../cpp/cpr/curlholder.cpp
        ../cpp/cpr/error.cpp
        ../cpp/cpr/multipart.cpp
        ../cpp/cpr/parameters.cpp
        ../cpp/cpr/payload.cpp
        ../cpp/cpr/proxies.cpp
        ../cpp/cpr/proxyauth.cpp
        ../cpp/cpr/redirect.cpp
        ../cpp/cpr/response.cpp
        ../cpp/cpr/session.cpp
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
        # --- Third Party (required by RN) ---
        "${BUILD_DIR}/third-party-ndk/boost"
        "${BUILD_DIR}/third-party-ndk/double-conversion"
        "${BUILD_DIR}/third-party-ndk/folly"
        "${BUILD_DIR}/third-party-ndk/glog"
        # --- React Native ---
        "${NODE_MODULES_DIR}/react-native/React"
        "${NODE_MODULES_DIR}/react-native/React/Base"
        "${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/jni"
        "${NODE_MODULES_DIR}/react-native/ReactAndroid/src/main/java/com/facebook/react/turbomodule/core/jni"
        "${NODE_MODULES_DIR}/react-native/ReactCommon"
        "${NODE_MODULES_DIR}/react-native/ReactCommon/callinvoker"
        "${NODE_MODULES_DIR}/react-native/ReactCommon/jsi"
        "${NODE_MODULES_DIR}/hermes-engine/android/include/"
        ${INCLUDE_JSI_CPP} # only on older RN versions
        ${INCLUDE_JSIDYNAMIC_CPP} # only on older RN versions
        "src/main/cpp"
        "../cpp"
)

include_directories(
        "../cpp/include"
)

# find libraries

file (GLOB LIBRN_DIR "${BUILD_DIR}/react-native-0*/jni/${ANDROID_ABI}")

if(${FOR_HERMES})
    file (GLOB LIBHERMES_DIR "${BUILD_DIR}/third-party-ndk/hermes/jni/${ANDROID_ABI}")
    # Use Hermes
    find_library(
            JS_ENGINE_LIB
            hermes
            PATHS ${LIBHERMES_DIR}
            NO_CMAKE_FIND_ROOT_PATH
    )

else()
    file (GLOB LIBJSC_DIR "${BUILD_DIR}/android-jsc*.aar/jni/${ANDROID_ABI}")
    # Use JSC
    find_library(
            JS_ENGINE_LIB
            jscexecutor
            PATHS ${LIBRN_DIR}
            NO_CMAKE_FIND_ROOT_PATH
    )

endif()

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
if(${REACT_NATIVE_VERSION} LESS 66)
        # JSI lib didn't exist on RN 0.65 and before. Simply omit it.
        set (JSI_LIB "")
else()
        # RN 0.66 distributes libjsi.so, can be used instead of compiling jsi.cpp manually.
        find_library(
                JSI_LIB
                jsi
                PATHS ${LIBRN_DIR}
                NO_CMAKE_FIND_ROOT_PATH
        )
endif()


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
        ${JS_ENGINE_LIB} # <-- Hermes or JSC
        ${REACT_NATIVE_JNI_LIB}
        ${FBJNI_LIB}
        ${FOLLY_JSON_LIB}
        curl::curl
        ${z-lib}
        android
)
