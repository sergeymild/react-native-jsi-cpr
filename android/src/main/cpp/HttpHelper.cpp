//
// Created by Sergei Golishnikov on 06/03/2022.
//

#include "HttpHelper.h"
#include <react/jni/WritableNativeMap.h>

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
    __android_log_print(ANDROID_LOG_VERBOSE, "😇", "registerNatives");
    registerHybrid({
                           makeNativeMethod("initHybrid",
                                            HttpHelper::initHybrid),
                           makeNativeMethod("installJSIBindings",
                                            HttpHelper::installJSIBindings),
                   });
}





void HttpHelper::installJSIBindings(jstring certPath) {
    __android_log_print(ANDROID_LOG_VERBOSE, "😇", "registerJsiBindings");
    jsi::Object webSocketsObject = jsi::Object(*runtime_);
    //std::shared_ptr<jsi::Object> sharedObj = std::make_shared<jsi::Object>(webSocketsObject);

    http = new jsiHttp::JsiHttp(runtime_, jsCallInvoker_);
    http->installJSIBindings(make_local(certPath)->toStdString(), &webSocketsObject);

    runtime_->global().setProperty(*runtime_, "jsiHttp", std::move(webSocketsObject));
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

    __android_log_write(ANDROID_LOG_INFO, "🥲", "initHybrid...");
    auto jsCallInvoker = jsCallInvokerHolder->cthis()->getCallInvoker();
    return makeCxxInstance(jThis, (jsi::Runtime *) jsContext, jsCallInvoker);
}
