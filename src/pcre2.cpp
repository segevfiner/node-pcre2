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
    Napi::Value Test(const Napi::CallbackInfo &info);

    size_t PatternSize(Napi::Env env) const;

    pcre2_code *m_re;
    pcre2_match_data *m_match_data;
    size_t m_size;
};

Napi::Object PCRE2::Init(Napi::Env env, Napi::Object exports) {
    Napi::Function func = DefineClass(env, "PCRE2", {
        InstanceMethod<&PCRE2::Test>("test"),
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
    std::string patternStr = pattern.Utf8Value();

    int errornumber;
    size_t erroroffset;
    m_re = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(patternStr.c_str()),
        patternStr.size(),
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

    m_size = PatternSize(info.Env()) + pcre2_get_match_data_size(m_match_data);
    Napi::MemoryManagement::AdjustExternalMemory(info.Env(), m_size);
}

PCRE2::~PCRE2() {
    pcre2_match_data_free(m_match_data);
    pcre2_code_free(m_re);
    Napi::MemoryManagement::AdjustExternalMemory(Env(), -m_size);
}

Napi::Value PCRE2::Test(const Napi::CallbackInfo &info)
{
    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }
    if (!info[0].IsString()) {
        throw Napi::TypeError::New(info.Env(), "Expected a string");
    }

    Napi::String subject = info[0].As<Napi::String>();
    std::string subjectStr = subject.Utf8Value();

    int rc = pcre2_match(
        m_re,
        reinterpret_cast<PCRE2_SPTR>(subjectStr.c_str()),
        subjectStr.length(),
        0,
        0,
        m_match_data,
        nullptr
    );
    if (rc < 0) {
        if (rc == PCRE2_ERROR_NOMATCH) {
            return Napi::Boolean::New(info.Env(), false);
        } else {
            std::ostringstream oss;
            oss << "PCRE2 matching error:" << rc;
            throw Napi::Error::New(info.Env(), oss.str());
        }
    }

    return Napi::Boolean::New(info.Env(), true);
}

size_t PCRE2::PatternSize(Napi::Env env) const {
    size_t size;
    int rc = pcre2_pattern_info(m_re, PCRE2_INFO_SIZE, &size);
    if (rc < 0) {
        std::ostringstream oss;
        oss << "PCRE2 error:" << rc;
        throw Napi::Error::New(env, oss.str());
    }

    return size;
}

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
    PCRE2::Init(env, exports);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
