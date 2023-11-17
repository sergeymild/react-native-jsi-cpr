#include <jni.h>
#include "react-native-jsi-cpr.h"

extern "C"
JNIEXPORT jdouble JNICALL
Java_com_jsicpr_JsiCprModule_nativeMultiply(JNIEnv *env, jclass type, jdouble a, jdouble b) {
    return jsicpr::multiply(a, b);
}
