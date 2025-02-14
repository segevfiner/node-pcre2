#ifndef NODE_PCRE2_MATCH_ALL_ITERATOR_H_
#define NODE_PCRE2_MATCH_ALL_ITERATOR_H_

#include <napi.h>

class PCRE2;

class PCRE2StringIterator : public Napi::ObjectWrap<PCRE2StringIterator> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    explicit PCRE2StringIterator(const Napi::CallbackInfo &info);
    virtual ~PCRE2StringIterator();

    PCRE2StringIterator(const PCRE2StringIterator&) = delete;
    PCRE2StringIterator& operator=(const PCRE2StringIterator&) = delete;

private:
    Napi::Value Iterator(const Napi::CallbackInfo &info);
    Napi::Value Next(const Napi::CallbackInfo &info);

    Napi::ObjectReference m_private;
    std::u16string m_subject;
    Napi::ObjectReference m_pcre2Ref;
    PCRE2 *m_pcre2;
};

#endif // NODE_PCRE2_MATCH_ALL_ITERATOR_H_
