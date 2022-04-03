//
// Created by Sergei Golishnikov on 06/03/2022.
//

#include "HttpHelper.h"

#include <utility>
#include "iostream"

using namespace facebook;
using namespace facebook::jni;

using TSelf = local_ref<HybridClass<HttpHelper>::jhybriddata>;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *) {
    return facebook::jni::initialize(vm, [] {
        HttpHelper::registerNatives();
    });
}

// JNI binding
void HttpHelper::registerNatives() {
    registerHybrid({
                           makeNativeMethod("initHybrid",
                                            HttpHelper::initHybrid),
                           makeNativeMethod("installJSIBindings",
                                            HttpHelper::installJSIBindings),
                   });
}





void HttpHelper::installJSIBindings(jstring certPath) {
    http = new jsiHttp::JsiHttp(runtime_, jsCallInvoker_);
    http->installJSIBindings(make_local(certPath)->toStdString());
}


HttpHelper::HttpHelper(
        jni::alias_ref<HttpHelper::javaobject> jThis,
        jsi::Runtime *rt,
        std::shared_ptr<facebook::react::CallInvoker> jsCallInvoker)
        : javaPart_(jni::make_global(jThis)),
          runtime_(rt),
          jsCallInvoker_(std::move(jsCallInvoker)) {}

// JNI init
TSelf HttpHelper::initHybrid(
        alias_ref<jhybridobject> jThis,
        jlong jsContext,
        jni::alias_ref<facebook::react::CallInvokerHolder::javaobject>
        jsCallInvokerHolder) {

    auto jsCallInvoker = jsCallInvokerHolder->cthis()->getCallInvoker();
    return makeCxxInstance(jThis, (jsi::Runtime *) jsContext, jsCallInvoker);
}
