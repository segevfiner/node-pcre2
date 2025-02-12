import bindings from "bindings";

// eslint-disable-next-line @typescript-eslint/no-unused-vars
declare namespace Addon {
  class PCRE2 extends RegExp {
    constructor(pattern: RegExp | PCRE2 | string, flags?: string);
  }

  const PCRE2_MAJOR: number;
  const PCRE2_MINOR: number;
}

export const { PCRE2, PCRE2_MAJOR, PCRE2_MINOR } = bindings(
  "pcre2.node"
) as typeof Addon;
