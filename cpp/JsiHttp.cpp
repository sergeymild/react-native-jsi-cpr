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
          jsCallInvoker_(std::move(jsCallInvoker)) {}

void JsiHttp::installJSIBindings(std::string cPath) {
    this->certPath = cPath;
    
    //MARK: makeHttpRequest
    auto makeHttpRequest = jsi::Function::createFromHostFunction(
            *runtime_,
            jsi::PropNameID::forUtf8(*runtime_, "makeHttpRequest"),
            2,
             [=](jsi::Runtime &runtime,
                const jsi::Value &thisArg,
                const jsi::Value *args,
                size_t count
            ) -> jsi::Value {

                auto requestObject = args[0].asObject(runtime);
                std::string uniqueId = requestObject.getProperty(runtime, "requestId").asString(
                        runtime).utf8(runtime);

                auto jsCallback = args[1].getObject(runtime).asFunction(runtime);
                callbacks[uniqueId] = std::make_shared<jsi::Function>(std::move(jsCallback));

                prepareForRequest(runtime, args);

                return jsi::Value::undefined();

            });

    //MARK: httpCancelRequest
    auto httpCancelRequest = jsi::Function::createFromHostFunction(
            *runtime_,
            jsi::PropNameID::forUtf8(*runtime_, "httpCancelRequest"),
            1,
            [=](jsi::Runtime &runtime,
                const jsi::Value &thisArg,
                const jsi::Value *args,
                size_t count) -> jsi::Value {
                auto uniqueId = args[0].asString(runtime).utf8(runtime);
                std::cout << uniqueId + " cancelRequest" << std::endl;
                    
                Response response;
                response.uniqueId = std::move(uniqueId);
                response.error = "REQUEST_CANCELLED";
                response.type = ResultError;
                response.status = 80;
                sendToJS(response);
                return jsi::Value::undefined();
            });


    jsi::Object jsObject = jsi::Object(*runtime_);
    jsObject.setProperty(*runtime_, "httpCancelRequest", std::move(httpCancelRequest));
    jsObject.setProperty(*runtime_, "makeHttpRequest", std::move(makeHttpRequest));
    runtime_->global().setProperty(*runtime_, "jsiHttp", std::move(jsObject));
}


//MARK: prepareForRequest
void JsiHttp::prepareForRequest(jsi::Runtime &runtime, const jsi::Value *args) {
    auto requestObject = args[0].asObject(runtime);
    auto uniqueId = requestObject.getProperty(runtime, "requestId").asString(runtime).utf8(
            runtime);
    auto m = requestObject.getProperty(runtime, "method").asString(runtime).utf8(runtime);
    auto base = requestObject.getProperty(runtime, "baseUrl").asString(runtime).utf8(runtime);
    auto end = requestObject.getProperty(runtime, "url").asString(runtime).utf8(runtime);

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

    makeRequest(uniqueId, m, base, end, &cprHeaders, timeout, skipResponseHeaders, std::move(dataObject));
}




//MARK: makeRequest
void JsiHttp::makeRequest(const string& uniqueId,
                          const string& method,
                          const string& baseUrl,
                          const string& endpoint,
                          cpr::Header *headers,
                          double timeout,
                          bool skipResponseHeaders,
                          jsi::Value&& data) {


    auto full = baseUrl + endpoint;

    cpr::Body* body = nullptr;
    std::vector<cpr::Part> multipartParts;


    if ((method == "POST" || method == "PATCH" || method == "DELETE" || method == "PUT") && !data.isUndefined() && !data.isNull()) {
        //body = new cpr::Body(data.asString(*runtime_).utf8(*runtime_));
        body = jsiHttp::prepareRequestBody(*runtime_, data, headers);
        if (body == nullptr) {
            multipartParts = convertJSIObjectToMultipart(*runtime_, data);
        }
    }
    
    cpr::Timeout time(timeout);
    std::shared_ptr<cpr::Session> session = std::make_shared<cpr::Session>();
    jsiHttp::EnableOrDisableSSLVerification(this->certPath, session);
    
    session->SetOption(cpr::Url{baseUrl + endpoint});
    session->SetTimeout(time);
    if (body != nullptr) session->SetBody(std::move(*body));
    if (multipartParts.size() > 0) {
        session->SetMultipart(cpr::Multipart(multipartParts));
    }
    session->SetHeader(*headers);
    
    auto callback = [=, h = *headers](cpr::Response r) {
        std::cout << uniqueId + " queueWork" << std::endl;

        // do not try invoke callback if request was cancelled;
        if (!callbacks[uniqueId]) {
            std::cout << uniqueId + " was cancelled" << std::endl;
            return;
        }

        processRequest(uniqueId, skipResponseHeaders, r);

        std::cout << uniqueId + " endWork" << std::endl;
    };
    
    if (method == "GET") {
        session->GetCallback(callback);
    } else if (method == "POST") {
        session->PostCallback(callback);
    } else if (method == "PATCH") {
        session->PatchCallback(callback);
    } else if (method == "PUT") {
        session->PutCallback(callback);
    } else if (method == "DELETE") {
        session->DeleteCallback(callback);
    } else {
        session->GetCallback(callback);
    }
}

void JsiHttp::sendToJS(Response res) {
    jsCallInvoker_->invokeAsync([=]() {
        jsi::Object responseObject = convertResponseToJsiObject(*runtime_, res);
        auto f = callbacks[res.uniqueId];
        if (!f) return;
        if (res.type == ResultOk) {
            f->call(*runtime_, responseObject, jsi::Value::undefined());
        } else {
            f->call(*runtime_, jsi::Value::undefined(), responseObject);
        }
        callbacks.erase(res.uniqueId);
    });
}


//MARK: processGetRequest
void JsiHttp::processRequest(std::string uniqueId, bool skipResponseHeaders, cpr::Response res) {
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
        if (res.error.code == cpr::ErrorCode::OPERATION_TIMEDOUT) response.status = 90;
        if (res.error.code == cpr::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR) response.status = 70;
        if (res.error.code == cpr::ErrorCode::SSL_CACERT_ERROR) response.status = 71;
        if (res.error.code == cpr::ErrorCode::SSL_CONNECT_ERROR) response.status = 72;
        if (res.error.code == cpr::ErrorCode::SSL_LOCAL_CERTIFICATE_ERROR) response.status = 73;
        if (res.error.code == cpr::ErrorCode::GENERIC_SSL_ERROR) response.status = 74;
    }

    sendToJS(response);
}


} //namespace jsiHttp
