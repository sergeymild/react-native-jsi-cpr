#ifdef __cplusplus
#import "react-native-jsi-cpr.h"
#endif

#ifdef RCT_NEW_ARCH_ENABLED
#import "RNJsiCprSpec.h"

@interface JsiCpr : NSObject <NativeJsiCprSpec>
#else
#import <React/RCTBridgeModule.h>

@interface JsiCpr : NSObject <RCTBridgeModule>
#endif

@end
