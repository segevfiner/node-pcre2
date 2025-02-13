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
