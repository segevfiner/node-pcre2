#include <sstream>
#include "InstanceData.h"
#include "PCRE2.h"

const napi_type_tag PCRE2TypeTag = {
    0x1edf75a38336451d, 0xa5ed9ce2e4c00c38
};

Napi::Object PCRE2::Init(Napi::Env env, Napi::Object exports) {
    InstanceData *instanceData = env.GetInstanceData<InstanceData>();

    Napi::Function func = DefineClass(env, "PCRE2", {
        InstanceMethod<&PCRE2::Exec>("exec"),
        InstanceMethod<&PCRE2::Test>("test"),
        InstanceMethod<&PCRE2::ToString>("toString"),
        InstanceMethod<&PCRE2::ToString>(Napi::Symbol::For(env, "nodejs.util.inspect.custom")),
        InstanceMethod<&PCRE2::Match>(instanceData->Symbol.Get("match").As<Napi::Symbol>()),
        InstanceMethod<&PCRE2::Search>(instanceData->Symbol.Get("search").As<Napi::Symbol>()),
        InstanceMethod<&PCRE2::Split>(instanceData->Symbol.Get("split").As<Napi::Symbol>()),
        InstanceMethod<&PCRE2::MatchAll>(instanceData->Symbol.Get("matchAll").As<Napi::Symbol>()),
        InstanceMethod<&PCRE2::Replace>(instanceData->Symbol.Get("replace").As<Napi::Symbol>()),
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
        InstanceAccessor<&PCRE2::PCRE2Mode>("pcre2"),
        StaticAccessor<&PCRE2::Species>(instanceData->Symbol.Get("species").As<Napi::Symbol>()),
    });

    instanceData->PCRE2 = Napi::Persistent(func);
    exports.Set("PCRE2", func);

    return exports;
}

PCRE2::PCRE2(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<PCRE2>(info)
    // Flags to try and behave more closely to JS RegExp, though we might want to make this configurable
    , m_options(0)
    , m_global(false)
    , m_sticky(false)
    , m_hasIndices(false)
    , m_pcre2(false)
    , m_lastIndex(0)
    , m_tierUpTicks(1)
{
    InstanceData *instanceData = info.Env().GetInstanceData<InstanceData>();

    Value().TypeTag(&PCRE2TypeTag);

    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }

    if (info[0].IsString()) {
        m_pattern = info[0].As<Napi::String>().Utf16Value();
    } else if (info[0].IsObject() && info[0].As<Napi::Object>().InstanceOf(instanceData->RegExp.Value())) {
        Napi::Object re = info[0].As<Napi::Object>();
        Napi::Value source = re.Get("source");
        if (!source.IsString()) {
            throw Napi::TypeError::New(info.Env(), "RegExp source is not a string");
        }
        m_pattern = source.As<Napi::String>().Utf16Value();

        Napi::Value flags = re.Get("flags");
        if (!flags.IsString()) {
            throw Napi::TypeError::New(info.Env(), "RegExp flags is not a string");
        }
        m_flags = flags.As<Napi::String>().Utf8Value();
    } else if (info[0].IsObject() && info[0].As<Napi::Object>().CheckTypeTag(&PCRE2TypeTag)) {
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

    if (!m_pcre2) {
        m_options = PCRE2_ALT_BSUX | PCRE2_DOLLAR_ENDONLY | PCRE2_MATCH_UNSET_BACKREF;
    }

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
        PCRE2_UCHAR errorBuffer[256];
        pcre2_get_error_message(errornumber, errorBuffer, sizeof(errorBuffer));
        Napi::String error = Napi::String::New(info.Env(), reinterpret_cast<const char16_t*>(errorBuffer));
        std::ostringstream oss;
        oss << "PCRE2 compilation failed at offset " << erroroffset << ": " << error.Utf8Value();
        throw Napi::Error::New(info.Env(), oss.str());
    }

    m_matchData = pcre2_match_data_create_from_pattern(m_re, nullptr);
    if (m_matchData == nullptr) {
        pcre2_code_free(m_re);
        throw Napi::Error::New(info.Env(), "PCRE2 match data allocation failed");
    }

    uint32_t option_bits;
    pcre2_pattern_info(m_re, PCRE2_INFO_ALLOPTIONS, &option_bits);
    m_utf8 = (option_bits & PCRE2_UTF) != 0;

    uint32_t newline;
    pcre2_pattern_info(m_re, PCRE2_INFO_NEWLINE, &newline);
    m_crlfIsNewline =
        newline == PCRE2_NEWLINE_ANY ||
        newline == PCRE2_NEWLINE_CRLF ||
        newline == PCRE2_NEWLINE_ANYCRLF;

    m_size = (m_pattern.size() * sizeof(char16_t)) + PatternSize(info.Env()) + pcre2_get_match_data_size(m_matchData);
    Napi::MemoryManagement::AdjustExternalMemory(info.Env(), m_size);
}

PCRE2::~PCRE2() {
    pcre2_match_data_free(m_matchData);
    pcre2_code_free(m_re);
    Napi::MemoryManagement::AdjustExternalMemory(Env(), -m_size);
}

Napi::Value PCRE2::ExecImpl(Napi::Env env, const Napi::String &subject, uint32_t options /* = 0 */) {
    Napi::EscapableHandleScope scope(env);
    InstanceData *instanceData = env.GetInstanceData<InstanceData>();

    std::u16string subjectStr = subject.Utf16Value();

    if (!m_global && !m_sticky) {
        m_lastIndex = 0;
    }

    TierUpTick(env);
    int rc = pcre2_match(
        m_re,
        reinterpret_cast<PCRE2_SPTR>(subjectStr.c_str()),
        subjectStr.length(),
        !m_global && !m_sticky ? 0 : m_lastIndex,
        options | (m_sticky ? PCRE2_ANCHORED : 0),
        m_matchData,
        nullptr
    );
    // TODO Might want to report pcre2_get_match_data_heapframes_size to AdjustExternalMemory
    if (rc < 0) {
        if (rc == PCRE2_ERROR_NOMATCH) {
            if (m_global || m_sticky) {
                m_lastIndex = 0;
            }
            return env.Null();
        }

        PCRE2_UCHAR errorBuffer[256];
        pcre2_get_error_message(rc, errorBuffer, sizeof(errorBuffer));
        Napi::String error = Napi::String::New(env, reinterpret_cast<const char16_t*>(errorBuffer));
        std::ostringstream oss;
        oss << "PCRE2 matching error " << rc << ": " << error.Utf8Value();
        throw Napi::Error::New(env, oss.str());
    }

    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(m_matchData);

    if (m_global || m_sticky) {
        m_lastIndex = ovector[1];
    }

    Napi::Array result = Napi::Array::New(env, rc);
    result["index"] = ovector[0];
    result["input"] = subject;

    Napi::Array indices;
    if (m_hasIndices) {
        indices = Napi::Array::New(env, rc);
        result["indices"] = indices;
    }

    for (PCRE2_SIZE i = 0; i < rc; i++) {
        if (ovector[2*i] == PCRE2_UNSET) {
            result[i] = env.Undefined();
            if (m_hasIndices) {
                indices[i] = env.Undefined();
            }
            continue;
        }

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
                if (ovector[2*n] == PCRE2_UNSET) {
                    groupsIndices.Set(groupName, env.Undefined());
                    continue;
                }
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

    return scope.Escape(result);
}

Napi::Value PCRE2::Exec(const Napi::CallbackInfo &info) {
    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }

    return ExecImpl(info.Env(), info[0].ToString());
}

Napi::Value PCRE2::Test(const Napi::CallbackInfo &info) {
    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }

    Napi::String subject = info[0].ToString();
    std::u16string subjectStr = info[0].As<Napi::String>().Utf16Value();

    TierUpTick(info.Env());
    int rc = pcre2_match(
        m_re,
        reinterpret_cast<PCRE2_SPTR>(subjectStr.c_str()),
        subjectStr.length(),
        m_lastIndex,
        m_sticky ? PCRE2_ANCHORED : 0,
        // TODO I think it is possible to use a smaller pcre2_match_data so it doesn't fill in ovector for performance
        m_matchData,
        nullptr
    );
    // TODO Might want to report pcre2_get_match_data_heapframes_size to AdjustExternalMemory
    if (rc < 0) {
        if (rc == PCRE2_ERROR_NOMATCH) {
            m_lastIndex = 0;
            return Napi::Boolean::New(info.Env(), false);
        }

        PCRE2_UCHAR errorBuffer[256];
        pcre2_get_error_message(rc, errorBuffer, sizeof(errorBuffer));
        Napi::String error = Napi::String::New(info.Env(), reinterpret_cast<const char16_t*>(errorBuffer));
        std::ostringstream oss;
        oss << "PCRE2 matching error " << rc << ": " << error.Utf8Value();
        throw Napi::Error::New(info.Env(), oss.str());
    }

    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(m_matchData);

    if (m_global || m_sticky) {
        m_lastIndex = ovector[1];
    }

    return Napi::Boolean::New(info.Env(), true);
}

Napi::Value PCRE2::ToString(const Napi::CallbackInfo &info) {
    std::ostringstream oss;

    oss << "pcre2";

    if (!m_flags.empty()) {
        oss << "('" << m_flags << "')";
    }

    std::string pattern = Napi::String::New(info.Env(), m_pattern).Utf8Value();
    oss << "`" << pattern << "`";

    return Napi::String::New(info.Env(), oss.str());
}

Napi::Value PCRE2::Match(const Napi::CallbackInfo &info) {
    InstanceData *instanceData = info.Env().GetInstanceData<InstanceData>();

    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }

    Napi::String subject = info[0].ToString();
    std::u16string subjectStr = subject.Utf16Value();

    if (!m_global) {
        return ExecImpl(info.Env(), subject);
    }

    m_lastIndex = 0;
    uint32_t options = 0;
    Napi::Array result = Napi::Array::New(info.Env());
    while (true) {
        Napi::Array match = ExecImpl(info.Env(), subject, options).As<Napi::Array>();
        if (match.IsNull()) {
            if (result.Length() == 0) {
                return info.Env().Null();
            }

            if (options == 0) {
                break;
            }

            m_lastIndex = AdvanceStringIndex(subjectStr, m_lastIndex);
            continue;
        }

        instanceData->ArrayPush.Call(result, { match.Get(0u) });

        options = 0;
        if (match.Get(0u).As<Napi::String>().Utf16Value().empty()) {
            if (m_lastIndex == subjectStr.length()) {
                break;
            }

            if (m_pcre2) {
                options = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
            } else {
                m_lastIndex = AdvanceStringIndex(subjectStr, m_lastIndex);
            }
        }
    }

    return result;
}

Napi::Value PCRE2::Search(const Napi::CallbackInfo &info) {
    InstanceData *instanceData = info.Env().GetInstanceData<InstanceData>();

    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }

    Napi::String subject = info[0].ToString();
    std::u16string subjectStr = subject.Utf16Value();

    size_t lastIndex = m_lastIndex;

    m_lastIndex = 0;
    Napi::Array match = ExecImpl(info.Env(), subject).As<Napi::Array>();
    m_lastIndex = lastIndex;
    if (match.IsNull()) {
        return Napi::Number::New(info.Env(), -1);
    }

    return match.Get("index");
}

Napi::Value PCRE2::Split(const Napi::CallbackInfo &info) {
    InstanceData *instanceData = info.Env().GetInstanceData<InstanceData>();

    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }
    Napi::String subject = info[0].ToString();
    std::u16string subjectStr = subject.Utf16Value();

    uint32_t limit = UINT32_MAX;
    if (info.Length() >= 2 && !info[1].IsUndefined()) {
        limit = info[1].ToNumber().Uint32Value();
    }

    Napi::Array result = Napi::Array::New(info.Env());
    if (limit == 0) {
        return result;
    }

    Napi::Function speciesCtor = SpeciesConstructor(info.Env(), Value(), instanceData->PCRE2.Value());
    std::string newFlags = m_flags;
    if (m_flags.find("y") == -1) {
        newFlags += "y";
    }
    PCRE2 *splitter = PCRE2::Unwrap(speciesCtor.New({ Value(), Napi::String::New(info.Env(), newFlags) }));

    if (subjectStr.empty()) {
        Napi::Array match = splitter->ExecImpl(info.Env(), subject).As<Napi::Array>();
        if (match.IsNull()) {
            return result;
        }

        result[0u] = subject;
        return result;
    }

    size_t p = 0;
    size_t q = p;

    while (q < subjectStr.size()) {
        splitter->m_lastIndex = q;
        Napi::Array match = splitter->ExecImpl(info.Env(), subject).As<Napi::Array>();
        if (match.IsNull()) {
            q = splitter->AdvanceStringIndex(subjectStr, q);
        } else {
            size_t e = std::min(splitter->m_lastIndex, subjectStr.size());
            if (e == p) {
                q = splitter->AdvanceStringIndex(subjectStr, q);
            } else {
                instanceData->ArrayPush.Call(result, { Napi::String::New(info.Env(), subjectStr.c_str() + p, q - p) });
                if (result.Length() == limit) {
                    return result;
                }

                p = e;

                size_t numberOfCaptures = std::max(match.Length() - 1, 0u);
                for (size_t i = 1; i < numberOfCaptures; i++) {
                    instanceData->ArrayPush.Call(result, { match.Get(i) });
                    if (result.Length() == limit) {
                        return result;
                    }
                }

                q = p;
            }
        }
    }

    instanceData->ArrayPush.Call(result, { Napi::String::New(info.Env(), subjectStr.c_str() + p, subjectStr.size() - p) });
    return result;
}

Napi::Value PCRE2::MatchAll(const Napi::CallbackInfo &info) {
    InstanceData *instanceData = info.Env().GetInstanceData<InstanceData>();

    Napi::Function speciesCtor = SpeciesConstructor(info.Env(), Value(), instanceData->PCRE2.Value());
    PCRE2 *matcher = PCRE2::Unwrap(speciesCtor.New({ Value(), Napi::String::New(info.Env(), m_flags) }));
    matcher->m_lastIndex = m_lastIndex;

    return instanceData->PCRE2StringIterator.New({ matcher->Value(), info[0] });
}

Napi::Value PCRE2::Replace(const Napi::CallbackInfo &info) {
    InstanceData *instanceData = info.Env().GetInstanceData<InstanceData>();

    if (info.Length() < 2) {
        throw Napi::TypeError::New(info.Env(), "Wrong number of arguments");
    }

    Napi::String subject = info[0].ToString();
    std::u16string subjectStr = subject.Utf16Value();

    if (m_global) {
        m_lastIndex = 0;
    }

    if (!info[1].IsFunction()) {
        Napi::String replacement = info[1].ToString();
        std::u16string replacementStr = replacement.Utf16Value();
        std::vector<PCRE2_UCHAR> outputBuffer(subjectStr.size() + (subjectStr.size() / 2));

        while (true) {
            PCRE2_SIZE outputBufferSize = outputBuffer.size();
            int rc = pcre2_substitute(
                m_re,
                reinterpret_cast<PCRE2_SPTR16>(subjectStr.c_str()),
                subjectStr.length(),
                0,
                PCRE2_SUBSTITUTE_OVERFLOW_LENGTH | (m_global ? PCRE2_SUBSTITUTE_GLOBAL : 0),
                m_matchData,
                nullptr,
                reinterpret_cast<PCRE2_SPTR16>(replacementStr.c_str()),
                replacementStr.length(),
                outputBuffer.data(),
                &outputBufferSize
            );
            if (rc < 0) {
                if (rc == PCRE2_ERROR_NOMEMORY) {
                    outputBuffer.resize(outputBufferSize);
                    continue;
                }

                PCRE2_UCHAR errorBuffer[256];
                pcre2_get_error_message(rc, errorBuffer, sizeof(errorBuffer));
                Napi::String error = Napi::String::New(info.Env(), reinterpret_cast<const char16_t*>(errorBuffer));
                std::ostringstream oss;
                oss << "PCRE2 substituion error " << rc << ": " << error.Utf8Value();
                throw Napi::Error::New(info.Env(), oss.str());
            }

            return Napi::String::New(info.Env(), reinterpret_cast<const char16_t*>(outputBuffer.data()), outputBufferSize);
        }
    } else {
        Napi::Function replacer = info[1].As<Napi::Function>();
        std::u16string result;

        uint32_t options = 0;
        size_t nextSubjectPosition = 0;
        while (true) {
            Napi::HandleScope scope(info.Env());

            Napi::Array match = ExecImpl(info.Env(), subject).As<Napi::Array>();
            if (match.IsNull()) {
                if (options == 0) {
                    break;
                }

                m_lastIndex = AdvanceStringIndex(subjectStr, m_lastIndex);
                continue;
            }

            Napi::String matched = match.Get(0u).ToString();
            std::vector<napi_value> replacerArgs = { matched };
            for (size_t i = 1; i < match.Length(); i++) {
                replacerArgs.push_back(match.Get(i).ToString());
            }
            replacerArgs.push_back(match.Get("index"));
            replacerArgs.push_back(subject);
            Napi::Value groups = match.Get("groups");
            if (!groups.IsUndefined()) {
                replacerArgs.push_back(groups);
            }
            Napi::String replacement = replacer.Call(replacerArgs).ToString();

            Napi::Number index = match.Get("index").As<Napi::Number>();
            result.append(subjectStr, nextSubjectPosition, index.Int64Value() - nextSubjectPosition);
            result.append(replacement.Utf16Value());
            nextSubjectPosition = index.Int64Value() + matched.Utf16Value().length();

            options = 0;
            if (match.Get(0u).As<Napi::String>().Utf16Value().empty()) {
                if (m_lastIndex == subjectStr.length()) {
                    break;
                }

                if (m_pcre2) {
                    options = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
                } else {
                    m_lastIndex = AdvanceStringIndex(subjectStr, m_lastIndex);
                }
            }
        }

        if (nextSubjectPosition < subjectStr.length()) {
            result.append(subjectStr, nextSubjectPosition, subjectStr.length() - nextSubjectPosition);
        }

        return Napi::String::New(info.Env(), result);
    }
}

Napi::Value PCRE2::GetLastIndex(const Napi::CallbackInfo &info) {
    return Napi::Number::New(info.Env(), m_lastIndex);
}

void PCRE2::SetLastIndex(const Napi::CallbackInfo &info, const Napi::Value &value) {
    if (!value.IsNumber()) {
        throw Napi::TypeError::New(info.Env(), "Expected a number");
    }

    m_lastIndex = value.As<Napi::Number>().Int64Value();
}

void PCRE2::ParseFlags(Napi::Env env, const std::string &flags) {
    // TODO We can support a bunch of other flags that are supoorted by PCRE2
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
            // TODO PCRE2 extra flags
            // case 'x':
            //     if (m_options | PCRE2_EXTENDED) {
            //         m_options |= PCRE2_EXTENDED_MORE;
            //     }
            //     m_options |= PCRE2_EXTENDED;
            //     break;
            // case 'n':
            //    m_options |= PCRE2_NO_AUTO_CAPTURE;
            //    break;
            // TODO Add aD, aS, aW, aP, aT, a, r, J, U
            case 'p':
                m_pcre2 = true;
                break;
            // TODO Add flag to disable JIT or force immediate JIT?
            default:
                throw Napi::Error::New(env, "Unsupported flag: " + std::string{flag});
        }
    }
}


size_t PCRE2::AdvanceStringIndex(const std::u16string &subjectStr, size_t index) {
    if (index == subjectStr.length()) {
        return index;
    }

    index++;

    if (m_crlfIsNewline &&
        index < subjectStr.length() - 1 && // We are at CRLF
        subjectStr[index] == '\r' &&
        subjectStr[index - 1] == '\n') {
            // Advance by one more.
            index++;
    } else if (m_utf8) {
        // Otherwise, ensure we advance a whole UTF-8 character
        while (index < subjectStr.length()) {
            if ((subjectStr[index] & 0xc0) != 0x80) {
                break;
            }
            index++;
        }
    }

    return index;
}

bool PCRE2::Global() const {
    return m_global;
}

size_t PCRE2::LastIndex() const {
    return m_lastIndex;
}

void PCRE2::SetLastIndex(size_t lastIndex) {
    m_lastIndex = lastIndex;
}

bool PCRE2::PCRE2Mode() const {
    return m_pcre2;
}

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

Napi::Value PCRE2::DotAll(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), m_options & PCRE2_DOTALL);
}

Napi::Value PCRE2::Global(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), m_global);
}

Napi::Value PCRE2::HasIndices(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), m_hasIndices);
}

Napi::Value PCRE2::IgnoreCase(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), m_options & PCRE2_CASELESS);
}

Napi::Value PCRE2::Multiline(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), m_options & PCRE2_MULTILINE);
}

Napi::Value PCRE2::Sticky(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), m_sticky);
}

Napi::Value PCRE2::Unicode(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), m_options & PCRE2_UTF);
}

Napi::Value PCRE2::UnicodeSets(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), m_options & PCRE2_ALT_EXTENDED_CLASS);
}

Napi::Value PCRE2::PCRE2Mode(const Napi::CallbackInfo &info) {
    return Napi::Boolean::New(info.Env(), m_pcre2);
}

Napi::Value PCRE2::Species(const Napi::CallbackInfo &info) {
    return info.This();
}

Napi::Function PCRE2::SpeciesConstructor(Napi::Env env, const Napi::Object &obj, const Napi::Function &defaultConstructor) {
    Napi::EscapableHandleScope scope(env);

    Napi::Value ctor = obj.Get("constructor");
    if (ctor.IsUndefined()) {
        return defaultConstructor;
    }

    if (!ctor.IsObject()) {
        throw Napi::TypeError::New(env, "constructor is not an object");
    }

    Napi::Value species = ctor.As<Napi::Object>().Get(env.GetInstanceData<InstanceData>()->Symbol.Get("species").As<Napi::Symbol>());
    if (species.IsUndefined() || species.IsNull()) {
        return defaultConstructor;
    }

    if (!species.IsFunction()) {
        throw Napi::TypeError::New(env, "species is not a function");
    }

    return scope.Escape(species).As<Napi::Function>();
}
