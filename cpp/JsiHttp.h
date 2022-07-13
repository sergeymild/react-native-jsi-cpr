#import <jsi/jsi.h>
#import "ThreadPool.h"
#include "map"
#include <ReactCommon/CallInvoker.h>
#include "JsiUtils.h"

using namespace facebook;
namespace jsiHttp {


typedef std::function<void(Response)> HttpCallback;

class JsiHttp {
    
public:
    void installJSIBindings(std::string certPath);
    explicit JsiHttp(
                jsi::Runtime *rt,
                std::shared_ptr<facebook::react::CallInvoker> jsCallInvoker);
    
    
private:
    ThreadPool *pool;
    std::map<std::string, std::shared_ptr<jsi::Function>> callbacks;
    std::shared_ptr<react::CallInvoker> jsCallInvoker_;
    std::string certPath;
    facebook::jsi::Runtime *runtime_;
    
    void prepareForRequest(jsi::Runtime& runtime, const jsi::Value* args);
    void processRequest(std::string uniqueId, bool skipResponseHeaders, cpr::Response res);
    void sendToJS(Response res);
    void makeRequest(
                     const std::string& uniqueId,
                     const std::string& method,
                     const std::string& baseUrl,
                     const std::string& endpoint,
                     cpr::Header *headers,
                     double timeout,
                     bool skipResponseHeaders,
                     jsi::Value&& data);
    
};


}
