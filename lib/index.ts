import bindings from 'bindings';

declare namespace Addon {
    class PCRE2 {}
}

export const { PCRE2 } = bindings('pcre2.node') as typeof Addon;
