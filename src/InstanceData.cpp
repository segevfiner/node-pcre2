#include "InstanceData.h"

InstanceData::InstanceData(Napi::Env env) {
    compileContext = pcre2_compile_context_create(nullptr);
    pcre2_set_newline(compileContext, PCRE2_NEWLINE_ANYCRLF);
    pcre2_set_compile_extra_options(compileContext, PCRE2_EXTRA_ALT_BSUX);

    Symbol = Napi::Persistent(env.Global().Get("Symbol").As<Napi::Object>());
    RegExp = Napi::Persistent(env.Global().Get("RegExp").As<Napi::Function>());
    ObjectCreate = Napi::Persistent(env.Global().Get("Object").As<Napi::Object>().Get("create").As<Napi::Function>());
    ArrayPush = Napi::Persistent(env.Global().Get("Array").As<Napi::Function>().Get("prototype").As<Napi::Object>().Get("push").As<Napi::Function>());
}

InstanceData::~InstanceData() {
    pcre2_compile_context_free(compileContext);
}
