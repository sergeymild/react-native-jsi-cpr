#include "cpr/cpr.h"
#include "JsiHttp.h"
#include "iostream"
#include <curl/curl.h>

using namespace facebook;
using namespace std;

namespace jsiHttp {

JsiHttp::JsiHttp(
        jsi::Runtime *rt,
        std::shared_ptr<facebook::react::CallInvoker> jsCallInvoker)
        : runtime_(rt),
          jsCallInvoker_(std::move(jsCallInvoker)) {
              pool = new ThreadPool();
          }

void JsiHttp::installJSIBindings(std::string cPath, facebook::jsi::Object *jsObject) {
    this->certPath = cPath;

    auto makeHttpRequest = jsi::Function::createFromHostFunction(
            *runtime_,
            jsi::PropNameID::forUtf8(*runtime_, "makeHttpRequest"),
            2,
             [=](jsi::Runtime &runtime,
                const jsi::Value &thisArg,
                const jsi::Value *args,
                size_t count) -> jsi::Value {

                auto requestObject = args[0].asObject(runtime);
                std::string uniqueId = requestObject.getProperty(runtime, "requestId").asString(
                        runtime).utf8(runtime);

                auto jsCallback = args[1].getObject(runtime).asFunction(runtime);
                callbacks[uniqueId] = std::make_shared<jsi::Function>(std::move(jsCallback));

                prepareForRequest(runtime, args);

                return jsi::Value::undefined();

            });

    auto httpCancelRequest = jsi::Function::createFromHostFunction(
            *runtime_,
            jsi::PropNameID::forUtf8(*runtime_, "httpCancelRequest"),
            1,
            [=](jsi::Runtime &runtime,
                const jsi::Value &thisArg,
                const jsi::Value *args,
                size_t count) -> jsi::Value {
                auto uniqueId = args[0].asString(runtime).utf8(runtime);
                callbacks.erase(uniqueId);
                return jsi::Value::undefined();
            });


    jsObject->setProperty(*runtime_, "httpCancelRequest", std::move(httpCancelRequest));
    jsObject->setProperty(*runtime_, "makeHttpRequest", std::move(makeHttpRequest));
}


//MARK: prepareForRequest
void JsiHttp::prepareForRequest(jsi::Runtime &runtime, const jsi::Value *args) {
    auto requestObject = args[0].asObject(runtime);
    auto uniqueId = requestObject.getProperty(runtime, "requestId").asString(runtime).utf8(
            runtime);
    auto m = requestObject.getProperty(runtime, "method").asString(runtime).utf8(runtime);
    auto base = requestObject.getProperty(runtime, "baseUrl").asString(runtime).utf8(runtime);
    auto end = requestObject.getProperty(runtime, "url").asString(runtime).utf8(runtime);
    jsi::Value rawDataType = requestObject.getProperty(runtime, "dataType");
    string dataType = "";
    if (!rawDataType.isUndefined() && !rawDataType.isNull()) {
        dataType = rawDataType.asString(runtime).utf8(runtime);
    }

    cpr::Header cprHeaders;


    auto headerObject = requestObject.getProperty(runtime, "headers");
    if (!headerObject.isUndefined() && headerObject.isObject()) {
        auto headers = headerObject.asObject(runtime);
        jsi::Array propertyNames = headers.getPropertyNames(runtime);
        size_t size = propertyNames.size(runtime);
        for (size_t i = 0; i < size; i++) {
            jsi::String name = propertyNames.getValueAtIndex(runtime, i).getString(runtime);
            jsi::String value = headers.getProperty(runtime, name).asString(runtime);
            cprHeaders[name.utf8(runtime)] = value.utf8(runtime);
        }
    }
    
    if (dataType == "json") {
        cprHeaders["Content-Type"] = "application/json";
    }
    
    if (dataType == "formUrlEncoded") {
        cprHeaders["Content-Type"] = "application/x-www-form-urlencoded";
    }
    
    if (dataType == "string") {
        cprHeaders["Content-Type"] = "text/plain; charset=UTF-8";
    }

    auto timeoutProperty = requestObject.getProperty(runtime, "timeout");
    double timeout = 0;
    if (!timeoutProperty.isUndefined()) {
        timeout = timeoutProperty.asNumber();
    }
    auto skipResponseHeaders = false;
    if (requestObject.hasProperty(runtime, "skipResponseHeaders")) {
        skipResponseHeaders = requestObject.getProperty(runtime, "skipResponseHeaders").getBool();
    }

    auto dataObject = requestObject.getProperty(runtime, "data");
//    std::string dataString;
//    if (!dataObject.isUndefined() && dataObject.isString()) {
//        dataString = dataObject.asString(runtime).utf8(runtime);
//    }

    makeRequest(uniqueId, m, base, end, dataType, cprHeaders, timeout, skipResponseHeaders, std::move(dataObject));
}




//MARK: makeRequest
void JsiHttp::makeRequest(const string& uniqueId,
                          const string& method,
                          const string& baseUrl,
                          const string& endpoint,
                          const std::string& dataType,
                          const cpr::Header& headers,
                          double timeout,
                          bool skipResponseHeaders,
                          jsi::Value&& data) {


    auto full = baseUrl + endpoint;
    
    cpr::Body* body = nullptr;
    std::vector<cpr::Part> multipartParts;
    
    
    if ((method == "POST" || method == "PATCH" || method == "DELETE") && !data.isUndefined()) {
        if (dataType == "json" || dataType == "string") {
            body = new cpr::Body(data.asString(*runtime_).utf8(*runtime_));
        }
        
        if (dataType == "formData") {
            multipartParts = convertJSIObjectToMultipart(*runtime_, data);
        }
    }

    pool->queueWork([=]() {
                std::cout << uniqueId + " queueWork" << std::endl;

                cpr::Timeout time(timeout);
                cpr::Session session;
                session.EnableOrDisableSSLVerification(this->certPath);
        
                session.SetOption(cpr::Url{baseUrl + endpoint});
                session.SetTimeout(time);
                if (body != nullptr) session.SetBody(std::move(*body));
                if (multipartParts.size() > 0) {
                    session.SetMultipart(cpr::Multipart(multipartParts));
                }
                session.SetHeader(headers);
                cpr::Response r = session.ByMethodName(method);
                processRequest(uniqueId, skipResponseHeaders, timeout, r);

                std::cout << uniqueId + " endWork" << std::endl;
            });

}


//MARK: processGetRequest
void JsiHttp::processRequest(std::string uniqueId, bool skipResponseHeaders, double timeout, cpr::Response res) {
    Response response;
    response.uniqueId = std::move(uniqueId);

    response.status = res.status_code;
    response.body = res.text;
    response.skipResponseHeaders = skipResponseHeaders;
    response.elapsed = res.elapsed;
    response.endpoint = res.url.str();

    if (!skipResponseHeaders) {
        response.headers = std::map<std::string, std::string>(res.header.begin(), res.header.end());
    }

    if (!res.error && res.status_code >= 200 && res.status_code < 300) {
        response.type = ResultOk;
        if (!skipResponseHeaders) {
            response.headers = std::map<std::string, std::string>(res.header.begin(),
                                                             res.header.end());
        }
    } else {
        response.type = ResultError;
        response.error = errorCodeToString(res.error);
        if (res.error.code == cpr::ErrorCode::OPERATION_TIMEDOUT) {
            response.status = 504;
        }
    }

    jsCallInvoker_->invokeAsync([=]() {
        jsi::Object responseObject = convertResponseToJsiObject(*runtime_, response);
        auto f = callbacks[response.uniqueId];
        if (!f) return;
        if (response.type == ResultOk) {
            f->call(*runtime_, responseObject, jsi::Value::undefined());
        } else {
            f->call(*runtime_, jsi::Value::undefined(), responseObject);
        }
        callbacks.erase(response.uniqueId);
    });
}


}
