#pragma once
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

std::string errorCodeToString(const cpr::Error& error);
jsi::Object convertNSDictionaryToJSIObject(jsi::Runtime &runtime, std::map<std::string, std::string> value);
facebook::jsi::Object convertResponseToJsiObject(facebook::jsi::Runtime &runtime, const Response& response);
std::vector<cpr::Part> convertJSIObjectToMultipart(jsi::Runtime &runtime, jsi::Value& data);

cpr::Body* prepareRequestBody(jsi::Runtime &runtime, jsi::Value& data, cpr::Header *headers);
cpr::Response ByMethodName(const std::string methodName, cpr::Session *session);
void EnableOrDisableSSLVerification(const std::string certPath, cpr::Session *session);

}
