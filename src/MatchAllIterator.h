#ifndef NODE_PCRE2_MATCH_ALL_ITERATOR_H_
#define NODE_PCRE2_MATCH_ALL_ITERATOR_H_

#include <napi.h>

class PCRE2;

class MatchAllIterator : public Napi::ObjectWrap<MatchAllIterator> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    explicit MatchAllIterator(const Napi::CallbackInfo &info);
    virtual ~MatchAllIterator();

    MatchAllIterator(const MatchAllIterator&) = delete;
    MatchAllIterator& operator=(const MatchAllIterator&) = delete;

private:
    Napi::Value Iterator(const Napi::CallbackInfo &info);
    Napi::Value Next(const Napi::CallbackInfo &info);

    Napi::ObjectReference m_pcre2Ref;
    PCRE2 *m_pcre2;
};

#endif // NODE_PCRE2_MATCH_ALL_ITERATOR_H_
