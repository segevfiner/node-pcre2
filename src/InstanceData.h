#ifndef NODE_PCRE2_INSTANCE_DATA_H_
#define NODE_PCRE2_INSTANCE_DATA_H_

#include <napi.h>
#include <pcre2.h>

class InstanceData {
public:
    explicit InstanceData(Napi::Env env);

    virtual ~InstanceData();

    InstanceData(const InstanceData&) = delete;
    InstanceData& operator=(const InstanceData&) = delete;

    pcre2_compile_context *compileContext;

    Napi::ObjectReference Symbol;
    Napi::FunctionReference RegExp;
    Napi::FunctionReference ObjectCreate;
    Napi::FunctionReference ArrayPush;

    Napi::FunctionReference PCRE2;
    Napi::FunctionReference PCRE2StringIterator;
};

#endif // NODE_PCRE2_INSTANCE_DATA_H_
