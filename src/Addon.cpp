#include <napi.h>
#include <pcre2.h>
#include "InstanceData.h"
#include "PCRE2.h"
#include "PCRE2StringIterator.h"

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
    env.SetInstanceData(new InstanceData(env));
    PCRE2::Init(env, exports);
    PCRE2StringIterator::Init(env, exports);
    exports["PCRE2_MAJOR"] = PCRE2_MAJOR;
    exports["PCRE2_MINOR"] = PCRE2_MINOR;
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
