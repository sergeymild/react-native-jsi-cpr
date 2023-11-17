#if ONANDROID
#include "android/log.h"
#endif

#include "iostream"


namespace jsiWs {

void log(const std::string& message) {
#if ONANDROID
    __android_log_print(ANDROID_LOG_VERBOSE, "JSI::", "%s", message.c_str());
#else
    std::cout << "JSI:: " << message << std::endl;
#endif
}

}
