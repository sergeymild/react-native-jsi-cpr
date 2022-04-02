#import <jsi/jsi.h>
#include "map"
#include "cpr/cpr.h"

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
    auto dataObject = data.asObject(runtime);
    jsi::Array propertyNames = dataObject.getPropertyNames(runtime);
    size_t size = propertyNames.size(runtime);

    std::vector<cpr::Part> parts;
    for (size_t i = 0; i < size; i++) {
        jsi::String name = propertyNames.getValueAtIndex(runtime, i).getString(runtime);
        auto c_name = name.utf8(runtime);
        
        jsi::Value value = dataObject.getProperty(runtime, name);
        if (value.isUndefined() || value.isNull()) continue;
        
        if (c_name == "file") {
            jsi::Object fileObject = value.asObject(runtime);
            std::string fileName = fileObject.getProperty(runtime, "name").asString(runtime).utf8(runtime);
            std::string filePath = fileObject.getProperty(runtime, "path").asString(runtime).utf8(runtime);
            // on IOS remove file:// scheme
            if (filePath.rfind("file://") == 0) {
                filePath = filePath.replace(0, 7, "");
            }
            parts.push_back(cpr::Part(fileName, cpr::File(filePath)));
            continue;
        }
        
        if (value.isString()) {
            std::string v = value.asString(runtime).utf8(runtime);
            parts.push_back(cpr::Part(c_name, v));
        }
        
        if (value.isNumber()) {
            long v = lrint(value.asNumber());
            parts.push_back(cpr::Part(c_name, v));
        }
        
        if (value.isBool()) {
            parts.push_back(cpr::Part(c_name, value.getBool() ? "1" : "0"));
        }
    }

    return parts;
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
