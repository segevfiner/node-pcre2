import { describe, expect, test } from "vitest";
import { PCRE2, pcre2 } from "..";

describe("PCRE2 constructor", () => {
  test("single argument", () => {
    const re = new PCRE2("abc");
    expect(re.source).toBe("abc");
    expect(re.flags).toBe("");
    expect(re.global).toBe(false);
    expect(re.ignoreCase).toBe(false);
    expect(re.multiline).toBe(false);
    expect(re.sticky).toBe(false);
    expect(re.unicode).toBe(false);
    expect(re.dotAll).toBe(false);
    expect(re.hasIndices).toBe(false);
    expect(re.unicodeSets).toBe(false);
    expect(re.extended).toBe(false);
    expect(re.extendedMore).toBe(false);
    expect(re.noAutoCapture).toBe(false);
    expect(re.pcre2).toBe(false);
  });

  test("two arguments", () => {
    const re = new PCRE2("abc", "i");
    expect(re.source).toBe("abc");
    expect(re.flags).toBe("i");
    expect(re.global).toBe(false);
    expect(re.ignoreCase).toBe(true);
    expect(re.multiline).toBe(false);
    expect(re.sticky).toBe(false);
    expect(re.unicode).toBe(false);
    expect(re.dotAll).toBe(false);
    expect(re.hasIndices).toBe(false);
    expect(re.unicodeSets).toBe(false);
    expect(re.extended).toBe(false);
    expect(re.extendedMore).toBe(false);
    expect(re.noAutoCapture).toBe(false);
    expect(re.pcre2).toBe(false);
  });
});

describe("PCRE2 tagged template literal", () => {
  test("no flags", () => {
    const re = pcre2`abc`;
    expect(re.source).toBe("abc");
    expect(re.flags).toBe("");
    expect(re.global).toBe(false);
    expect(re.ignoreCase).toBe(false);
    expect(re.multiline).toBe(false);
    expect(re.sticky).toBe(false);
    expect(re.unicode).toBe(false);
    expect(re.dotAll).toBe(false);
    expect(re.hasIndices).toBe(false);
    expect(re.unicodeSets).toBe(false);
    expect(re.extended).toBe(false);
    expect(re.extendedMore).toBe(false);
    expect(re.noAutoCapture).toBe(false);
    expect(re.pcre2).toBe(false);
  });

  test("with flags", () => {
    const re = pcre2("i")`abc`;
    expect(re.source).toBe("abc");
    expect(re.flags).toBe("i");
    expect(re.global).toBe(false);
    expect(re.ignoreCase).toBe(true);
    expect(re.multiline).toBe(false);
    expect(re.sticky).toBe(false);
    expect(re.unicode).toBe(false);
    expect(re.dotAll).toBe(false);
    expect(re.hasIndices).toBe(false);
    expect(re.unicodeSets).toBe(false);
    expect(re.extended).toBe(false);
    expect(re.extendedMore).toBe(false);
    expect(re.noAutoCapture).toBe(false);
    expect(re.pcre2).toBe(false);
  });
});
