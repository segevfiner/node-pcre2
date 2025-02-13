#include "InstanceData.h"
#include "PCRE2.h"
#include "MatchAllIterator.h"

Napi::Object MatchAllIterator::Init(Napi::Env env, Napi::Object exports)
{
    InstanceData *instanceData = env.GetInstanceData<InstanceData>();

    Napi::Function ctor = DefineClass(env, "MatchAllIterator", {
        InstanceMethod<&MatchAllIterator::Iterator>(instanceData->Symbol.Get("iterator").As<Napi::Symbol>()),
        InstanceAccessor<&MatchAllIterator::Next>("next"),
    });

    instanceData->MatchAllIterator = Napi::Persistent(ctor);

    return exports;
}

MatchAllIterator::MatchAllIterator(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<MatchAllIterator>(info)
{
    m_pcre2Ref = Napi::Persistent(info[0].As<Napi::Object>());
    m_pcre2 = PCRE2::Unwrap(m_pcre2Ref.Value());

    Value().Set("subject", info[1]);
}

MatchAllIterator::~MatchAllIterator()
{

}

Napi::Value MatchAllIterator::Iterator(const Napi::CallbackInfo &info)
{
    return Value();
}

Napi::Value MatchAllIterator::Next(const Napi::CallbackInfo &info)
{
    Napi::Value result = m_pcre2->ExecImpl(info.Env(), Value().Get("subject").As<Napi::String>());

    // TODO Handle empty matches

    return result;
}
