//
// Created by Sergei Golishnikov on 06/03/2022.
//

#include <fbjni/fbjni.h>
#include <jsi/jsi.h>
#include <ReactCommon/CallInvokerHolder.h>
#include <map>
#include "JsiHttp.h"

class HttpHelper : public facebook::jni::HybridClass<HttpHelper> {

public:
    static constexpr auto kJavaDescriptor = "Lcom/reactnativejsicpr/HttpHelper;";
    static facebook::jni::local_ref<jhybriddata> initHybrid(
            facebook::jni::alias_ref<jhybridobject> jThis,
            jlong jsContext,
            facebook::jni::alias_ref<facebook::react::CallInvokerHolder::javaobject> jsCallInvokerHolder);

    static void registerNatives();

    void installJSIBindings(jstring certPath);

private:
    friend HybridBase;
    facebook::jni::global_ref<HttpHelper::javaobject> javaPart_;
    facebook::jsi::Runtime *runtime_;
    jsiHttp::JsiHttp *http;
    std::shared_ptr<facebook::react::CallInvoker> jsCallInvoker_;
    std::map<std::string, std::shared_ptr<facebook::jsi::Function>> webSocketCallbacks_;
    explicit HttpHelper(
            facebook::jni::alias_ref<HttpHelper::jhybridobject> jThis,
            facebook::jsi::Runtime *rt,
            std::shared_ptr<facebook::react::CallInvoker> jsCallInvoker);
};


