#include <napi.h>
#include <pcre2.h>
#include <sstream>

class InstanceData {
public:
    explicit InstanceData(Napi::Env env);

    virtual ~InstanceData() {}

    InstanceData(const InstanceData&) = delete;
    InstanceData& operator=(const InstanceData&) = delete;

    Napi::FunctionReference RegExp;

    Napi::FunctionReference PCRE2;
};

InstanceData::InstanceData(Napi::Env env) {
    RegExp = Napi::Persistent(env.Global().Get("RegExp").As<Napi::Function>());
}

class PCRE2 : public Napi::ObjectWrap<PCRE2> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    explicit PCRE2(const Napi::CallbackInfo &info);
    virtual ~PCRE2();

    PCRE2(const PCRE2&) = delete;
    PCRE2& operator=(const PCRE2&) = delete;

private:
    Napi::Value Test(const Napi::CallbackInfo &info);
    Napi::Value ToString(const Napi::CallbackInfo &info);
    Napi::Value Source(const Napi::CallbackInfo &info);
    Napi::Value Flags(const Napi::CallbackInfo &info);

    size_t PatternSize(Napi::Env env) const;

    std::u16string m_pattern;
    std::string m_flags;
    pcre2_code *m_re;
    pcre2_match_data *m_match_data;
    size_t m_size;
};

Napi::Object PCRE2::Init(Napi::Env env, Napi::Object exports) {
    InstanceData *instanceData = env.GetInstanceData<InstanceData>();

    Napi::Object symbol = env.Global().Get("Symbol").As<Napi::Object>();
    Napi::Function func = DefineClass(env, "PCRE2", {
        // InstanceMethod<&PCRE2::Exec>("exec"),
        InstanceMethod<&PCRE2::Test>("test"),
        // TODO There is no standard way to convert utf-16 to utf-8 and no support for formatting u16string....
        //  Also need to decide on the correct format for the string representation
        // InstanceMethod<&PCRE2::ToString>("toString"),
        // InstanceMethod<&PCRE2::ToString>(Napi::Symbol::For(env, "nodejs.util.inspect.custom")),
        // InstanceMethod<&PCRE2::Match>(symbol["match"]),
        // InstanceMethod<&PCRE2::MatchAll>(symbol["matchAll"]),
        // InstanceMethod<&PCRE2::Replace>(symbol["replace"]),
        // InstanceMethod<&PCRE2::Search>(symbol["search"]),
        // InstanceMethod<&PCRE2::Split>(symbol["split"]),
        // InstanceAccessor<&PCRE2::LastIndex>("lastIndex"),
        InstanceAccessor<&PCRE2::Source>("source"),
        InstanceAccessor<&PCRE2::Flags>("flags"),
        // InstanceAccessor<&PCRE2::DotAll>("dotAll"),
        // InstanceAccessor<&PCRE2::Global>("global"),
        // InstanceAccessor<&PCRE2::HasIndices>("hasIndices"),
        // InstanceAccessor<&PCRE2::IgnoreCase>("ignoreCase"),
        // InstanceAccessor<&PCRE2::Multiline>("multiline"),
        // InstanceAccessor<&PCRE2::Sticky>("sticky"),
        // InstanceAccessor<&PCRE2::Unicode>("unicode"),
        // InstanceAccessor<&PCRE2::UnicodeSets>("unicodeSets"),
    });

    instanceData->PCRE2 = Napi::Persistent(func);
    exports.Set("PCRE2", func);

    return exports;
}

PCRE2::PCRE2(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<PCRE2>(info)
{
    InstanceData *instanceData = info.Env().GetInstanceData<InstanceData>();

    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }

    if (info[0].IsString()) {
        m_pattern = info[0].As<Napi::String>().Utf16Value();
    } else if (info[0].IsObject() && info[0].As<Napi::Object>().InstanceOf(instanceData->RegExp.Value())) {
        Napi::Object re = info[0].As<Napi::Object>();
        Napi::Value source = re.Get("source");
        if (!source.IsString()) {
            throw Napi::TypeError::New(info.Env(), "Expected a string");
        }
        m_pattern = source.As<Napi::String>().Utf16Value();

        Napi::Value flags = re.Get("flags");
        if (!flags.IsString()) {
            throw Napi::TypeError::New(info.Env(), "Expected a string");
        }
        m_flags = flags.As<Napi::String>().Utf8Value();
    } else if (info[0].IsObject() && info[0].As<Napi::Object>().InstanceOf(instanceData->PCRE2.Value())) {
        PCRE2 *pcre2 = PCRE2::Unwrap(info[0].As<Napi::Object>());
        m_pattern = pcre2->m_pattern;
        m_flags = pcre2->m_flags;
    } else {
        throw Napi::TypeError::New(info.Env(), "Expected a string");
    }

    if (info.Length() > 1) {
        if (!info[1].IsString()) {
            throw Napi::TypeError::New(info.Env(), "Expected a string");
        }
        m_flags = info[1].As<Napi::String>().Utf8Value();
    }

    int errornumber;
    size_t erroroffset;
    m_re = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(m_pattern.c_str()),
        m_pattern.size(),
        // TODO Convert flags to PCRE2 options
        0,
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

    m_size = (m_pattern.size() * sizeof(char16_t)) + PatternSize(info.Env()) + pcre2_get_match_data_size(m_match_data);
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

    std::u16string subjectStr = info[0].As<Napi::String>().Utf16Value();

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
            oss << "PCRE2 matching error: " << rc;
            throw Napi::Error::New(info.Env(), oss.str());
        }
    }

    return Napi::Boolean::New(info.Env(), true);
}

// TODO There is no standard way to convert utf-16 to utf-8 and no support for formatting u16string....
// Napi::Value PCRE2::ToString(const Napi::CallbackInfo &info)
// {
//     std::ostringstream oss;
//     oss << "pcre2`" << m_source << "`";
//     return Napi::String::New(info.Env(), oss.str());
// }

size_t PCRE2::PatternSize(Napi::Env env) const {
    size_t size;
    int rc = pcre2_pattern_info(m_re, PCRE2_INFO_SIZE, &size);
    if (rc < 0) {
        std::ostringstream oss;
        oss << "PCRE2 error: " << rc;
        throw Napi::Error::New(env, oss.str());
    }

    return size;
}

Napi::Value PCRE2::Source(const Napi::CallbackInfo &info) {
    return Napi::String::New(info.Env(), m_pattern);
}

Napi::Value PCRE2::Flags(const Napi::CallbackInfo &info) {
    return Napi::String::New(info.Env(), m_flags);
}

static Napi::Object Init(Napi::Env env, Napi::Object exports) {
    env.SetInstanceData(new InstanceData(env));
    PCRE2::Init(env, exports);
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
