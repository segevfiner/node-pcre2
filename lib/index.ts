import bindings from 'bindings';

declare namespace PCRE2 {
    class PCRE2 {}
}

export = bindings('pcre2.node') as typeof PCRE2;
