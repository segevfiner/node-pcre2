#include "InstanceData.h"
#include "PCRE2.h"
#include "PCRE2StringIterator.h"

Napi::Object PCRE2StringIterator::Init(Napi::Env env, Napi::Object exports)
{
    InstanceData *instanceData = env.GetInstanceData<InstanceData>();

    Napi::Function ctor = DefineClass(env, "Object [PCRE2 String Iterator]", {
        InstanceMethod<&PCRE2StringIterator::Iterator>(instanceData->Symbol.Get("iterator").As<Napi::Symbol>()),
        InstanceMethod<&PCRE2StringIterator::Next>("next"),
    });

    instanceData->PCRE2StringIterator = Napi::Persistent(ctor);

    return exports;
}

PCRE2StringIterator::PCRE2StringIterator(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<PCRE2StringIterator>(info)
{
    m_pcre2Ref = Napi::Persistent(info[0].As<Napi::Object>());
    m_pcre2 = PCRE2::Unwrap(m_pcre2Ref.Value());

    m_private = Napi::Persistent(Napi::Object::New(info.Env()));
    m_private.Set("subject", info[1]);
    m_subject = info[1].As<Napi::String>().Utf16Value();
}

PCRE2StringIterator::~PCRE2StringIterator()
{
}

Napi::Value PCRE2StringIterator::Iterator(const Napi::CallbackInfo &info)
{
    return Value();
}

Napi::Value PCRE2StringIterator::Next(const Napi::CallbackInfo &info)
{
    Napi::Value value = m_pcre2->ExecImpl(info.Env(), m_private.Get("subject").As<Napi::String>());

    m_pcre2->HandleEmptyMatch(info.Env(), value.As<Napi::Array>(), m_subject);

    Napi::Object result = Napi::Object::New(info.Env());
    if (!value.IsNull()) {
        result["value"] = value;
    } else {
        result["done"] = Napi::Boolean::New(info.Env(), true);
    }

    return result;
}
