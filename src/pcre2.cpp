#include <napi.h>
#include <pcre2.h>
#include <sstream>

class PCRE2 : public Napi::ObjectWrap<PCRE2> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    explicit PCRE2(const Napi::CallbackInfo &info);
    virtual ~PCRE2();

    PCRE2(const PCRE2&) = delete;
    PCRE2& operator=(const PCRE2&) = delete;

private:
    pcre2_code *m_re;
    pcre2_match_data *m_match_data;
};

Napi::Object PCRE2::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "PCRE2", {

    });

    exports.Set("PCRE2", func);

    return exports;
}

PCRE2::PCRE2(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<PCRE2>(info)
{
    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }
    if (!info[0].IsString()) {
        throw Napi::TypeError::New(info.Env(), "Expected a string");
    }

    Napi::String pattern = info[0].As<Napi::String>();

    int errornumber;
    size_t erroroffset;
    m_re = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(pattern.Utf8Value().c_str()),
        pattern.Utf8Value().size(),
        PCRE2_UTF,
        &errornumber,
        &erroroffset,
        nullptr
    );
    if (m_re == nullptr) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        std::ostringstream oss;
        oss << "PCRE2 compilation failed at offset" << erroroffset << ": " << buffer;
        throw Napi::Error::New(info.Env(), oss.str());
    }

    m_match_data = pcre2_match_data_create_from_pattern(m_re, nullptr);
    if (m_match_data == nullptr) {
        pcre2_code_free(m_re);
        throw Napi::Error::New(info.Env(), "PCRE2 match data allocation failed");
    }

    // TODO
    // Napi::MemoryManagement::AdjustExternalMemory(info.Env(), )
}

PCRE2::~PCRE2() {
    pcre2_match_data_free(m_match_data);
    pcre2_code_free(m_re);
}

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
    PCRE2::Init(env, exports);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
