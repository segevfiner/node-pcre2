#include <sstream>
#include "InstanceData.h"
#include "PCRE2.h"

Napi::Object PCRE2::Init(Napi::Env env, Napi::Object exports) {
    InstanceData *instanceData = env.GetInstanceData<InstanceData>();

    Napi::Function func = DefineClass(env, "PCRE2", {
        InstanceMethod<&PCRE2::Exec>("exec"),
        InstanceMethod<&PCRE2::Test>("test"),
        // TODO There is no standard way to convert utf-16 to utf-8 and no support for formatting u16string....
        //  Also need to decide on the correct format for the string representation
        // InstanceMethod<&PCRE2::ToString>("toString"),
        // InstanceMethod<&PCRE2::ToString>(Napi::Symbol::For(env, "nodejs.util.inspect.custom")),
        InstanceMethod<&PCRE2::Match>(instanceData->Symbol.Get("match").As<Napi::Symbol>()),
        InstanceMethod<&PCRE2::MatchAll>(instanceData->Symbol.Get("matchAll").As<Napi::Symbol>()),
        // InstanceMethod<&PCRE2::Replace>(symbol["replace"]), // This one can be troublesome as pcre2_substitute doesn't handle dynamic replacement by the callout
        // InstanceMethod<&PCRE2::Search>(symbol["search"]),
        // InstanceMethod<&PCRE2::Split>(symbol["split"]),
        InstanceAccessor<&PCRE2::GetLastIndex, &PCRE2::SetLastIndex>("lastIndex"),
        InstanceAccessor<&PCRE2::Source>("source"),
        InstanceAccessor<&PCRE2::Flags>("flags"),
        InstanceAccessor<&PCRE2::DotAll>("dotAll"),
        InstanceAccessor<&PCRE2::Global>("global"),
        InstanceAccessor<&PCRE2::HasIndices>("hasIndices"),
        InstanceAccessor<&PCRE2::IgnoreCase>("ignoreCase"),
        InstanceAccessor<&PCRE2::Multiline>("multiline"),
        InstanceAccessor<&PCRE2::Sticky>("sticky"),
        InstanceAccessor<&PCRE2::Unicode>("unicode"),
        InstanceAccessor<&PCRE2::UnicodeSets>("unicodeSets"),
        StaticMethod<&PCRE2::Species>(instanceData->Symbol.Get("species").As<Napi::Symbol>()),
    });

    instanceData->PCRE2 = Napi::Persistent(func);
    exports.Set("PCRE2", func);

    return exports;
}

PCRE2::PCRE2(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<PCRE2>(info)
    , m_options(PCRE2_ALT_BSUX | PCRE2_DOLLAR_ENDONLY | PCRE2_MATCH_UNSET_BACKREF)
    , m_global(false)
    , m_sticky(false)
    , m_hasIndices(false)
    , m_lastIndex(0)
    , m_tierUpTicks(1)
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

    ParseFlags(info.Env(), m_flags);

    int errornumber;
    size_t erroroffset;
    m_re = pcre2_compile(
        reinterpret_cast<PCRE2_SPTR>(m_pattern.c_str()),
        m_pattern.size(),
        m_options,
        &errornumber,
        &erroroffset,
        instanceData->compileContext
    );
    if (m_re == nullptr) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        std::ostringstream oss;
        oss << "PCRE2 compilation failed at offset" << erroroffset << ": " << buffer;
        throw Napi::Error::New(info.Env(), oss.str());
    }

    m_matchData = pcre2_match_data_create_from_pattern(m_re, nullptr);
    if (m_matchData == nullptr) {
        pcre2_code_free(m_re);
        throw Napi::Error::New(info.Env(), "PCRE2 match data allocation failed");
    }

    m_size = (m_pattern.size() * sizeof(char16_t)) + PatternSize(info.Env()) + pcre2_get_match_data_size(m_matchData);
    Napi::MemoryManagement::AdjustExternalMemory(info.Env(), m_size);
}

PCRE2::~PCRE2() {
    pcre2_match_data_free(m_matchData);
    pcre2_code_free(m_re);
    Napi::MemoryManagement::AdjustExternalMemory(Env(), -m_size);
}

Napi::Value PCRE2::Exec(const Napi::CallbackInfo &info)
{
    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }
    if (!info[0].IsString()) {
        throw Napi::TypeError::New(info.Env(), "Expected a string");
    }

    return ExecImpl(info.Env(), info[0].As<Napi::String>());
}

Napi::Value PCRE2::ExecImpl(Napi::Env env, const Napi::String &subject)
{
    InstanceData *instanceData = env.GetInstanceData<InstanceData>();

    std::u16string subjectStr = subject.Utf16Value();

    Napi::EscapableHandleScope scope(env);

    TierUpTick(env);
    int rc = pcre2_match(
        m_re,
        reinterpret_cast<PCRE2_SPTR>(subjectStr.c_str()),
        subjectStr.length(),
        m_lastIndex,
        m_sticky ? PCRE2_ANCHORED : 0,
        m_matchData,
        nullptr
    );
    if (rc < 0) {
        if (rc == PCRE2_ERROR_NOMATCH) {
            m_lastIndex = 0;
            return env.Null();
        }

        std::ostringstream oss;
        oss << "PCRE2 matching error: " << rc;
        throw Napi::Error::New(env, oss.str());
    }

    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(m_matchData);
    Napi::Array result = Napi::Array::New(env, rc);

    Napi::Array indices;
    if (m_hasIndices) {
        indices = Napi::Array::New(env, rc);
        result["indices"] = indices;
    }

    for (PCRE2_SIZE i = 0; i < rc; i++) {
        const char16_t *substring_start = subjectStr.c_str() + ovector[2*i];
        PCRE2_SIZE substring_length = ovector[2*i+1] - ovector[2*i];
        result[i] = Napi::String::New(env, substring_start, substring_length);

        if (m_hasIndices) {
            Napi::Array indice = Napi::Array::New(env, 2);
            indice[0u] = ovector[2*i];
            indice[1] = ovector[2*i+1];
            indices[i] = indice;
        }
    }

    result["index"] = ovector[0];
    result["input"] = subject;

    uint32_t nameCount;
    pcre2_pattern_info(
        m_re,
        PCRE2_INFO_NAMECOUNT,
        &nameCount);

    if (nameCount > 0) {
        PCRE2_SPTR nameTable;
        pcre2_pattern_info(
            m_re,
            PCRE2_INFO_NAMETABLE,
            &nameTable);

        uint32_t nameEntrySize;
        pcre2_pattern_info(
            m_re,
            PCRE2_INFO_NAMEENTRYSIZE,
            &nameEntrySize);

        Napi::Object groups = instanceData->ObjectCreate.Call({ env.Null() }).As<Napi::Object>();

        Napi::Object groupsIndices;
        if (m_hasIndices) {
            groupsIndices = instanceData->ObjectCreate.Call({ env.Null() }).As<Napi::Object>();
            indices["groups"] = groupsIndices;
        }

        PCRE2_SPTR tabptr = nameTable;
        for (uint32_t i = 0; i < nameCount; i++) {
            Napi::String groupName = Napi::String::New(env, reinterpret_cast<const char16_t*>(tabptr + 1), nameEntrySize - 2);

            int n = tabptr[0];
            groups.Set(
                groupName,
                Napi::String::New(env, subjectStr.c_str() + ovector[2*n], ovector[2*n+1] - ovector[2*n]));
            tabptr += nameEntrySize;

            if (m_hasIndices) {
                Napi::Array indice = Napi::Array::New(env, 2);
                indice[0u] = ovector[2*n];
                indice[1] = ovector[2*n+1];
                groupsIndices.Set(groupName, indice);
            }
        }

        result["groups"] = groups;
    } else {
        result["groups"] = env.Undefined();
    }

    if (m_global || m_sticky) {
        m_lastIndex = ovector[1];
    }

    return scope.Escape(result);
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

    TierUpTick(info.Env());
    int rc = pcre2_match(
        m_re,
        reinterpret_cast<PCRE2_SPTR>(subjectStr.c_str()),
        subjectStr.length(),
        0,
        0,
        m_matchData,
        nullptr
    );
    if (rc < 0) {
        if (rc == PCRE2_ERROR_NOMATCH) {
            return Napi::Boolean::New(info.Env(), false);
        }

        std::ostringstream oss;
        oss << "PCRE2 matching error: " << rc;
        throw Napi::Error::New(info.Env(), oss.str());
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

Napi::Value PCRE2::Match(const Napi::CallbackInfo &info)
{
    InstanceData *instanceData = info.Env().GetInstanceData<InstanceData>();

    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }
    if (!info[0].IsString()) {
        throw Napi::TypeError::New(info.Env(), "Expected a string");
    }

    if (!m_global) {
        return ExecImpl(info.Env(), info[0].As<Napi::String>()).As<Napi::Object>();
    }

    Napi::Array result = Napi::Array::New(info.Env());
    for (uint32_t i; ; i++) {
        Napi::Value match = ExecImpl(info.Env(), info[0].As<Napi::String>()).As<Napi::Object>();
        if (match.IsNull()) {
            break;
        }

        // TODO Handle zero length matches

        instanceData->ArrayPush.Call(result, { match.As<Napi::Array>().Get(0u) });
    }

    return result;
}

Napi::Value PCRE2::MatchAll(const Napi::CallbackInfo &info)
{
    InstanceData *instanceData = info.Env().GetInstanceData<InstanceData>();

    Napi::Function speciesCtor = this->Value().Get("constructor").As<Napi::Object>().Get(instanceData->Symbol.Get("species").As<Napi::Symbol>()).As<Napi::Function>();
    PCRE2 *matcher = PCRE2::Unwrap(speciesCtor.New({ this->Value(), Napi::String::New(info.Env(), m_flags) }));
    matcher->m_lastIndex = m_lastIndex;

    return Napi::Value();
}

Napi::Value PCRE2::GetLastIndex(const Napi::CallbackInfo &info)
{
    return Napi::Number::New(info.Env(), m_lastIndex);
}

void PCRE2::SetLastIndex(const Napi::CallbackInfo &info, const Napi::Value &value)
{
    if (!value.IsNumber()) {
        throw Napi::TypeError::New(info.Env(), "Expected a number");
    }

    m_lastIndex = value.As<Napi::Number>().Int64Value();
}

void PCRE2::ParseFlags(Napi::Env env, const std::string &flags)
{
    for (char flag : flags) {
        switch (flag) {
            case 'd':
                m_hasIndices = true;
                break;
            case 'g':
                m_global = true;
                break;
            case 'i':
                m_options |= PCRE2_CASELESS;
                break;
            case 'm':
                m_options |= PCRE2_MULTILINE;
                break;
            case 's':
                m_options |= PCRE2_DOTALL;
                break;
            case 'u':
                m_options |= PCRE2_UTF;
                break;
            case 'v':
                m_options |= PCRE2_ALT_EXTENDED_CLASS;
                break;
            case 'y':
                m_sticky = true;
                break;
            default:
                throw Napi::Error::New(env, "Unsupported flag: " + std::string{flag});
        }
    }
}

size_t PCRE2::PatternSize(Napi::Env env) const
{
    size_t size;
    int rc = pcre2_pattern_info(m_re, PCRE2_INFO_SIZE, &size);
    if (rc < 0) {
        std::ostringstream oss;
        oss << "PCRE2 error: " << rc;
        throw Napi::Error::New(env, oss.str());
    }

    return size;
}

void PCRE2::TierUpTick(Napi::Env env) {
    if (m_tierUpTicks > 0) {
        if (m_tierUpTicks--) {
            pcre2_jit_compile(m_re, PCRE2_JIT_COMPLETE);

            size_t jitSize;
            pcre2_pattern_info(
                m_re,
                PCRE2_INFO_JITSIZE,
                &jitSize
            );

            if (jitSize != 0) {
                m_size += jitSize;
                Napi::MemoryManagement::AdjustExternalMemory(env, jitSize);
            }
        }
    }
}

Napi::Value PCRE2::Source(const Napi::CallbackInfo &info) {
    return Napi::String::New(info.Env(), m_pattern);
}

Napi::Value PCRE2::Flags(const Napi::CallbackInfo &info) {
    return Napi::String::New(info.Env(), m_flags);
}

Napi::Value PCRE2::DotAll(const Napi::CallbackInfo &info)
{
    return Napi::Boolean::New(info.Env(), m_options & PCRE2_DOTALL);
}

Napi::Value PCRE2::Global(const Napi::CallbackInfo &info)
{
    return Napi::Boolean::New(info.Env(), m_global);
}

Napi::Value PCRE2::HasIndices(const Napi::CallbackInfo &info)
{
    return Napi::Boolean::New(info.Env(), m_hasIndices);
}

Napi::Value PCRE2::IgnoreCase(const Napi::CallbackInfo &info)
{
    return Napi::Boolean::New(info.Env(), m_options & PCRE2_CASELESS);
}

Napi::Value PCRE2::Multiline(const Napi::CallbackInfo &info)
{
    return Napi::Boolean::New(info.Env(), m_options & PCRE2_MULTILINE);
}

Napi::Value PCRE2::Sticky(const Napi::CallbackInfo &info)
{
    return Napi::Boolean::New(info.Env(), m_sticky);
}

Napi::Value PCRE2::Unicode(const Napi::CallbackInfo &info)
{
    return Napi::Boolean::New(info.Env(), m_options & PCRE2_UTF);
}

Napi::Value PCRE2::UnicodeSets(const Napi::CallbackInfo &info)
{
    return Napi::Boolean::New(info.Env(), m_options & PCRE2_ALT_EXTENDED_CLASS);
}

Napi::Value PCRE2::Species(const Napi::CallbackInfo &info)
{
    InstanceData *instanceData = info.Env().GetInstanceData<InstanceData>();
    return instanceData->PCRE2.Value();
}
