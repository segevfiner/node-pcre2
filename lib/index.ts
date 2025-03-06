import bindings from "bindings";

declare namespace Addon {
  class PCRE2 {
    constructor(pattern: string, flags?: string);

    static readonly [Symbol.species]: PCRE2;

    exec(string: string): RegExpExecArray | null;
    test(string: string): boolean;

    readonly lastIndex: number;
    readonly source: string;
    readonly flags: string;
    readonly global: boolean;
    readonly ignoreCase: boolean;
    readonly multiline: boolean;
    readonly sticky: boolean;
    readonly unicode: boolean;
    readonly dotAll: boolean;
    readonly hasIndices: boolean;
    readonly unicodeSets: boolean;

    [Symbol.match](string: string): RegExpMatchArray | null;
    [Symbol.search](string: string): number;
    [Symbol.split](string: string, limit?: number): string[];
    [Symbol.matchAll](str: string): RegExpStringIterator<RegExpMatchArray>;
    [Symbol.replace](str: string, replacement: string): string;

    // PCRE2 extras
    readonly extended: boolean;
    readonly extendedMore: boolean;
    readonly noAutoCapture: boolean;
    readonly asciiBsd: boolean;
    readonly asciiBss: boolean;
    readonly asciiBsw: boolean;
    readonly asciiDigit: boolean;
    readonly asciiPosix: boolean;
    readonly caselessRestrict: boolean;
    readonly dupnames: boolean;
    readonly ungreedy: boolean;
    readonly pcre2: boolean;
  }

  const PCRE2_MAJOR: number;
  const PCRE2_MINOR: number;
}

export const { PCRE2, PCRE2_MAJOR, PCRE2_MINOR } = bindings(
  "pcre2.node"
) as typeof Addon;

type pcre2Tag = {
  (flags: string): pcre2Tag;
  (template: TemplateStringsArray, ...substitutions: unknown[]): Addon.PCRE2;
}

function createTag(flags: string): pcre2Tag {
  function pcre2(flags: string): pcre2Tag;
  function pcre2(template: TemplateStringsArray, ...substitutions: unknown[]): Addon.PCRE2;
  function pcre2(templateOrFlags: string | TemplateStringsArray, ...substitutions: unknown[]): Addon.PCRE2 | pcre2Tag {
    if (typeof templateOrFlags === 'string') {
      return createTag(templateOrFlags);
    }

    const pattern = String.raw(templateOrFlags, ...substitutions);
    return new PCRE2(pattern, flags);
  }

  return pcre2;
}

export const pcre2 = createTag("");
