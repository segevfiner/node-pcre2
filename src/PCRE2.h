#ifndef NODE_PCRE2_PCRE2_H_
#define NODE_PCRE2_PCRE2_H_

#include <napi.h>
#include <pcre2.h>

class PCRE2 : public Napi::ObjectWrap<PCRE2> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    explicit PCRE2(const Napi::CallbackInfo &info);
    virtual ~PCRE2();

    Napi::Value ExecImpl(Napi::Env env, const Napi::String &subject, uint32_t options = 0);
    size_t AdvanceStringIndex(const std::u16string &subjectStr, size_t index);
    bool Global() const;
    bool PCRE2Mode() const;
    size_t LastIndex() const;
    void SetLastIndex(size_t lastIndex);

    PCRE2(const PCRE2&) = delete;
    PCRE2& operator=(const PCRE2&) = delete;

private:
    Napi::Value Exec(const Napi::CallbackInfo &info);
    Napi::Value Test(const Napi::CallbackInfo &info);
    Napi::Value ToString(const Napi::CallbackInfo &info);
    Napi::Value Match(const Napi::CallbackInfo &info);
    Napi::Value Search(const Napi::CallbackInfo &info);
    Napi::Value Split(const Napi::CallbackInfo &info);
    Napi::Value MatchAll(const Napi::CallbackInfo &info);
    Napi::Value Replace(const Napi::CallbackInfo &info);
    Napi::Value GetLastIndex(const Napi::CallbackInfo &info);
    void SetLastIndex(const Napi::CallbackInfo &info, const Napi::Value &value);
    Napi::Value Source(const Napi::CallbackInfo &info);
    Napi::Value Flags(const Napi::CallbackInfo &info);
    Napi::Value DotAll(const Napi::CallbackInfo &info);
    Napi::Value Global(const Napi::CallbackInfo &info);
    Napi::Value HasIndices(const Napi::CallbackInfo &info);
    Napi::Value IgnoreCase(const Napi::CallbackInfo &info);
    Napi::Value Multiline(const Napi::CallbackInfo &info);
    Napi::Value Sticky(const Napi::CallbackInfo &info);
    Napi::Value Unicode(const Napi::CallbackInfo &info);
    Napi::Value UnicodeSets(const Napi::CallbackInfo &info);
    Napi::Value Extended(const Napi::CallbackInfo &info);
    Napi::Value ExtendedMore(const Napi::CallbackInfo &info);
    Napi::Value NoAutoCapture(const Napi::CallbackInfo &info);
    Napi::Value AsciiBsd(const Napi::CallbackInfo &info);
    Napi::Value AsciiBss(const Napi::CallbackInfo &info);
    Napi::Value AsciiBsw(const Napi::CallbackInfo &info);
    Napi::Value AsciiDigit(const Napi::CallbackInfo &info);
    Napi::Value AsciiPosix(const Napi::CallbackInfo &info);
    Napi::Value CaselessRestrict(const Napi::CallbackInfo &info);
    Napi::Value Dupnames(const Napi::CallbackInfo &info);
    Napi::Value Ungreedy(const Napi::CallbackInfo &info);
    Napi::Value PCRE2Mode(const Napi::CallbackInfo &info);

    static Napi::Value Species(const Napi::CallbackInfo &info);
    static Napi::Function SpeciesConstructor(Napi::Env env, const Napi::Object &obj, const Napi::Function &defaultConstructor);

    void ParseFlags(Napi::Env env, const std::string &flags);
    size_t PatternSize(Napi::Env env) const;
    void AdjustMatchDataHeapFramesSize(Napi::Env env);

    void TierUpTick(Napi::Env env);

    std::u16string m_pattern;
    std::string m_flags;
    uint32_t m_options;
    uint32_t m_extraOptions;
    bool m_global;
    bool m_sticky;
    bool m_hasIndices;
    bool m_pcre2;
    pcre2_code *m_re;
    pcre2_match_data *m_matchData;
    size_t m_lastIndex;
    int m_tierUpTicks;
    bool m_utf8;
    bool m_crlfIsNewline;
    size_t m_size;
    size_t m_matchDataHeapframesSize;
};

#endif // NODE_PCRE2_PCRE2_H_
