#import "JsiCpr.h"
#import <React/RCTBlobManager.h>
#import <React/RCTUIManager.h>
#import <React/RCTBridge+Private.h>
#import <ReactCommon/RCTTurboModule.h>
#import <jsi/jsi.h>
#import "JsiHttp.h"

@implementation JsiCpr

RCT_EXPORT_MODULE()

jsiHttp::JsiHttp *http;

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install) {
    NSLog(@"Installing JsiCpr polyfill Bindings...");
    auto _bridge = [RCTBridge currentBridge];
    auto _cxxBridge = (RCTCxxBridge*)_bridge;
    if (_cxxBridge == nil) return @false;
    jsi::Runtime* _runtime = (jsi::Runtime*) _cxxBridge.runtime;
    if (_runtime == nil) return @false;
    
    http = new jsiHttp::JsiHttp(_runtime, _bridge.jsCallInvoker);
    http->installJSIBindings("");

    return @true;
}


@end
