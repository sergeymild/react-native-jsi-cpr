#import <jsi/jsi.h>
#include "map"
#include "cpr/cpr.h"
#include <curl/curl.h>

using namespace facebook;

namespace jsiHttp {

enum ResultType {
    ResultOk,
    ResultError
};

struct Response {
    ResultType type;
    std::string uniqueId;
    std::string error;
    std::string body;
    std::string endpoint;
    int status;
    double elapsed;
    bool skipResponseHeaders;
    std::map<std::string, std::string> headers;
};

std::string errorCodeToString(const cpr::Error& error) {
    switch (error.code) {
        case cpr::ErrorCode::CONNECTION_FAILURE:
            return "CONNECTION_FAILURE";
        case cpr::ErrorCode::EMPTY_RESPONSE:
            return "EMPTY_RESPONSE";
        case cpr::ErrorCode::GENERIC_SSL_ERROR:
            return "GENERIC_SSL_ERROR";
        case cpr::ErrorCode::HOST_RESOLUTION_FAILURE:
            return "HOST_RESOLUTION_FAILURE";
        case cpr::ErrorCode::INTERNAL_ERROR:
            return "INTERNAL_ERROR";
        case cpr::ErrorCode::INVALID_URL_FORMAT:
            return "INVALID_URL_FORMAT";
        case cpr::ErrorCode::NETWORK_RECEIVE_ERROR:
            return "NETWORK_RECEIVE_ERROR";
        case cpr::ErrorCode::NETWORK_SEND_FAILURE:
            return "NETWORK_SEND_FAILURE";
        case cpr::ErrorCode::OK:
            return "OK";
        case cpr::ErrorCode::OPERATION_TIMEDOUT:
            return "OPERATION_TIMEDOUT";
        case cpr::ErrorCode::PROXY_RESOLUTION_FAILURE:
            return "PROXY_RESOLUTION_FAILURE";
        case cpr::ErrorCode::REQUEST_CANCELLED:
            return "REQUEST_CANCELLED";
        case cpr::ErrorCode::SSL_CACERT_ERROR:
            return "SSL_CACERT_ERROR";
        case cpr::ErrorCode::SSL_CONNECT_ERROR:
            return "SSL_CONNECT_ERROR";
        case cpr::ErrorCode::SSL_LOCAL_CERTIFICATE_ERROR:
            return "SSL_LOCAL_CERTIFICATE_ERROR";
        case cpr::ErrorCode::SSL_REMOTE_CERTIFICATE_ERROR:
            return "SSL_REMOTE_CERTIFICATE_ERROR";
        case cpr::ErrorCode::TOO_MANY_REDIRECTS:
            return "TOO_MANY_REDIRECTS";
        case cpr::ErrorCode::UNKNOWN_ERROR:
            return "UNKNOWN_ERROR";
        case cpr::ErrorCode::UNSUPPORTED_PROTOCOL:
            return "UNSUPPORTED_PROTOCOL";
    }
}

//MARK: convertNSDictionaryToJSIObject
std::vector<cpr::Part>
convertJSIObjectToMultipart(jsi::Runtime &runtime, jsi::Value& data) {
    std::vector<cpr::Part> parts;

    auto dataObject = data.asObject(runtime);
    if (!dataObject.hasProperty(runtime, "formData")) return parts;
    auto rawArray = dataObject.getProperty(runtime, "formData").asObject(runtime);
    if (!rawArray.isArray(runtime)) {
        throw "dataObjectMustBeAnArray";
    }

    auto arrayOfData = rawArray.asArray(runtime);
    //jsi::Array arrayOfDataNames = arrayOfData.getPropertyNames(runtime);
    size_t arrayOfDataSize = arrayOfData.size(runtime);



    for (size_t i = 0; i < arrayOfDataSize; i++) {
        auto dataObject = arrayOfData.getValueAtIndex(runtime, i).asObject(runtime);
        auto name = dataObject.getProperty(runtime, "name").asString(runtime).utf8(runtime);

        // file
        if (dataObject.hasProperty(runtime, "path")) {
            auto path = dataObject.getProperty(runtime, "path").asString(runtime).utf8(runtime);
            // on IOS remove file:// scheme
            if (path.rfind("file://") == 0) path = path.replace(0, 7, "");
            parts.push_back(cpr::Part(name, cpr::File(path)));
        }

        // rest parameters
        if (dataObject.hasProperty(runtime, "value")) {
            auto value = dataObject.getProperty(runtime, "value");
            if (value.isString()) {
                std::string v = value.asString(runtime).utf8(runtime);
                parts.push_back(cpr::Part(name, v));
            }

            if (value.isNumber()) {
                long v = lrint(value.asNumber());
                parts.push_back(cpr::Part(name, v));
            }

            if (value.isBool()) {
                parts.push_back(cpr::Part(name, value.getBool() ? "1" : "0"));
            }
            continue;
        }
    }

    return parts;
}

cpr::Response ByMethodName(const std::string methodName, cpr::Session *session) {
    if (methodName == "GET") {
        return session->Get();
    } else if (methodName == "POST") {
        return session->Post();
    } else if (methodName == "PATCH") {
        return session->Patch();
    } else if (methodName == "PUT") {
        return session->Put();
    } else if (methodName == "DELETE") {
        return session->Delete();
    }
    return session->Get();
}

void EnableOrDisableSSLVerification(const std::string certPath, cpr::Session *session) {
    #if defined(ONANDROID)
        if (!certPath.empty()) {
          curl_easy_setopt(session->GetCurlHolder().get()->handle, CURLOPT_CAINFO, certPath.c_str());
        } else {
          curl_easy_setopt(session->GetCurlHolder().get()->handle, CURLOPT_SSL_VERIFYPEER, 0L);
          curl_easy_setopt(session->GetCurlHolder().get()->handle, CURLOPT_SSL_VERIFYHOST, 0L);
        }
    #endif
}

cpr::Body* prepareRequestBody(jsi::Runtime &runtime, jsi::Value& data, cpr::Header *headers) {
    cpr::Body* body = nullptr;

    jsi::Object dataObject = data.asObject(runtime);

    if (dataObject.hasProperty(runtime, "json")) {
        auto json = dataObject.getProperty(runtime, "json");
        if (!json.isUndefined() && !json.isNull()) {
            body = new cpr::Body(json.asString(runtime).utf8(runtime));
            (*headers)["Content-Type"] = "application/json";
        }
    }

    if (dataObject.hasProperty(runtime, "string")) {
        auto string = dataObject.getProperty(runtime, "string");
        if (!string.isUndefined() && !string.isNull()) {
            body = new cpr::Body(string.asString(runtime).utf8(runtime));
        }
    }

    if (dataObject.hasProperty(runtime, "formUrlEncoded")) {
        auto formUrlEncoded = dataObject.getProperty(runtime, "formUrlEncoded");
        if (!formUrlEncoded.isUndefined() && !formUrlEncoded.isNull()) {
            body = new cpr::Body(formUrlEncoded.asString(runtime).utf8(runtime));
            //(*headers)["Content-Type"] = "application/json";
        }
    }

    return body;
}


//MARK: convertNSDictionaryToJSIObject
jsi::Object
convertNSDictionaryToJSIObject(jsi::Runtime &runtime, std::map<std::string, std::string> value) {
    jsi::Object result = jsi::Object(runtime);

    std::map<std::string, std::string>::iterator it = value.begin();

    while (it != value.end()) {
        std::string key = it->first;
        std::string v = it->second;
        result.setProperty(runtime, jsi::String::createFromUtf8(runtime, key),
                           jsi::String::createFromUtf8(runtime, v));

        it++;
    }

    return result;
}


//MARK: convertResponseToJsiObject
facebook::jsi::Object
convertResponseToJsiObject(facebook::jsi::Runtime &runtime, const Response& response) {
    jsi::Object responseObject = jsi::Object(runtime);
    responseObject.setProperty(runtime, "status", response.status);
    responseObject.setProperty(runtime, "data",
                               jsi::String::createFromUtf8(runtime, response.body));
    responseObject.setProperty(runtime, "elapsed", jsi::Value(response.elapsed));
    responseObject.setProperty(runtime, "requestId",
                               jsi::String::createFromUtf8(runtime, response.uniqueId));
    responseObject.setProperty(runtime, "endpoint",
                               jsi::String::createFromUtf8(runtime, response.endpoint));

    if (!response.skipResponseHeaders) {
        jsi::Object headersObject = convertNSDictionaryToJSIObject(runtime,
                                                                   response.headers);
        responseObject.setProperty(runtime, "headers", headersObject);
    }

    if (response.type == ResultOk) {
        responseObject.setProperty(runtime, "type", "success");
    } else {
        responseObject.setProperty(runtime, "error",
                                   jsi::String::createFromUtf8(runtime, response.error));
        responseObject.setProperty(runtime, "type", "error");
    }


    return responseObject;
}


}
